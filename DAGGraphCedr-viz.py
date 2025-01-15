from typing import Dict, List, Any, Union

import networkx as nx
import matplotlib.pyplot as plt
import json
from networkx.drawing.nx_pydot import write_dot


class CurrentMax:
    def __init__(self, initial_max_edge, initial_max_stage):
        self.edge = initial_max_edge
        self.stage = initial_max_stage

    def update_current_max(self, max_edge, max_stage):
        self.edge = max_edge
        self.stage = max_stage


class PrevNodeSet:
    def __init__(self, initial_edge, initial_stage):
        self.currentMax = CurrentMax(initial_edge, initial_stage)
        self.Nodes = [initial_edge[0]]


file = open("CEDR-DAG/testCase.json")
inputjs = json.load(file)

labels = {}

for node in inputjs["KernelInstanceMap"]:
    if node[1] == "-1":
        labels[node[0]] = ''
    else:
        labels[node[0]] = node[1]

edges = []
for edge in inputjs["DAGEdge"]:
    edges.append(edge)

nodePosIn = []
nodeStage = {}
for node in inputjs["nodeIndexStage"]:
    nodePosIn.append(node)
    nodeStage[node[0]] = node[1]

prevNode = {}
to_be_removed_edge = []
for edge in edges:
    if edge[1] not in prevNode:
        prevNode[edge[1]] = PrevNodeSet(edge, nodeStage[edge[0]])
    else:
        # I have seen this node
        # update node
        # update current max
        # -- remove the existing max edge
        prevNode[edge[1]].Nodes.append(edge[0])
        if prevNode[edge[1]].currentMax.stage < nodeStage[edge[0]]:
            prevNode[edge[1]].currentMax.update_current_max(edge, nodeStage[edge[0]])
            for node in prevNode[edge[1]].Nodes:
                if nodeStage[node] < prevNode[edge[1]].currentMax.stage:
                    # delete duplicated
                    remove_edge = [node, edge[1]]
                    if remove_edge not in to_be_removed_edge and labels[node] == '':
                        to_be_removed_edge.append(remove_edge)

for edge in to_be_removed_edge:
    edges.remove(edge)
G = nx.DiGraph()
G.add_edges_from(edges)

# node position in the graph
xposCounter = {}
for pos in nodePosIn:
    if pos[1] not in xposCounter:
        xposCounter[pos[1]] = 0
    else:
        value = xposCounter[pos[1]]
        xposCounter[pos[1]] = value + 1
    G.add_node(pos[0], pos=(pos[1], xposCounter[pos[1]] if pos[1] in xposCounter else 0), kernel="fft")
    G.add_node(pos[0], labelPos=(pos[1], (xposCounter[pos[1]] if pos[1] in xposCounter else 0) - 0.2))

posNode = nx.get_node_attributes(G, 'pos')

# posNode = nx.nx_agraph.pygraphviz_layout(G)
posLabel = nx.get_node_attributes(G, 'labelPos')
options = {
    "font_size": 36,
    "node_size": 3000,
    "node_color": "white",
    "edgecolors": "black",
    "linewidths": 5,
    "width": 5,
}

nx.draw_networkx(G, posNode, with_labels=True, edge_color='b', arrowsize=25,
                 connectionstyle="arc3,rad=0.1", **options)
# nx.draw_networkx(G, posNode, with_labels=True, edge_color='b', arrowsize=25,
#                  **options)

nx.draw_networkx_labels(G, posLabel, labels=labels, font_size=25, font_color='r')
# Set margins for the axes so that nodes aren't clipped
ax = plt.gca()
ax.margins(0.20)

write_dot(G, 'test.dot')
nx.write_gml(G, 'network.gml')

print(nodeStage)
print(edges)
plt.axis("off")

plt.show()
