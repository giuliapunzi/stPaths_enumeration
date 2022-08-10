# stPaths_enumeration
## Lazy Algorithms for st-Paths Enumeration 
This repository contains the code for several lazy algorithms for st-paths enumeration, plus three classical ones.

The classical algorithms (`stPaths-classical.cpp` and `stPaths-classicalBCC.cpp`) perform binary partition, mantaining in each recursive node a certificate for the existence of a path: for the first algorithm, the certificate is a visit from target node t, while in the second algorithm the certificate is the sequence of BCCs connecting the current node to the target.

We also added an implementation of Johnson's classical st-paths enumeration algorithm, `Johnson.cpp`.

Algorithm `BC-DFS.cpp` is an implemetnation of the hop-constrained algorithm from [this paper](https://www.researchgate.net/publication/351427251_Efficient_Hop-constrained_s-t_Simple_Path_Enumeration) by Peng et al. (2021), where the constraint is set to be virtually infinite, to obtain an enumeration of paths of any length. 

The other `.cpp` files (`exhaustive.cpp`, `stPaths-v0opt.cpp`, `stPaths-v05.cpp`, `stPaths-v075opt.cpp`, `stPaths-v1cpp`) contain increasingly complex lazy algorithms for path enumeration. Files `stPaths-v0.cpp` and `stPaths-v0opt.cpp` contain the same algorithm, but the latter is in a more optimized form. 

All the above algorithms need the input graph in format `.nde`.
Each `.cpp` file is compilable on its own. After regular compilation such as `g++ -O3 stPaths-whichever.cpp -o whichever`, the output executable will take four parameters as input; in order: 
- FILEDIRECTORY: directory of the input `.nde` file containing the desired input graph
- source: index of the source node (indices go from 0 to N-1, where N is the number of nodes of the graph)
- target: index of the target node (indices go from 0 to N-1, where N is the number of nodes of the graph)
- MAX_TIME: max timeout in seconds. 

An example execution for finding paths from node 5 to node 782 of graph `inputgraph.nde` with timeout of 10 minutes is as follows:  
```
./whichever ./dataset/inputgraph.nde 5 782 600
```
When terminating, the algorithm will report the following parameters: elapsed time, number of recursive calls carried out, number of visits of the graph performed, number of paths found, number of dead ends. 


The last algorithm `find-st.cpp` takes as input a graph in `.nde` format and it randomly tests pairs of nodes to find valid sources and targets (i.e. there is at least one path connecting the source to the target). It can be normally compiled as `g++ -O3 find-st.cpp -o executable_name`; the resulting executable takes just one input, the FILEDIRECTORY of the desired input graph. Thus, to execute it:
```
./executable_name ./dataset/inputgraph.nde
```
When terminating, it will output the indices of two nodes that can be used as source and target for the other algorithms.