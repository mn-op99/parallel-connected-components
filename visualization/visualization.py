import networkx as nx
import matplotlib.pyplot as plt
import random
import os


# -------------------------------
# ENSURE RESULTS DIR
# -------------------------------
def ensure_results_dir():
    os.makedirs("results", exist_ok=True)


# -------------------------------
# DENSE GRAPH (probabilistic)
# -------------------------------
def generate_dense(n, p):
    edges = []
    for i in range(n):
        for j in range(i + 1, n):
            if random.random() < p:
                edges.append((i, j))
    return edges


# -------------------------------
# MEDIUM (preferential attachment)
# -------------------------------
def generate_medium(n):
    G = nx.barabasi_albert_graph(n, 2)
    return list(G.edges())


# -------------------------------
# SPARSE (tree-like)
# -------------------------------
def generate_sparse(n, extra_prob=0.2):
    edges = []
    for i in range(1, n):
        parent = random.randint(0, i - 1)
        edges.append((i, parent))

        if i > 5 and random.random() < extra_prob:
            extra = random.randint(0, i - 1)
            edges.append((i, extra))
    return edges


# -------------------------------
# BUILD GRAPH TYPES
# -------------------------------
def build_graph(graph_type, size=100):

    edges = []
    offset = 0

    if graph_type == "dense":
        edges = generate_dense(size, 0.1)

    elif graph_type == "sparse":
        edges = generate_sparse(size)

    elif graph_type == "balanced":
        # mix of all three
        e1 = generate_dense(size // 2, 0.05)
        e2 = generate_medium(size // 3)
        e3 = generate_sparse(size // 3)

        # adjust offsets
        def offset_edges(e, off):
            return [(u + off, v + off) for u, v in e]

        edges.extend(offset_edges(e1, 0))
        edges.extend(offset_edges(e2, size // 2))
        edges.extend(offset_edges(e3, size // 2 + size // 3))

    return edges


# -------------------------------
# VISUALIZE GRAPH
# -------------------------------
def visualize(edges, name):
    ensure_results_dir()

    G = nx.Graph()
    G.add_edges_from(edges)

    pos = nx.spring_layout(G, seed=42)

    plt.figure(figsize=(6, 6))
    nx.draw(G, pos, node_size=50, width=0.5)

    plt.title(name)
    plt.savefig(f"results/{name}.png")
    plt.close()


# -------------------------------
# DEGREE DISTRIBUTION
# -------------------------------
def plot_degree_distribution(edges, name):
    ensure_results_dir()

    G = nx.Graph()
    G.add_edges_from(edges)

    degrees = [deg for _, deg in G.degree()]

    plt.figure()
    plt.hist(degrees, bins=20)

    plt.xlabel("Degree")
    plt.ylabel("Frequency")
    plt.title(f"Degree Distribution - {name}")

    plt.savefig(f"results/{name}_degree.png")
    plt.close()


# -------------------------------
# MAIN
# -------------------------------
if __name__ == "__main__":

    print("Generating synthetic graphs...")

    # Dense
    dense_edges = build_graph("dense", 120)
    visualize(dense_edges, "dense_graph")
    plot_degree_distribution(dense_edges, "dense")

    # Sparse
    sparse_edges = build_graph("sparse", 120)
    visualize(sparse_edges, "sparse_graph")
    plot_degree_distribution(sparse_edges, "sparse")

    # Balanced
    balanced_edges = build_graph("balanced", 120)
    visualize(balanced_edges, "balanced_graph")
    plot_degree_distribution(balanced_edges, "balanced")

    print("✅ Graphs saved in 'results/' folder")