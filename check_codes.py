import argparse
import math
from concurrent.futures import ThreadPoolExecutor, as_completed
from itertools import combinations_with_replacement, repeat
from os import walk, cpu_count
import shutil
import os
import subprocess
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
from matplotlib.colors import ListedColormap, BoundaryNorm


def run_parser(filename, json_dir, include_markers, dir):
    outfile_path = f"{json_dir}/{filename.split('.')[0]}.json"
    with open(outfile_path, "w") as outfile:
        subprocess.run(["clang++", "-Xclang", "-ast-dump=json", "-Iinclude", "-fsyntax-only", f"{dir}/{filename}"], stdout=outfile)
        args = ["python3", "./ast-parser/ast_parser.py", outfile_path, outfile_path]
        if include_markers:
            args.append("-m")
        subprocess.run(args)

def plot_heatmap(data, title, palette="coolwarm"):
    bounds = [0.5, 0.6, 0.7, 0.8, 0.85, 0.9, 0.95, 1.0]

    colors = sns.color_palette(palette, n_colors=len(bounds) - 1)
    colors[-1] = "#cd1204"
    cmap = ListedColormap(colors)
    norm = BoundaryNorm(boundaries=bounds, ncolors=cmap.N, clip=True)

    mask = data.isna()
    annot_ok = not mask.values.any()

    plt.figure(figsize=(8, 6))
    sns.heatmap(
        data.astype(float), mask=mask,
        cmap=cmap, norm=norm,
        cbar_kws={"ticks": bounds, "spacing": "proportional"},
        annot=annot_ok, fmt=".2f",
        vmin=0, vmax=1, linewidths=0.5, linecolor="gray"
    )
    plt.title(title)
    plt.tight_layout()
    plt.show()

def put_into_table(tbl, i, j, v):
    tbl.loc[i, j] = v
    tbl.loc[j, i] = v


def run_logic(a_path, b_path):
    res = subprocess.run(["./tree_isomorphism", a_path, b_path, "--metrics", args.metrics],
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
    return scores

def parse_args():
    p = argparse.ArgumentParser(
        description="Compare C++ sources by AST-derived similarity metrics.",
        formatter_class=argparse.RawTextHelpFormatter,
    )
    p.add_argument("src", nargs="?", default="./codes",
                   help="Directory with C++ source files (default: ./codes)",)
    p.add_argument("--json-dir", default="generated",
                   help=("Directory for AST .json files. "
                         "Default: 'generated' (inside SRC).")
    )
    p.add_argument("--clean-json", action="store_true",
                   help="Remove generated .json files at the end.",
    )
    p.add_argument("--reuse-json", action="store_true",
                   help="Reuse existing AST .json files; skip regeneration if present.",
    )
    p.add_argument("--markers", action="store_true",
                   help="Enable fragment analysis based on in-source markers.",
    )
    p.add_argument("--metrics", default="STRICT,LEV,TED",
                   help="Comma-separated list of metrics to compute "
                        "(STRICT,LEV,TED). Default: STRICT,LEV,TED"
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
        max_workers = min(cpu_count() or 4, len(filenames))
        with ThreadPoolExecutor(max_workers=max_workers) as ex:
            ex.map(run_parser, filenames, repeat(json_dir), repeat(include_markers), repeat(dir))
    TED_table = pd.DataFrame()
    LEV_table = pd.DataFrame()
    STRICT_table = pd.DataFrame()

    trees_to_check = sorted(f for f in next(walk(json_dir), (None, None, []))[2] if f.endswith(".json"))
    pairs = []
    for i, j in combinations_with_replacement(range(len(trees_to_check)), 2):
        a_file, b_file = trees_to_check[i], trees_to_check[j]
        a_path = os.path.join(json_dir, a_file)
        b_path = os.path.join(json_dir, b_file)
        pairs.append((i, j, a_path, b_path))

    max_workers = min(cpu_count() or 4, len(pairs)) or 1
    with ThreadPoolExecutor(max_workers=max_workers) as ex:
        future2ij = {
            ex.submit(run_logic, a, b): (i, j)
            for (i, j, a, b) in pairs
        }
        for fut in as_completed(future2ij):
            i, j = future2ij[fut]
            scores = fut.result()
            a_file, b_file = trees_to_check[i], trees_to_check[j]
            if "ERROR" in scores:
                print(f"[WARNING]: files {a_file} and {b_file} weren't compared.")

            ted    = scores.get("TED")
            lev    = scores.get("Levenshtein distance")
            strict = scores.get("strict similarity")

            if ted is not None and not math.isnan(ted):
                put_into_table(TED_table, a_file, b_file, ted)
            if lev is not None and not math.isnan(lev):
                put_into_table(LEV_table, a_file, b_file, lev)
            if strict is not None and not math.isnan(strict):
                put_into_table(STRICT_table, a_file, b_file, strict)


    if "TED" in args.metrics:
        TED_table = TED_table.sort_index().sort_index(axis=1)
        print("\n=== TED Table ===")
        print(TED_table.round(2))
        plot_heatmap(TED_table, "Tree Edit Distance (TED)")

    if "LEV" in args.metrics:
        LEV_table = LEV_table.sort_index().sort_index(axis=1)
        print("\n=== Levenshtein Distance Table ===")
        print(LEV_table.round(2))
        plot_heatmap(LEV_table, "Levenshtein Distance")

    if "STRICT" in args.metrics:
        STRICT_table = STRICT_table.sort_index().sort_index(axis=1)
        print("\n=== Strict Similarity Table ===")
        print(STRICT_table.astype(int))
        plot_heatmap(STRICT_table, "Strict Similarity")

    if delete_json_files:
        shutil.rmtree(json_dir)

