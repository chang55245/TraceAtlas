import networkx as nx
import matplotlib.pyplot as plt
import json

def read_dag_from_json(filepath):
    with open(filepath, 'r') as f:
        return json.load(f)

def visualize_dag(nodes):
    G = nx.DiGraph()

    # Add nodes and edges to the graph
    for node, info in nodes.items():
        G.add_node(node, stage=info['stage'], kernel=info.get('kernel', 0))
        if info['next'] is not None:
            for next_node in info['next']:
                G.add_edge(node, next_node)

    # Get positions for horizontal layout
    pos = {}
    stage_nodes = {}
    for node, stage in nx.get_node_attributes(G, 'stage').items():
        if stage not in stage_nodes:
            stage_nodes[stage] = []
        stage_nodes[stage].append(node)

    x_offset = 0
    for stage, nodes_in_stage in sorted(stage_nodes.items()):
        y_offset = len(nodes_in_stage) / 2
        for i, node in enumerate(nodes_in_stage):
            pos[node] = (x_offset, -i + y_offset)
        x_offset += 2

    # Draw the graph
    plt.figure(figsize=(14, 10))

    # Shade stages with kernel=1
    for stage, nodes_in_stage in stage_nodes.items():
        if any(G.nodes[node]['kernel'] == 1 for node in nodes_in_stage):
            x_values = [pos[node][0] for node in nodes_in_stage]
            y_min = min([pos[node][1] for node in nodes_in_stage]) - 0.5
            y_max = max([pos[node][1] for node in nodes_in_stage]) + 0.5
            plt.fill_betweenx([y_min, y_max], min(x_values) - 0.75, max(x_values) + 0.75, color='#d1e7ff', alpha=0.5)

    # Draw nodes and edges
    nx.draw(G, pos, with_labels=True, node_color="skyblue", node_size=2000, font_size=14, font_weight="bold")
    nx.draw_networkx_edges(
        G,
        pos,
        arrows=True,  # Ensure arrows are shown
        arrowstyle='-|>',  # Defines the arrow style (optional)
        arrowsize=30,  # Controls the size of the arrow head
        edge_color='gray',
        min_target_margin=20,
        width=3
    )

    # Save and show the graph
    plt.title("DAG Visualization with Kernel Stages Highlighted")
    plt.savefig("dag_visualization.svg")
    plt.show()


def compare_dags(dag1_path, dag2_path):
    # Read both DAGs
    dag1 = read_dag_from_json(dag1_path)
    dag2 = read_dag_from_json(dag2_path)
    
    # Create visualizations for both
    plt.figure(1)
    visualize_dag(dag1)
    plt.title(f"DAG 1: {dag1_path}")
    
    plt.figure(2)
    visualize_dag(dag2)
    plt.title(f"DAG 2: {dag2_path}")
    
    # Save the visualizations
    plt.figure(1)
    plt.savefig("dag1_visualization.svg")
    plt.figure(2)
    plt.savefig("dag2_visualization.svg")
    plt.show()

if __name__ == "__main__":
    import sys
    if len(sys.argv) != 3:
        print("Usage: python3 dag-compare.py <dag1.json> <dag2.json>")
        sys.exit(1)
    
    compare_dags(sys.argv[1], sys.argv[2])

# # Example DAG input
# nodes = {
#     'A': {'stage': 0, 'next': ['B', 'C']},
#     'B': {'stage': 1, 'next': ['D', 'E'], 'kernel': 1},
#     'C': {'stage': 1, 'next': ['F', 'G']},
#     'D': {'stage': 2, 'next': ['H']},
#     'E': {'stage': 2, 'next': ['I', 'J']},
#     'F': {'stage': 2, 'next': ['K']},
#     'G': {'stage': 2, 'next': ['L', 'M']},
#     'H': {'stage': 3, 'next': ['N'], 'kernel': 1},
#     'I': {'stage': 3, 'next': []},
#     'J': {'stage': 3, 'next': ['O', 'P']},
#     'K': {'stage': 3, 'next': []},
#     'L': {'stage': 3, 'next': ['Q']},
#     'M': {'stage': 3, 'next': []},
#     'N': {'stage': 4, 'next': ['R']},
#     'O': {'stage': 4, 'next': []},
#     'P': {'stage': 4, 'next': ['S']},
#     'Q': {'stage': 4, 'next': ['T']},
#     'R': {'stage': 5, 'next': []},
#     'S': {'stage': 5, 'next': []},
#     'T': {'stage': 5, 'next': []}
# }

# # Visualize the DAG
# visualize_dag(nodes)