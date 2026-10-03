import json
import os
import matplotlib.pyplot as plt
from collections import defaultdict


# LOAD JSON LINES
def load_json_lines(file):
    data = []
    with open(file, 'r') as f:
        for line in f:
            if line.strip():
                data.append(json.loads(line))
    return data


# SAFE AVERAGE
def safe_avg(values):
    if not values:
        return None
    return sum(values) / len(values)


# AGGREGATE (generic)
def aggregate(data, key):
    grouped = defaultdict(list)

    for d in data:
        grouped[d[key]].append(d)

    result = {}

    for k, values in grouped.items():
        hybrid = [v["hybrid_time"] for v in values if v["hybrid_time"] >= 0]
        dsu = [v["dsu_time"] for v in values if v["dsu_time"] >= 0]

        result[k] = {
            "hybrid": safe_avg(hybrid),
            "dsu": safe_avg(dsu)
        }

    return result


# CREATE RESULTS FOLDER
def ensure_results_dir():
    os.makedirs("results", exist_ok=True)


# EXP1: Runtime vs Size
# → separate plot per graph type
def plot_exp1(file):
    data = load_json_lines(file)

    # group: type → size → times
    grouped = defaultdict(lambda: defaultdict(lambda: {"hybrid": [], "dsu": []}))

    for d in data:
        gtype = d["graph_type"]
        size = d["param"]

        grouped[gtype][size]["hybrid"].append(d["hybrid_time"])
        grouped[gtype][size]["dsu"].append(d["dsu_time"])

    # create separate plot per graph type
    for gtype in grouped:

        sizes = sorted(grouped[gtype].keys())

        hybrid = [safe_avg(grouped[gtype][s]["hybrid"]) for s in sizes]
        dsu = [safe_avg(grouped[gtype][s]["dsu"]) for s in sizes]

        plt.figure()
        plt.plot(sizes, hybrid, marker='o', label="Hybrid")
        plt.plot(sizes, dsu, marker='s', label="DSU")

        plt.xlabel("Number of Nodes")
        plt.ylabel("Time (ms)")
        plt.title(f"Exp1: Runtime vs Size ({gtype})")
        plt.legend()
        plt.grid()
        plt.yscale("log")

        filename = f"results/exp1_size_{gtype}.png"
        plt.savefig(filename)
        plt.close()

        print(f"Saved {filename}")


# EXP2: Runtime vs Graph Type
def plot_exp2(file):
    data = load_json_lines(file)
    grouped = aggregate(data, "graph_type")

    types = sorted(grouped.keys())

    hybrid = [grouped[t]["hybrid"] for t in types]
    dsu = [grouped[t]["dsu"] for t in types]

    x = range(len(types))

    plt.figure()
    plt.bar(x, hybrid, width=0.4, label="Hybrid")
    plt.bar([i + 0.4 for i in x], dsu, width=0.4, label="DSU")

    plt.xticks([i + 0.2 for i in x], types)
    plt.ylabel("Time (ms)")
    plt.title("Exp2: Runtime vs Graph Type")
    plt.legend()
    plt.yscale("log")

    plt.savefig("results/exp2_runtime_vs_type.png")
    plt.close()


# EXP3: Runtime vs Density
def plot_exp3(file):
    data = load_json_lines(file)
    grouped = aggregate(data, "param")

    densities = sorted(grouped.keys())

    hybrid = [grouped[d]["hybrid"] for d in densities]
    dsu = [grouped[d]["dsu"] for d in densities]

    plt.figure()
    plt.plot(densities, hybrid, marker='o', label="Hybrid")
    plt.plot(densities, dsu, marker='s', label="DSU")

    plt.xlabel("Density")
    plt.ylabel("Time (ms)")
    plt.title("Exp3: Runtime vs Density")
    plt.legend()
    plt.grid()
    plt.yscale("log")

    plt.savefig("results/exp3_runtime_vs_density.png")
    plt.close()


# EXP4: Runtime vs Components
def plot_exp4(file):
    data = load_json_lines(file)
    grouped = aggregate(data, "graph_type")

    types = sorted(grouped.keys())

    hybrid = [grouped[t]["hybrid"] for t in types]
    dsu = [grouped[t]["dsu"] for t in types]

    x = range(len(types))

    plt.figure()
    plt.bar(x, hybrid, width=0.4, label="Hybrid")
    plt.bar([i + 0.4 for i in x], dsu, width=0.4, label="DSU")

    plt.xticks([i + 0.2 for i in x], types)
    plt.ylabel("Time (ms)")
    plt.title("Exp4: Runtime vs Component Distribution")
    plt.legend()
    plt.yscale("log")

    plt.savefig("results/exp4_runtime_vs_components.png")
    plt.close()


# EXP5: Speedup vs Threads
def plot_exp5(file):
    data = load_json_lines(file)
    grouped = aggregate(data, "param")

    threads = sorted(grouped.keys())

    hybrid = [grouped[t]["hybrid"] for t in threads]
    dsu = [grouped[t]["dsu"] for t in threads]

    base_h = hybrid[0]
    base_d = dsu[0]

    speedup_h = [base_h / t for t in hybrid]
    speedup_d = [base_d / t for t in dsu]

    plt.figure()
    plt.plot(threads, speedup_h, marker='o', label="Hybrid")
    plt.plot(threads, speedup_d, marker='s', label="DSU")

    plt.xlabel("Threads")
    plt.ylabel("Speedup")
    plt.title("Exp5: Speedup vs Threads")
    plt.legend()
    plt.grid()

    plt.savefig("results/exp5_speedup_vs_threads.png")
    plt.close()


# AUTO RUN ALL
if __name__ == "__main__":

    ensure_results_dir()

    experiments = {
        "exp1_size": plot_exp1,
        "exp2_type": plot_exp2,
        "exp3_density": plot_exp3,
        "exp4_components": plot_exp4,
        "exp5_threads": plot_exp5,
    }

    for exp, func in experiments.items():
        path = f"../logs/{exp}/runtime.json"

        if os.path.exists(path):
            print(f"Processing {exp}...")
            func(path)
        else:
            print(f"Skipping {exp} (no data found)")

    print("\nAll plots saved in 'results/' folder")