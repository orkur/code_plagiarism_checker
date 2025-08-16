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
python3 check_codes.py
```