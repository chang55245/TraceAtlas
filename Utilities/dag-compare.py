from queue import Full
import networkx as nx
import matplotlib.pyplot as plt
import json

def read_dag_from_json(filepath):
    with open(filepath, 'r') as f:
        return json.load(f)

def visualize_dag(nodes, ax):
    G = nx.DiGraph()
    sorted_nodes = sorted(nodes.items(), key=lambda x: int(''.join(filter(str.isdigit, x[0]))))
    # Add nodes and edges to the graph
    for node, info in sorted_nodes:
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

    for stage, nodes_in_stage in stage_nodes.items():
        if any(G.nodes[node]['kernel'] == 1 for node in nodes_in_stage):
            x_values = [pos[node][0] for node in nodes_in_stage]
            y_min = min([pos[node][1] for node in nodes_in_stage]) - 0.5
            y_max = max([pos[node][1] for node in nodes_in_stage]) + 0.5
            ax.fill_betweenx([y_min, y_max], min(x_values) - 0.75, max(x_values) + 0.75, color='#d1e7ff', alpha=0.5)
    
    # draw text of merged_from for each node and info['merged_from'] != []
    for node, info in sorted_nodes:
        if 'merged_from' in info  > 0:
            ax.text(pos[node][0], pos[node][1], f"merged_from: {info['merged_from']}", fontsize=12, ha='center', va='center')

    # Draw the graph on the specified axes
    ax.fill_betweenx([0, 0], 0, 0, color='white')  # Clear the axes
    nx.draw(G, pos, ax=ax, with_labels=True, node_color="skyblue", node_size=2000, font_size=14, font_weight="bold")
    nx.draw_networkx_edges(
        G,
        pos,
        ax=ax,
        arrows=True,
        arrowstyle='-|>',
        arrowsize=30,
        edge_color='gray',
        min_target_margin=20,
        width=3
    )
    
    # Make sure spines are visible
    ax.spines['top'].set_visible(True)
    ax.spines['right'].set_visible(True)
    ax.spines['bottom'].set_visible(True)
    ax.spines['left'].set_visible(True)

def compare_dags(dag1_path, dag2_path):
    # Read both DAGs
    dag1 = read_dag_from_json(dag1_path)
    dag2 = read_dag_from_json(dag2_path)
    
    # Create a single figure with subplots
    fig, axs = plt.subplots(1, 2, figsize=(14, 10))

    # Visualize DAG 1
    visualize_dag(dag1, axs[0])
    axs[0].set_title(f"DAG 1: {dag1_path}")

    # Visualize DAG 2
    visualize_dag(dag2, axs[1])
    axs[1].set_title(f"DAG 2: {dag2_path}")

    # Add rectangular frames around each subplot
    for ax in axs:
        # Make sure spines are visible and styled
        for spine in ax.spines.values():
            spine.set_visible(True)
            spine.set_edgecolor('black')
            spine.set_linewidth(2)
        
        # Set axis on
        ax.set_axis_on()

    # Adjust layout to add space between subplots
    plt.subplots_adjust(wspace=0.4)  # Adjust the width space between subplots

    # Use tight_layout to improve spacing
    plt.tight_layout()

    # Save the visualizations
    plt.savefig("dags_comparison.png")

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
# }