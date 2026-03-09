# 🤖 F4

## Table of Contents

- [Overview](#overview)
- [Installation](#installation)
- [Usage](#usage)
  - [Prepare Motion Data](#prepare-motion-data)
  - [Training & Play](#training-and-play)
- [Acknowledgement](#acknowledgement)

## 📖 Overview

This repository provides tools and configuration for legged robot reinforcement learning, including support for retargeting human motion data to robot models using [GMR](https://github.com/YanjieZe/GMR).

**Key Features:**

- Motion data retargeting from human motion capture data to robot models.
- Integration with [Isaac Lab](https://github.com/isaac-sim/IsaacLab) for simulation and training.
- Support for Unitree G1 (29 DoF) humanoid robot.

## ⚙️ Installation

### Prerequisites

- **Isaac Lab**: Ensure you have installed Isaac Lab `v2.3.1`. Follow the [official guide](https://isaac-sim.github.io/IsaacLab/main/source/setup/installation/index.html).
- **GMR**: Install [GMR](https://github.com/YanjieZe/GMR) for retargeting human motion data.
- **Git LFS**: Required for downloading large model files.

### Setup Steps

1. **Clone the Repository**

    ```bash
    git clone https://github.com/ggshao/F4
    cd F4
    ```

2. **Pull Git LFS Assets**

    ```bash
    git lfs install
    git lfs pull
    ```

3. **Install Dependencies**

    Use the Python interpreter associated with your Isaac Lab installation.

    ```bash
    python -m pip install pyyaml numpy torch
    ```

## 🚀 Usage

### 1. Prepare Motion Data

If you want to add more motion data, you can do so by following the steps below.

1. **Retarget human motion data** to the robot model. We recommend using [GMR](https://github.com/YanjieZe/GMR) for retargeting human motion data.

2. **Put the retargeted motion data** in the `temp/gmr_data` folder.

3. **Use a helper script** to convert the motion data to the required format:

    ```bash
    python scripts/tools/retarget/dataset_retarget.py \
        --robot g1 \
        --input_dir temp/gmr_data/ \
        --output_dir temp/lab_data/ \
        --config_file scripts/tools/retarget/config/g1_29dof.yaml \
        --loop clamp
    ```

4. **Move the converted data** from `temp/lab_data` to your motion data directory and update the `MotionDataCfg` in your task configuration file.

Please refer to the comments in the script for more details about the arguments, and refer to `scripts/tools/retarget/gmr_to_lab.py` for the data format used in this repository.

#### Single File Retargeting

If you want to retarget a single motion file (with optional frame clipping and interactive visualization), use `single_retarget.py`:

```bash
python scripts/tools/retarget/single_retarget.py \
    --robot g1 \
    --input_file temp/gmr_data/walk.pkl \
    --output_file temp/lab_data/walk.pkl \
    --config_file scripts/tools/retarget/config/g1_29dof.yaml \
    --loop clamp
```

To extract a specific frame range:

```bash
python scripts/tools/retarget/single_retarget.py \
    --robot g1 \
    --input_file temp/gmr_data/walk.pkl \
    --output_file temp/lab_data/walk_clip.pkl \
    --config_file scripts/tools/retarget/config/g1_29dof.yaml \
    --frame_range 10 100 \
    --loop wrap \
    --headless
```

#### Data Formats

**Input (GMR format)** — a pickle file containing a dict with:

| Key | Type | Shape | Description |
|-----|------|-------|-------------|
| `fps` | int | — | Frame rate |
| `root_pos` | ndarray | `(num_frames, 3)` | Root position |
| `root_rot` | ndarray | `(num_frames, 4)` | Root rotation quaternion (x, y, z, w) |
| `dof_pos` | ndarray | `(num_frames, num_dofs)` | Joint positions |

**Output (Lab format)** — a pickle file containing a dict with:

| Key | Type | Shape | Description |
|-----|------|-------|-------------|
| `fps` | int | — | Frame rate |
| `root_pos` | ndarray | `(num_frames, 3)` | Root position |
| `root_rot` | ndarray | `(num_frames, 4)` | Root rotation quaternion (w, x, y, z) |
| `dof_pos` | ndarray | `(num_frames, num_dofs)` | Joint positions (reordered to lab order) |
| `loop_mode` | int | — | 0 = clamp, 1 = wrap |
| `key_body_pos` | ndarray | `(num_frames, num_key_bodies, 3)` | Key body positions in world frame |

### 2. Training & Play

Refer to the [legged_lab](https://github.com/zitongbai/legged_lab) repository for training and playing with the converted motion data.

## 🙏 Acknowledgement

We would like to express our gratitude to the following open-source projects:

- [**Isaac Lab**](https://github.com/isaac-sim/IsaacLab) - The simulation foundation.
- [**legged_lab**](https://github.com/zitongbai/legged_lab) - Legged robot RL framework.
- [**GMR**](https://github.com/YanjieZe/GMR) - Excellent motion retargeting library.
- [**MimicKit**](https://github.com/xbpeng/MimicKit) - Reference for imitation learning.
