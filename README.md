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
Script is interactive, containing of 4 questions:
1. **Provide codes directory (default: ./codes):** - folder containing the .cpp files to parse. JSON trees will be written to `<directory>/generated`;
2. **Delete isomorphism trees after creating tables? (y/N):** - if yes, the folder `<directory>/generated` is removed after the tables are produced;
3. **Do you need to generate new json files to check codes? (Y/n):** - if yes, the script regenerates AST JSONs using clang++ and your parser; otherwise it reuses existing files in `generated/`;
4. **Check codes based on code markers? (y/N):** - if yes, **all** codes will be checked in specified fragments (more in `Student markers` chapter);
---
## Student markers
[todo]
