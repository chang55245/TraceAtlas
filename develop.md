# develop notes


## the merge algorithm
KernelSerial.cpp contains a merge algorithm to merge node in a graph according to the memory and compute complexity. 


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


## schedule algorithm

KernelSerial.cpp contains a schedule algorithm to schedule the merged nodes. 

1. use a map of vectors [merged_node_id, vector<schedule of nodes the merged node is composed of>] to store the schedule of each merged node. use the original dag to get the schedule of child nodes.
2. schedule the merged node use the merged dag
3. generate the overall schedule of the merged dag in terms of the original node id. 

## Task Merging Reorder
The TaskMergingReorder pass is used to reorder the merged nodes according to the schedule of each child node. 
1. generate the branch map for the merged dag according to the schedule of each child node. 
2. add task segmentation flags for code generation. 

## program correctness
1. need to prevent the start node and end node from being merged and reordered. they need to be the first and last node in the schedule and excuted in serial. 