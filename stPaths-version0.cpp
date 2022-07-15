#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <algorithm>
#include <list> 
#include <stack> 
#include <stdexcept>

using namespace std;
vector<int> deleted;

typedef vector<vector<int>> graph;
graph G;

// create graph from file filename 
graph create_graph(string filename)
{
    ifstream input_graph;
    input_graph.open(filename);
    
    int N, M;

    // file contains number of nodes, number of edges at fist line
    // and then one edge per line
    input_graph >> N >> M;

    G.resize(N);
    deleted.resize(N);

    int u, v;
    for(int i=0; i<M; i++)
    {
        input_graph >> u >> v;
        G[u].push_back(v);
        G[v].push_back(u);
    }

    input_graph.close();
    return G;
}
 


// ++++++++++++++++++++++++++++++++++ GRAPH REPORTING ++++++++++++++++++++++++++++++++

// checks if a given edge (u,v) belongs to graph G
inline bool is_edge(int u, int v)
{   
    if(deleted[u] || deleted[v])
        return false;
    
    if(find(G[u].begin(), G[u].end(), v) == G[u].end())
        return false;
    else
        return true;
}


// returns degree of node u
// remember: need to take into account deleted edges
inline int degree(int u)
{
    int deg = 0;
    if(deleted[u])
        return -1;
    else{
        for(int i = 0; i< G[u].size(); i++){
            if(!deleted[G[u][i]])
                deg++;
        }
    }

    return deg;
}


// print graph as list of adjacency lists
inline void printGraph()
{
    cout << endl;
    // for each node, look through its adjacency list
    for(int i =0; i < G.size(); i++)
    {
        if(!deleted[i]){
            cout << i << ": ";
            for (int j = 0; j< G[i].size(); j++){
                if(!deleted[G[i][j]])
                    cout << '(' << i << ", " << G[i][j] << "); ";
            }
            cout << endl;
        }
        
    }
    cout << endl;
}

inline void printDeleted(){
    for(int i= 0; i<deleted.size(); i++){
        if(deleted[i])
            cout << i << " ";
    }
    cout << endl;
}

// ++++++++++++++++++++++++++++++++++ GRAPH MODIFIERS ++++++++++++++++++++++++++++++++

// remove node u from the graph, by marking deleted vector to 0
inline void remove_node(int u)
{
    deleted[u] = 1;
    return;
}


// ++++++++++++++++++++++++++++++++ GRAPH VISITS +++++++++++++++++++++++++++++++

// recursive DFS procedure from node u
void DFS(int u, vector<bool> &visited){
    // cout << "Entering DFS for " << u << endl << flush;
    visited[u] = true;

    for(int i = 0; i < G[u].size(); i++)
    {
        if(!visited[G[u][i]] && !deleted[G[u][i]])
            DFS(G[u][i], visited);
    }

    return;
}

// starts a visit from t, and marks as good neighbors the neighbors of s that
// are reached through the visit. Outputs the vector of these good neighbors.
vector<int> check_neighbors(int s, int t){
    vector<bool> visited(G.size());

    // before starting, mark s as deleted
    deleted[s] = 1;

    // initialize all deleted nodes as visited
    for(int i = 0; i< visited.size(); i++){
        if(deleted[i])
            visited[i] = true;
        else
            visited[i] = false;
    }

    // for(auto x : visited)
    //     cout << x << " " << flush;
    // cout << endl;

    // launch DFS from node t
    DFS(t, visited);

    // find out which neighbors of s have been visited, and output them
    vector<int> good_neighbors;
    for(int i = 0; i < G[s].size(); i++){
        if(visited[G[s][i]])
            good_neighbors.push_back(G[s][i]);
    }

    // undo deletion of s 
    deleted[s] = 0;

    return good_neighbors;
}


int main(){

    return 0;
}
