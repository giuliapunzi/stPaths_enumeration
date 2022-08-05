# stPaths_enumeration
## Lazy Algorithms for st-Paths Enumeration 
This repository contains the code for six lazy algorithms for st-paths enumeration, plus a classical one.

The classical algorithm (`stPaths-classical.cpp`) performs a binary partition, mantaining in each recursive node a certificate for the existence of a path in the form of a DFS from target node t.
The other `.cpp` files (`stPaths-v0.cpp`, `stPaths-v05.cpp`, `stPaths-v075.cpp`, `stPaths-v075opt.cpp`, `stPaths-v095.cpp`) contain increasingly complex lazy algorithms for path enumeration. All the algorithms need the input graph in format `.nde`.

Each `.cpp` file is compilable on its own. After regular compilation such as `g++ -O3 stPaths-whichever.cpp -o whichever`, the output executable will need to be run followed by the directory of the input `.nde` file containing the desired input graph: `./whichever ./dataset/inputgraph.nde`
The executable will then run the corresponding algorithm on graph inputgraph, considering the first node as source and the last node as target, asking the user to set a timeout in milliseconds.

When terminating, the algorithm will report the following parameters: elapsed time, number of recursive calls carried out, number of visits of the graph performed, number of paths found, number of dead ends, time spent in the visits of the graph. 
It is also possible for the user to decide whether the produced parameters are also to be stored in a `.txt` file (created in the current directory). 
