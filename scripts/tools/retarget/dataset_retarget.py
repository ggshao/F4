"""
Batch retargeting tool: convert multiple GMR motion files to Lab format.

Behavior:
    - Reads all .pkl files from the input directory (sorted).
    - For each file, loads the GMR pickle and uses extract_gmr_data to convert it
      (entire motion — frame clipping is not supported in batch mode).
    - Runs the Isaac Lab simulator once with all motions (num_envs = number of
      motions) and collects key body positions.
    - Saves each converted motion dict to the output directory with the same
      filename.

Usage:
    python scripts/tools/retarget/dataset_retarget.py \\
        --robot g1 \\
        --input_dir temp/gmr_data/ \\
        --output_dir temp/lab_data/ \\
        --config_file scripts/tools/retarget/config/g1_29dof.yaml \\
        --loop clamp

Arguments:
    --robot         Robot name to use (default: g1).
    --input_dir     Directory containing input GMR .pkl files.
    --output_dir    Directory to write the converted .pkl files.
    --config_file   Path to the YAML configuration file.
                    Should contain: gmr_dof_names, lab_dof_names, lab_key_body_names
    --loop          Loop mode: 'clamp' or 'wrap' (default: clamp).

AppLauncher arguments:
    --headless      Run without GUI.
    --device        Device to use for simulation (default: cuda:0).
"""

import argparse
import pickle
import sys
import warnings
from pathlib import Path

import yaml
from isaaclab.app import AppLauncher

# ---------------------------------------------------------------------------
# Argument parsing (must happen before importing Isaac Sim modules)
# ---------------------------------------------------------------------------
parser = argparse.ArgumentParser(
    description="Batch retarget GMR motion files to Lab format."
)
parser.add_argument("--robot", type=str, default="g1", help="Robot name to use (default: g1).")
parser.add_argument(
    "--input_dir",
    type=str,
    required=True,
    help="Directory containing input GMR .pkl files.",
)
parser.add_argument(
    "--output_dir",
    type=str,
    required=True,
    help="Directory to write the converted .pkl files.",
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
    help="Loop mode for all motions (default: clamp).",
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
    raise


def list_input_files(input_dir: str):
    """Return a sorted list of .pkl files in *input_dir*."""
    p = Path(input_dir)
    if not p.exists() or not p.is_dir():
        raise ValueError(f"Input directory does not exist: {input_dir}")
    return sorted(f for f in p.iterdir() if f.is_file() and f.suffix == ".pkl")


def main():
    with open(args_cli.config_file, "r") as f:
        config = yaml.safe_load(f)

    gmr_dof_names = config["gmr_dof_names"]
    lab_dof_names = config["lab_dof_names"]
    lab_key_body_names = config["lab_key_body_names"]

    loop_mode = LoopMode.CLAMP if args_cli.loop == "clamp" else LoopMode.WRAP

    input_files = list_input_files(args_cli.input_dir)
    if not input_files:
        print(f"No .pkl files found in input directory: {args_cli.input_dir}")
        return

    Path(args_cli.output_dir).mkdir(parents=True, exist_ok=True)

    # ------------------------------------------------------------------
    # Load and convert all GMR files (entire motion, no frame clipping).
    # ------------------------------------------------------------------
    motion_data_dicts = []
    input_names = []
    fps_values = []

    print(f"Found {len(input_files)} file(s) to convert.")
    for p in input_files:
        print(f"Loading and converting: {p.name}")
        motion = extract_gmr_data(
            gmr_file_path=str(p),
            gmr_dof_names=gmr_dof_names,
            lab_dof_names=lab_dof_names,
            loop_mode=loop_mode,
            start_frame=0,
            end_frame=-1,
        )
        motion_data_dicts.append(motion)
        input_names.append(p.name)
        fps_values.append(motion["fps"])

    if not all(f == fps_values[0] for f in fps_values):
        print(fps_values)
        warnings.warn("Motions have different fps values. Using fps from the first motion.")

    fps = fps_values[0]
    dt = 1.0 / fps

    # ------------------------------------------------------------------
    # Run the simulator once for all motions.
    # ------------------------------------------------------------------
    sim = sim_utils.SimulationContext(sim_utils.SimulationCfg(dt=dt, device=args_cli.device))
    scene_cfg = ReplayMotionsSceneCfg(
        num_envs=len(motion_data_dicts),
        env_spacing=3.0,
        robot=ROBOT_CFG.replace(prim_path="{ENV_REGEX_NS}/Robot"),
    )
    scene = InteractiveScene(scene_cfg)

    sim.set_camera_view([2.0, 0.0, 2.5], [-0.5, 0.0, 0.5])

    sim.reset()
    print("Simulation starting ...")

    motion_data_dicts = run_simulator(
        simulation_app, sim, scene, motion_data_dicts, lab_key_body_names
    )

    # ------------------------------------------------------------------
    # Save outputs.
    # ------------------------------------------------------------------
    print("Saving converted motions to output directory...")
    for name, motion in zip(input_names, motion_data_dicts):
        out_path = Path(args_cli.output_dir) / name
        with open(out_path, "wb") as f:
            pickle.dump(motion, f)
        print(f"Saved: {out_path}")

    print("Closing simulation app...")
    simulation_app.close()
    print("Done.")


if __name__ == "__main__":
    main()
