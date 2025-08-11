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
        subprocess.run(["python3", "./ast-parser/ast_parser.py", outfile_path, outfile_path, include_markers ])

def plot_heatmap(data, title, cmap="coolwarm"):
    plt.figure(figsize=(8, 6))
    sns.heatmap(data.astype(float), annot=True, fmt=".2f", cmap=cmap, vmin=0, vmax=1, linewidths=0.5, linecolor='gray')
    plt.title(title)
    plt.tight_layout()
    plt.show()

def put_into_table(tbl, i, j, v):
    tbl.loc[i, j] = v
    tbl.loc[j, i] = v

if __name__ == '__main__':
    codes_dir = input("Provide codes directory (default: ./codes): ")
    dir = codes_dir if codes_dir != "" else "./codes"
    json_dir = os.path.join(dir, "generated")
    delete_json_files = input("Delete isomorphism trees after creating tables? (y/N): ").lower() == "y"
    try:
        os.makedirs(json_dir, exist_ok=True)
    except Exception:
        print("Couldn't create directory {}".format(json_dir))
        sys.exit(1)
    filenames = next(walk(dir), (None, None, []))[2]
    jsonify = input("Do you need to generate new json files to check codes? (Y/n): ").lower() != "n"
    if jsonify:
        include_markers = input("Check codes based on code markers? (y/N): ").lower()
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

