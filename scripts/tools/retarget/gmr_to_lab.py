"""
This module provides functionality to convert motion data from GMR format to the Lab format
used for legged robot reinforcement learning.

Ref:
    - https://github.com/xbpeng/MimicKit/blob/main/tools/gmr_to_mimickit/gmr_to_mimickit.py
    - https://github.com/HybridRobotics/whole_body_tracking/blob/main/scripts/csv_to_npz.py

GMR Format:
    The input GMR format should be a pickle file containing a dictionary with keys:
    - 'fps': Frame rate (int)
    - 'root_pos': Root position array, shape (num_frames, 3)
    - 'root_rot': Root rotation quaternions, shape (num_frames, 4), format (x, y, z, w)
    - 'dof_pos': Degrees of freedom positions, shape (num_frames, num_dofs)
    - 'local_body_pos': Currently unused (can be None)
    - 'link_body_list': Currently unused (can be None)

Output Lab Format:
    The output Lab format is a dictionary with keys:
    - 'fps': Frame rate (int)
    - 'root_pos': Root position array, shape (num_frames, 3)
    - 'root_rot': Root rotation quaternions, shape (num_frames, 4), format (w, x, y, z)
    - 'dof_pos': Degrees of freedom positions, shape (num_frames, num_dofs)
    - 'loop_mode': Loop mode (int, 0 for clamp, 1 for wrap)
    - 'key_body_pos': Key body positions in world frame, shape (num_frames, num_key_bodies, 3)
"""

import enum

import numpy as np
import pickle
import torch

import isaaclab.sim as sim_utils
import isaaclab.utils.math as math_utils
from isaaclab.assets import Articulation, ArticulationCfg, AssetBaseCfg
from isaaclab.markers import VisualizationMarkers, VisualizationMarkersCfg
from isaaclab.scene import InteractiveScene, InteractiveSceneCfg
from isaaclab.utils import configclass
from isaaclab.utils.assets import ISAAC_NUCLEUS_DIR


class LoopMode(enum.Enum):
    CLAMP = 0
    WRAP = 1


def extract_gmr_data(
    gmr_file_path: str,
    gmr_dof_names: list,
    lab_dof_names: list,
    loop_mode: LoopMode,
    start_frame: int = 0,
    end_frame: int = -1,
) -> dict:
    """Load a GMR pickle file and convert it to the Lab motion data format.

    Args:
        gmr_file_path: Path to the GMR pickle file.
        gmr_dof_names: Ordered list of DOF names in the GMR file.
        lab_dof_names: Ordered list of DOF names expected by the lab format.
        loop_mode: How the motion should loop (CLAMP or WRAP).
        start_frame: Index of the first frame to include (inclusive). Defaults to 0.
        end_frame: Index of the last frame to include (exclusive, like Python slicing).
            Use -1 (default) to include all frames up to the end.

    Returns:
        Dictionary in Lab motion format (without 'key_body_pos'; that is filled
        by :func:`run_simulator`).
    """
    with open(gmr_file_path, "rb") as f:
        gmr_data = pickle.load(f)

    fps = gmr_data["fps"]
    root_pos = gmr_data["root_pos"]        # (num_frames, 3)
    root_rot_quat = gmr_data["root_rot"]   # (num_frames, 4), xyzw
    dof_pos = gmr_data["dof_pos"]          # (num_frames, num_dofs)

    print("\n" + "=" * 60)
    print("📥 LOADED GMR DATA")
    print("=" * 60)
    print(f"⏱️  FPS:           type={type(fps).__name__}, value={fps}")
    print(f"📍 Root Position: type={type(root_pos).__name__}, shape={root_pos.shape}")
    print(f"🔄 Root Rotation: type={type(root_rot_quat).__name__}, shape={root_rot_quat.shape}")
    print(f"🦴 DOF Position:  type={type(dof_pos).__name__}, shape={dof_pos.shape}")
    print("=" * 60 + "\n")

    if root_pos.ndim != 2 or root_pos.shape[1] != 3:
        raise ValueError(f"Expected root_pos shape (num_frames, 3), got {root_pos.shape}")
    if root_rot_quat.ndim != 2 or root_rot_quat.shape[1] != 4:
        raise ValueError(f"Expected root_rot_quat shape (num_frames, 4), got {root_rot_quat.shape}")
    if dof_pos.ndim != 2:
        raise ValueError(f"Expected dof_pos to be 2D array, got {dof_pos.ndim}D")

    num_frames = dof_pos.shape[0]
    if end_frame == -1 or end_frame > num_frames:
        end_frame = num_frames
    assert 0 <= start_frame < end_frame <= num_frames, "Invalid start_frame or end_frame."

    # Build the index mapping from GMR order to lab order.
    gmr_to_lab_indices = []
    for lab_dof in lab_dof_names:
        if lab_dof in gmr_dof_names:
            gmr_to_lab_indices.append(gmr_dof_names.index(lab_dof))
        else:
            raise ValueError(f"DOF name '{lab_dof}' not found in GMR DOF names.")

    dof_pos_lab = dof_pos[:, gmr_to_lab_indices]

    return {
        "fps": fps,
        "root_pos": root_pos[start_frame:end_frame],
        "root_rot": root_rot_quat[start_frame:end_frame],
        "dof_pos": dof_pos_lab[start_frame:end_frame],
        "loop_mode": loop_mode.value,
    }


def run_simulator(
    simulation_app,
    sim: sim_utils.SimulationContext,
    scene: InteractiveScene,
    motion_data_dicts: list,
    key_body_names: list,
) -> list:
    """Step the Isaac Lab simulator to compute world-frame key-body positions.

    For each motion in *motion_data_dicts* the robot is replayed frame-by-frame
    (physics disabled, render only) and the positions of the bodies listed in
    *key_body_names* are recorded.  The results are stored back into each dict
    under the key ``'key_body_pos'``, and the root rotation is converted from
    (x, y, z, w) to (w, x, y, z) in-place.

    Args:
        simulation_app: The running Isaac Sim application object.
        sim: The :class:`~isaaclab.sim.SimulationContext` instance.
        scene: The :class:`~isaaclab.scene.InteractiveScene` containing
            a robot articulation registered under the key ``"robot"``.
        motion_data_dicts: List of motion dicts (one per environment) produced
            by :func:`extract_gmr_data`.
        key_body_names: Names of the robot bodies whose world-frame positions
            should be recorded (must match ``robot.data.body_names``).

    Returns:
        The same list of motion dicts, each enriched with
        ``'key_body_pos'`` (shape ``(num_frames, num_key_bodies, 3)``) and
        with ``'root_rot'`` converted to (w, x, y, z) format.
    """
    robot: Articulation = scene["robot"]

    marker_cfg = VisualizationMarkersCfg(
        prim_path="/Visuals/FrameVisualizerFromScript",
        markers={
            "red_sphere": sim_utils.SphereCfg(
                radius=0.03,
                visual_material=sim_utils.PreviewSurfaceCfg(diffuse_color=(1.0, 0.0, 0.0)),
            ),
        },
    )
    marker: VisualizationMarkers = VisualizationMarkers(marker_cfg)

    num_motions = len(motion_data_dicts)
    assert num_motions == scene.num_envs, "Number of motions must match number of environments."

    fps = motion_data_dicts[0]["fps"]
    root_pos_w_list = []
    root_quat_list = []
    dof_pos_list = []
    num_frames_list = []

    for motion_data in motion_data_dicts:
        root_pos_w_list.append(
            torch.from_numpy(motion_data["root_pos"]).to(scene.device).float()
        )

        root_quat_tensor = torch.from_numpy(motion_data["root_rot"]).to(scene.device).float()
        # Convert from GMR (x, y, z, w) to Lab (w, x, y, z) quaternion convention.
        root_quat_tensor = math_utils.convert_quat(root_quat_tensor, "wxyz")
        root_quat_tensor = math_utils.quat_unique(root_quat_tensor)
        root_quat_tensor = math_utils.normalize(root_quat_tensor)
        root_quat_list.append(root_quat_tensor)

        dof_pos_list.append(
            torch.from_numpy(motion_data["dof_pos"]).to(scene.device).float()
        )
        num_frames_list.append(motion_data["dof_pos"].shape[0])

    max_num_frames = max(num_frames_list)

    lab_body_names = robot.data.body_names
    key_body_indices = []
    for name in key_body_names:
        if name in lab_body_names:
            key_body_indices.append(lab_body_names.index(name))
        else:
            raise ValueError(f"Key body name '{name}' not found in robot body names.")

    key_body_pos_w_list = [
        torch.zeros((num_frames, len(key_body_indices), 3), device=scene.device)
        for num_frames in num_frames_list
    ]

    count = 0
    sim_time = 0.0
    dt = sim.cfg.dt

    while simulation_app.is_running():
        root_states = robot.data.default_root_state.clone()
        joint_pos = robot.data.default_joint_pos.clone()
        joint_vel = torch.zeros_like(robot.data.default_joint_vel)

        for motion_idx in range(num_motions):
            num_frames = num_frames_list[motion_idx]
            frame_idx = count if count < num_frames else num_frames - 1

            root_states[motion_idx, :3] = root_pos_w_list[motion_idx][frame_idx, :]
            root_states[motion_idx, :3] += scene.env_origins[motion_idx, :3]
            root_states[motion_idx, 3:7] = root_quat_list[motion_idx][frame_idx, :]
            root_states[motion_idx, 7:10] = 0.0
            root_states[motion_idx, 10:13] = 0.0

            joint_pos[motion_idx, :] = dof_pos_list[motion_idx][frame_idx, :]

        robot.write_root_state_to_sim(root_states)
        robot.write_joint_state_to_sim(joint_pos, joint_vel)

        sim.render()
        scene.update(dt)

        for motion_idx in range(num_motions):
            num_frames = num_frames_list[motion_idx]
            if count < num_frames:
                key_body_pos_w_tensor = (
                    robot.data.body_pos_w[motion_idx, key_body_indices, :]
                    - scene.env_origins[motion_idx, :3]
                )
                key_body_pos_w_list[motion_idx][count, :, :] = key_body_pos_w_tensor

        vis_key_body_pos_w = robot.data.body_pos_w[:, key_body_indices, :]
        marker.visualize(translations=vis_key_body_pos_w.reshape(-1, 3))

        count += 1
        sim_time += dt
        if count >= max_num_frames:
            break

    print(
        f"[INFO]: Simulation completed in {count} steps, "
        f"total time: {sim_time:.2f} seconds."
    )

    for motion_data_dict, root_quat in zip(motion_data_dicts, root_quat_list):
        motion_data_dict["root_rot"] = root_quat.cpu().numpy()

    for motion_data_dict, key_body_pos_w in zip(motion_data_dicts, key_body_pos_w_list):
        motion_data_dict["key_body_pos"] = key_body_pos_w.cpu().numpy()

    return motion_data_dicts


@configclass
class ReplayMotionsSceneCfg(InteractiveSceneCfg):
    """Configuration for a scene that replays recorded motions."""

    ground = AssetBaseCfg(prim_path="/World/defaultGroundPlane", spawn=sim_utils.GroundPlaneCfg())

    sky_light = AssetBaseCfg(
        prim_path="/World/skyLight",
        spawn=sim_utils.DomeLightCfg(
            intensity=750.0,
            texture_file=(
                f"{ISAAC_NUCLEUS_DIR}/Materials/Textures/Skies/PolyHaven/"
                "kloofendal_43d_clear_puresky_4k.hdr"
            ),
        ),
    )

    # Populated by the calling script with the appropriate robot config.
    robot: ArticulationCfg = None
