# develop notes

KernelSerial.cpp is a tool to merge node in a graph according to the memory and compute complexity. 

the merge is done by the following steps:

1. depth wise merge
merg two nodes in consecutive stages when the previous node has single outgoing edge and the next node has single incoming edge. 

2. breadth wise merge
merge node in the same stage and consider the following conditions
    2.1 node with small memory-plus-compute complexity must be merged to some nodes.
    2.2 the first priority to pick the other node to merge is that merging node will generate a new node that enable depth wise merge, which meeans the new node has single outgoing edge.
    2.3 Sort the nodes in the stage by memory-plus-compute complexity, the second priority to pick the other node to merge is that the node has the largest memory-plus-compute complexity. 
    2.4 repeat the above steps until no more node can be merged. 
 

3. repeat the above steps until no more node can be merged. 


# TODO

1. fix the complexity threshold and complexity computation
2. add the iterative merging
3. visualize the results, before and after merging
4. will iterating in the original node graph cause problem during merging while removing the merged nodes?
5. add global breadth wise merge