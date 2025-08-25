import argparse
import sys
from multiprocessing import Pool
from os import walk, cpu_count
import shutil
import os
import subprocess
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

def run_parser(filename, json_dir, include_markers, dir):
    outfile_path = f"{json_dir}/{filename.split('.')[0]}.json"
    with open(outfile_path, "w") as outfile:
        subprocess.run(["clang++", "-Xclang", "-ast-dump=json", "-Iinclude", "-fsyntax-only", f"{dir}/{filename}"], stdout=outfile)
        args = ["python3", "./ast-parser/ast_parser.py", outfile_path, outfile_path]
        if include_markers:
            args.append("-m")
        subprocess.run(args)

def plot_heatmap(data, title, cmap="coolwarm"):
    plt.figure(figsize=(8, 6))
    sns.heatmap(data.astype(float), annot=True, fmt=".2f", cmap=cmap, vmin=0, vmax=1, linewidths=0.5, linecolor='gray')
    plt.title(title)
    plt.tight_layout()
    plt.show()

def put_into_table(tbl, i, j, v):
    tbl.loc[i, j] = v
    tbl.loc[j, i] = v


def run_logic(i, j, a_path, b_path):
    res = subprocess.run(["./tree_isomorphism", a_path, b_path],
                         capture_output=True, text=True)
    if res.returncode != 0:
        return {"ERROR": float("nan")}
    scores = {}
    for line in res.stdout.strip().splitlines():
        try:
            k, v = line.split(": ")
            scores[k] = float(v)
        except ValueError:
            scores["ERROR"] = float("nan")
            break
    return i, j, scores

def parse_args():
    p = argparse.ArgumentParser(
        description="Compare C++ sources by AST-derived similarity metrics.",
        formatter_class=argparse.RawTextHelpFormatter,
    )
    p.add_argument(
        "src", nargs="?", default="./codes",
        help="Directory with C++ source files (default: ./codes)",
    )
    p.add_argument("--json-dir", default="generated",
                   help=("Directory for AST .json files. "
                         "Default: 'generated' (inside SRC).")
    )
    p.add_argument(
        "--clean-json", action="store_true",
        help="Remove generated .json files at the end.",
    )
    p.add_argument(
        "--reuse-json", action="store_true",
        help="Reuse existing AST .json files; skip regeneration if present.",
    )
    p.add_argument(
        "--markers", action="store_true",
        help="Enable fragment analysis based on in-source markers.",
    )
    args = p.parse_args()

    if not os.path.isdir(args.src):
        p.error(f"'{args.src}' is not a directory")

    args.json_dir = args.json_dir if os.path.isabs(args.json_dir) else os.path.join(args.src, args.json_dir)

    os.makedirs(args.json_dir, exist_ok=True)

    return args


if __name__ == '__main__':
    args = parse_args()
    dir = args.src
    json_dir = args.json_dir
    delete_json_files = args.clean_json
    filenames = next(walk(dir), (None, None, []))[2]
    reuse_json = args.reuse_json
    if not reuse_json:
        include_markers = args.markers
        with Pool(processes=cpu_count()) as pool:
            pool.starmap(run_parser, [(filename, json_dir, include_markers, dir) for filename in filenames])
    TED_table = pd.DataFrame()
    LEV_table = pd.DataFrame()
    STRICT_table = pd.DataFrame()

    trees_to_check = next(walk(json_dir), (None, None, []))[2]
    for i in range(len(trees_to_check)):
        for j in range(i, len(trees_to_check)):
            a_file, b_file = trees_to_check[i], trees_to_check[j]
            a_path = os.path.join(json_dir, a_file)
            b_path = os.path.join(json_dir, b_file)

            result = subprocess.run(
                ["./tree_isomorphism", a_path, b_path],
                capture_output=True,
                text=True
            )
            lines = result.stdout.strip().split("\n")
            scores = {}
            for line in lines:
                try:
                    key, val = line.split(": ")
                except ValueError:
                    print("There's a problem with files", a_file, b_file)
                    sys.exit(1)
                scores[key] = float(val)
            put_into_table(TED_table,   a_file, b_file, scores.get("TED"))
            put_into_table(LEV_table,   a_file, b_file, scores.get("Levenshtein distance"))
            put_into_table(STRICT_table,a_file, b_file, scores.get("strict similarity"))

    print("\n=== TED Table ===")
    print(TED_table.round(2))

    print("\n=== Levenshtein Distance Table ===")
    print(LEV_table.round(2))

    print("\n=== Strict Similarity Table ===")
    print(STRICT_table.astype(int))


    plot_heatmap(TED_table, "Tree Edit Distance (TED)")
    plot_heatmap(LEV_table, "Levenshtein Distance")
    plot_heatmap(STRICT_table, "Strict Similarity")
    if delete_json_files:
        shutil.rmtree(json_dir)

