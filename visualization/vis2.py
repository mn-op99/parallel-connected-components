import networkx as nx
import random


# -------------------------------
# GRAPH GENERATORS (same as yours)
# -------------------------------
def generate_dense(n, p):
    edges = []
    for i in range(n):
        for j in range(i + 1, n):
            if random.random() < p:
                edges.append((i, j))
    return edges


def generate_medium(n):
    G = nx.barabasi_albert_graph(n, 2)
    return list(G.edges())


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

    if graph_type == "dense":
        edges = generate_dense(size, 0.1)

    elif graph_type == "sparse":
        edges = generate_sparse(size)

    elif graph_type == "medium":
        edges = generate_medium(size)

    elif graph_type == "balanced":
        e1 = generate_dense(size // 2, 0.05)
        e2 = generate_medium(size // 3)
        e3 = generate_sparse(size // 3)

        def offset_edges(e, off):
            return [(u + off, v + off) for u, v in e]

        edges.extend(offset_edges(e1, 0))
        edges.extend(offset_edges(e2, size // 2))
        edges.extend(offset_edges(e3, size // 2 + size // 3))

    return edges


# -------------------------------
# ANALYSIS FUNCTION
# -------------------------------
def analyze_graph(edges, name):

    G = nx.Graph()
    G.add_edges_from(edges)

    num_nodes = G.number_of_nodes()
    num_edges = G.number_of_edges()

    # Average degree = 2m / n
    avg_degree = (2 * num_edges) / num_nodes if num_nodes > 0 else 0

    # Connected components
    components = list(nx.connected_components(G))
    num_components = len(components)

    # Sort components by size (descending)
    comp_sizes = sorted([len(c) for c in components], reverse=True)

    top_10 = comp_sizes[:10]

    # -------------------------------
    # PRINT RESULTS
    # -------------------------------
    print("\n==============================")
    print(f"Graph Type: {name}")
    print(f"Nodes: {num_nodes}")
    print(f"Edges: {num_edges}")
    print(f"Average Degree: {avg_degree:.2f}")
    print(f"Number of Components: {num_components}")
    print(f"Top 10 Component Sizes: {top_10}")
    print("==============================")


# -------------------------------
# MAIN
# -------------------------------
if __name__ == "__main__":

    random.seed(42)

    print("🔍 Generating and analyzing graphs...\n")

    graph_types = ["dense", "sparse", "medium", "balanced"]

    for gtype in graph_types:
        edges = build_graph(gtype, 120)
        analyze_graph(edges, gtype)

    print("\n✅ Done!")