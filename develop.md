# develop notes

KernelSerial.cpp is a tool to merge node in a graph according to the memory and compute complexity. 

the merge is done by the following steps:

1. depth wise merge
merg two nodes in consecutive stages when the previous node has single outgoing edge and the next node has single incoming edge. 

2. breadth wise merge
merge node in the same stage and consider the following conditions
    2.1 node with small memory-plus-compute complexity must be merged to some nodes to make the complexity of the node larger than the threshold.
    2.2 the first priority to merge the current node with a node is that the node has a common next node with the current node.
    2.3 Sort the nodes in the stage by memory-plus-compute complexity, the second priority to merge is that the node has the largest memory-plus-compute complexity if the node has no common next node with the current node. 
    2.4 repeat the above steps until no more node can be merged. 
    2.5 handle corner cases
        2.5.1 what if after all possible merging the node still has small memory-plus-compute complexity?, just stop the merge.
 

3. repeat the above steps until no more node can be merged. 


