import networkx as nx
import matplotlib.pyplot as plt
import  json
file = open("CEDR-DAG/pulse_doppler.json")
inputjs = json.load(file)

edges = []
for edge in inputjs["DAGEdge"]:
    edges.append(edge)


nodePosIn = []
for pos in inputjs["NodePosition"]:
    nodePosIn.append(pos)

labels = {}

for pos in inputjs["KernelInstanceMap"]:
    if pos[1] == "-1":
        labels[pos[0]] = ''
    else:
        labels[pos[0]] = pos[1]

G = nx.DiGraph()
G.add_edges_from(edges)


xposCounter = {}
for pos in nodePosIn:
    if pos[1] not in xposCounter:
        xposCounter[pos[1]] = 0
    else:
        value = xposCounter[pos[1]]
        xposCounter[pos[1]] = value + 1
    G.add_node(pos[0], pos=(pos[1], xposCounter[pos[1]] if pos[1] in xposCounter else 0 ))
    G.add_node(pos[0], labelPos=(pos[1], (xposCounter[pos[1]] if pos[1] in xposCounter else 0)-0.2))

posNode=nx.get_node_attributes(G,'pos')
posLabel=nx.get_node_attributes(G,'labelPos')
options = {
    "font_size": 36,
    "node_size": 3000,
    "node_color": "white",
    "edgecolors": "black",
    "linewidths": 5,
    "width": 5,
}

nx.draw_networkx(G, posNode,with_labels = True, edge_color = 'b', arrowsize=25,  **options)


nx.draw_networkx_labels(G, posLabel, labels=labels,font_size=25,font_color='r' )
# Set margins for the axes so that nodes aren't clipped
ax = plt.gca()
ax.margins(0.20)

plt.axis("off")
plt.show()