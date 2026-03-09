"""
Single-file retargeting tool: convert one GMR motion file to Lab format.

What does this script do?
    - Reorder the DOF data from GMR (MuJoCo order) to Lab (Isaac Lab order).
    - Add loop mode to the motion data.
    - Run the Isaac Lab simulator to compute world-frame key-body positions.

Usage:
    Command line:
        python scripts/tools/retarget/single_retarget.py \\
            --robot g1 \\
            --input_file <path_to_gmr_file> \\
            --output_file <path_to_output_file> \\
            --config_file <path_to_config_file> \\
            [--loop {wrap,clamp}] \\
            [--frame_range START END]

    Required arguments:
        --robot         Robot name to use (default: g1).
        --input_file    Path to the input GMR motion file (pickle format).
        --output_file   Path to save the converted motion data (pickle format).
        --config_file   Path to the YAML configuration file.
                        Should contain: gmr_dof_names, lab_dof_names, lab_key_body_names

    Optional arguments:
        --loop {wrap,clamp}     Loop mode for the motion (default: clamp).
        --frame_range START END Frame range to extract: START is inclusive,
                                END is exclusive (like Python slicing).
                                If not provided, all frames will be processed.

    AppLauncher arguments:
        --headless              Run without GUI.
        --device {cpu,cuda:0}   Device to use for simulation (default: cuda:0).

    Examples:
        # Convert full motion with GUI
        python scripts/tools/retarget/single_retarget.py \\
            --robot g1 \\
            --input_file temp/gmr_data/walk.pkl \\
            --output_file temp/lab_data/walk.pkl \\
            --config_file scripts/tools/retarget/config/g1_29dof.yaml

        # Convert specific frame range without GUI
        python scripts/tools/retarget/single_retarget.py \\
            --robot g1 \\
            --input_file temp/gmr_data/walk.pkl \\
            --output_file temp/lab_data/walk_clip.pkl \\
            --config_file scripts/tools/retarget/config/g1_29dof.yaml \\
            --frame_range 10 100 \\
            --loop wrap \\
            --headless
"""

import argparse
import pickle
import sys
from pathlib import Path

import yaml
from isaaclab.app import AppLauncher

# ---------------------------------------------------------------------------
# Argument parsing (must happen before importing Isaac Sim modules)
# ---------------------------------------------------------------------------
parser = argparse.ArgumentParser(description="Retarget a single GMR motion file to Lab format.")
parser.add_argument("--robot", type=str, default="g1", help="Robot name to use (default: g1).")
parser.add_argument(
    "--input_file",
    type=str,
    required=True,
    help="Path to the input GMR motion file (pickle format).",
)
parser.add_argument(
    "--output_file",
    type=str,
    required=True,
    help="Path to save the converted motion data (pickle format).",
)
parser.add_argument(
    "--config_file",
    type=str,
    required=True,
    help="Path to the YAML configuration file.",
)
parser.add_argument(
    "--loop",
    type=str,
    choices=["wrap", "clamp"],
    default="clamp",
    help="Loop mode for the motion (default: clamp).",
)
parser.add_argument(
    "--frame_range",
    nargs=2,
    type=int,
    metavar=("START", "END"),
    help="Frame range: START END (both inclusive). Defaults to all frames.",
)

AppLauncher.add_app_launcher_args(parser)
args_cli = parser.parse_args()

# ---------------------------------------------------------------------------
# Launch Isaac Sim
# ---------------------------------------------------------------------------
app_launcher = AppLauncher(args_cli)
simulation_app = app_launcher.app

# ---------------------------------------------------------------------------
# Remaining imports (after Isaac Sim is running)
# ---------------------------------------------------------------------------
import isaaclab.sim as sim_utils
from isaaclab.scene import InteractiveScene

if args_cli.robot == "g1":
    from legged_lab.assets.unitree import UNITREE_G1_29DOF_CFG as ROBOT_CFG
else:
    raise ValueError(f"Robot '{args_cli.robot}' is not supported.")

# Make sibling modules importable.
script_dir = Path(__file__).parent
sys.path.insert(0, str(script_dir))

try:
    from gmr_to_lab import LoopMode, ReplayMotionsSceneCfg, extract_gmr_data, run_simulator
except ImportError as e:
    print(f"Error importing from gmr_to_lab.py: {e}")
    print("Make sure gmr_to_lab.py is in the same directory as this script.")
    raise


if __name__ == "__main__":
    with open(args_cli.config_file, "r") as f:
        config = yaml.safe_load(f)

    gmr_dof_names = config["gmr_dof_names"]
    lab_dof_names = config["lab_dof_names"]
    lab_key_body_names = config["lab_key_body_names"]

    loop_mode = LoopMode.CLAMP if args_cli.loop == "clamp" else LoopMode.WRAP

    start_frame = args_cli.frame_range[0] if args_cli.frame_range else 0
    end_frame = args_cli.frame_range[1] if args_cli.frame_range else -1

    motion_data_dict = extract_gmr_data(
        gmr_file_path=args_cli.input_file,
        gmr_dof_names=gmr_dof_names,
        lab_dof_names=lab_dof_names,
        loop_mode=loop_mode,
        start_frame=start_frame,
        end_frame=end_frame,
    )

    fps = motion_data_dict["fps"]
    dt = 1.0 / fps

    sim = sim_utils.SimulationContext(sim_utils.SimulationCfg(dt=dt, device=args_cli.device))
    scene_cfg = ReplayMotionsSceneCfg(
        num_envs=1,
        env_spacing=3.0,
        robot=ROBOT_CFG.replace(prim_path="{ENV_REGEX_NS}/Robot"),
    )
    scene = InteractiveScene(scene_cfg)

    sim.set_camera_view([2.0, 0.0, 2.5], [-0.5, 0.0, 0.5])

    sim.reset()
    print("Simulation starting ...")

    motion_data_dicts = run_simulator(
        simulation_app, sim, scene, [motion_data_dict], lab_key_body_names
    )
    motion_data_dict = motion_data_dicts[0]

    print("\n" + "=" * 60)
    print("💾 SAVING CONVERTED DATA")
    print("=" * 60)
    print(f"📁 Output File:      {args_cli.output_file}")
    print(f"🧮 Frames extracted: {end_frame - start_frame if args_cli.frame_range else 'All'}")
    print(f"🔁 Loop Mode:        {loop_mode.name}")
    print("=" * 60 + "\n")

    Path(args_cli.output_file).parent.mkdir(parents=True, exist_ok=True)
    with open(args_cli.output_file, "wb") as f:
        pickle.dump(motion_data_dict, f)
    print("✅ Data saved successfully.")

    print("Closing simulation app...")
    simulation_app.close()
    print("✅ Simulation app closed.")
