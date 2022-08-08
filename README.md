# stPaths_enumeration
## Lazy Algorithms for st-Paths Enumeration 
This repository contains the code for five lazy algorithms for st-paths enumeration, plus two classical ones.

The classical algorithms (`stPaths-classical.cpp` and `stPaths-classicalBCC.cpp`) perform binary partition, mantaining in each recursive node a certificate for the existence of a path: for the first algorithm, the certificate is a visit from target node t, while in the second algorithm the certificate is the sequence of BCCs connecting the current node to the target.
The other `.cpp` files (`stPaths-v0.cpp`, `stPaths-v05.cpp`, `stPaths-v075opt.cpp`, `stPaths-v1cpp`, `stPaths-v1opt.cpp`) contain increasingly complex lazy algorithms for path enumeration. All the algorithms need the input graph in format `.nde`.

Each `.cpp` file is compilable on its own. After regular compilation such as `g++ -O3 stPaths-whichever.cpp -o whichever`, the output executable will need to be run followed by the directory of the input `.nde` file containing the desired input graph: `./whichever ./dataset/inputgraph.nde`
The executable will then run the corresponding algorithm on graph inputgraph, asking the user for the indices of the source node s and of the target node t, and asking also to set a timeout in seconds.

When terminating, the algorithm will report the following parameters: elapsed time, number of recursive calls carried out, number of visits of the graph performed, number of paths found, number of dead ends, time spent in the visits of the graph. 
It is possible for the user to decide whether the produced parameters are also to be stored in a `.txt` file (created in the current directory). 
