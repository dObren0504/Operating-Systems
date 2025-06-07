# 🗺️ Treasure Manager

A Linux-based C project to manage digital treasure hunts. It features a signal-driven monitor, dynamic score calculation, and treasure inspection across multiple hunt directories.

---

## 📁 Project Structure

- `treasure_hub.c`: Main program and CLI interface for controlling the monitor and user interactions.
- `treasure_manager.c`: Executed by the monitor to list/view treasures and hunts based on commands.
- `calculate_score.c`: Calculates player scores from binary treasure data.

---

## ⚙️ Features

- 📡 Signal-based monitor using `SIGUSR1`, `SIGTERM`, and `SIGCHLD`
- 📁 Reads commands from `/tmp/command.txt` and dispatches them
- 🧮 Calculates scores for each user based on `treasures.dat`
- 🧵 Uses pipes for inter-process communication
- 🧑‍💻 Simple CLI interface with interactive prompts

---

## 🔧 Requirements

- GCC (C compiler)
- Linux OS (signal and file system behavior is POSIX-based)

---

## 🔨 Build Instructions

You only need to compile `treasure_hub.c`. It will auto-compile the other components when run.

```bash
gcc -Wall -o treasure_hub treasure_hub.c
