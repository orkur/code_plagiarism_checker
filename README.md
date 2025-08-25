# code_plagiarism_checker

## Installation requirements

### System prerequisites
- `clang++` — used for creating ast trees
- `g++` - used for compiling plagiarism logic
- `make`
- Python **3.12.0** (with `pip`)

---

### C++17

This project uses **nlohmann/json** at `include/nlohmann/json.hpp` (no linking required, file should be installed using a makefile).

Build the executable:

```bash
make build
```
this produces both `./tree_isomorphism` and `include/nlohmann/json.hpp`.

---

### Python 3.12.0
set up virtual environment, then install dependencies:
```bash
python -m venv .venv
source .venv/bin/activate
python -m pip install -U pip
pip install -r requirements.txt
```

## Run
From the project root run:
```bash
python3 check_codes.py [SRC] [--json-dir DIR] [--reuse-json] [--clean-json] [--markers] [--metrics]
```
Arguments
1. `SRC` (optional, default: `./codes`) — directory with C++ sources to analyze.
2. `--json-dir DIR` (optional, default: `generated` under SRC) — where AST .json files are stored. If DIR is relative, it is resolved inside SRC; absolute paths are respected.
3. `--reuse-json` — reuse existing .json files; skip regeneration if the file already exists.
4. `--clean-json` — remove the JSON directory after the run completes.
5. `--markers` — enable fragment analysis based on in-source markers (see Student markers section).
6. `--metrics` — Comma-separated list of metrics to compute (STRICT,LEV,TED). Default: STRICT,LEV,TED
---
## Student markers
[todo]
