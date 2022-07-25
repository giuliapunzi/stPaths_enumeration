#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stack>

using namespace std;
vector<int> deleted;
// vector<int> degree; // keep global degree vector, updating it as deletions and insertions go

stack<int> del_stack; // stack of nodes that have been deleted

typedef vector<vector<int>> graph;
graph G;

vector<int> reachable; // marks nodes that have been visited by the DFS
// reachable[i] = -1 if it was a deleted node; = 0 if not reached through the visit and =1 otherwise



bool is_edge(int u, int v);

// create graph from file filename 
// WE REMOVE SELF LOOPS
graph create_graph(char* filename)
{
    FILE* input_graph = fopen(filename, "r");
    // input_graph.open(filename);
    
    int N, M;

    // file contains number of nodes, number of edges at fist line
    // and then one edge per line
    // input_graph >> N >> M;
    fscanf(input_graph, "%d %d", &N, &M);

    int real_edges = 0;

    G.resize(N);
    deleted.resize(N);

    int u, v;
    for(int i=0; i<M; i++)
    {
        fscanf(input_graph, "%d,%d", &u, &v);
        // input_graph >> u >> v;
        // make sure no self-loops or multiedges are created 
        if (u != v && !is_edge(u,v)){
            G[u].push_back(v);
            G[v].push_back(u);
            real_edges++;
        }
        
    }

    cout << "Input graph has " << N << " nodes and " << real_edges << " edges. "<< endl;

    // also initialize reachable vector to be all 1
    reachable.resize(G.size(), 1);
    for(int i = 0; i < G.size() ; i++)
        reachable[i] = 1;

    fclose(input_graph);
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
        return 0;
    else{
        for(int i = 0; i< G[u].size(); i++){
            if(!deleted[G[u][i]])
                deg++;
        }
    }

    return deg;
}

// outputs the vector of (non removed) neighbors of u
// NOTE: u can be a removed node
inline vector<int> neighbors(int u)
{
    vector<int> neigh; 
    for(int i = 0; i<G[u].size(); i++){
        if(!deleted[G[u][i]])
            neigh.push_back(G[u][i]);
    }

    return neigh;
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
    del_stack.push(u);
    return;
}


// ++++++++++++++++++++++++++++++++ GRAPH VISITS +++++++++++++++++++++++++++++++

// recursive DFS procedure from node u
void DFS(int u){
    // cout << "Entering DFS for " << u << endl << flush;
    reachable[u] = 1;

    for(int i = 0; i < G[u].size(); i++)
    {
        // reachable being equal to zero means that the node has not been deleted nor already visited
        if(reachable[G[u][i]]==0)
            DFS(G[u][i]);
    }

    return;
}

// starts a visit from t, and marks as reachable the nodes that
// are reached through the visit.
void reachability_check(int t){
    reachable.resize(G.size());

    // initialize all deleted nodes as visited
    for(int i = 0; i< reachable.size(); i++){
        if(deleted[i])
            reachable[i] = -1;
        else
            reachable[i] = 0;
    }

    // launch DFS from node t
    DFS(t);

    // go through all nodes of the graph and deleted the ones with reachable value = 0
    // delete means both mark deleted[u] = 0 and add them to the stack of deleted nodes
    for(int u = 0; u < G.size(); u++){
        // here we need the differentiation between -1 and 0: otherwise we add to the stack nodes already removed
        if(reachable[u] == 0){
            // deleted[u] = true;
            // del_stack.push(u);
            remove_node(u);
        }
    }

    return;
}



// global variable used to count the number of paths
// also count the total length of the paths up to now 
// (can be substituted with full enumeration)
long long count_paths;
long long total_length;
int curr_path_len;
long good_diff_len;

// global variable used to find how many dead ends there are
long long dead_ends;
long long dead_total_len; // total length of dead ends
long dead_diff_len; // edges only belonging to dead ends; increase by 1 every time we backtrack

// constant which limits the number of function calls
// plus global variable that takes into account the number of calls performed
const int MAX_CALLS = 1000000; 
int calls_performed;
bool lampadina;

// paths must return the status, either success or fail
// we do so by returning true/false: true = success
bool paths_05(int u, int t){
    // base cases
    if(u== t){
        return true;
    }


    // we have non-deleted neighbors to explore
    if(degree(u) > 0){
        remove_node(u); // also adds to stack

        bool success = true; 
        for(auto v: G[u]){ 
            if(!deleted[v]){ // we take the next non-deleted element of G[u], noting that these deleted elements dynamically change during the for loop
                success = paths_05(v, t);
                if(!success && !reachable[u]){ // ALTERNATIVE: if(degree(u) == 0 && lampadina)
                    return false; // return at first failing neighbor
                }
                if(!success && reachable[u]) // if(degree(u)>0 && lampadina) HERE WE ARE AT THE ARTICULATION POINT
                    lampadina=false;
            }
            
        }

        // rimettere le cose a posto
        // pop stack until u (included) and mark as not deleted
        // WHAT ABOUT REACHABILITY?
        while(del_stack.top() != u){
            deleted[del_stack.top()] = 0;
            del_stack.pop();
        }
        deleted[u] = 0;
        del_stack.pop();

        return true;
    }



    // here we are in the case where degree(u) = 0. If lampadina, we just return; else we perform the visit
    // if(degree(u) == 0 && !lampadina){ 
    if(lampadina)
        return false;
    
    
    // VISITA: marca cancellati i nodi non raggiungibili da t + vettore reachable
    reachability_check(t);
    // accendo la lampadina
    lampadina = true;
    
    return false;



    // ======================FROM VERSION ZERO ============================

    // calls_performed++;
    // curr_path_len++;

    // if(calls_performed >= MAX_CALLS)
    //     return true;
    
    // if(calls_performed % 10000 == 0)
    //     cout << "*" << flush;

    // if(s == t){
    //     count_paths++;
    //     good_diff_len++;

    //     // every time we arrive at t, we sum to the total length the current path length
    //     // we also need to decrease the current path length
    //     total_length += curr_path_len;
    //     curr_path_len--;
    //     return true;
    // }
    
    // vector<int> curr_neigh = neighbors(s);
    

    // if(curr_neigh.size() == 0){
    //     // dead ends is increased: we failed on a node
    //     dead_ends++;

    //     // increase total dead ends' length
    //     dead_total_len = dead_total_len + curr_path_len;
    //     dead_diff_len++;

    //     // decrease current path length as we are backtracking
    //     curr_path_len--;

        
    //     return false;
    // }
        


    // deleted[s] = 1;

    // // printGraph();

    // // the return value is the OR of the values for the neighbors
    // // bool ret_value = false;
    // bool neigh_value = true;
    // bool ret_value = false;
    // bool first_good = false;
    // // int num_good_neigh = 0; // counter needed for good_diff_len: the latter is increased only if exactly one good neighbor
    // int i = 0;
    // while(neigh_value && i < curr_neigh.size()){
    //     neigh_value = paths(curr_neigh[i], t);
    //     ret_value = ret_value || neigh_value;


    //     if(calls_performed >= MAX_CALLS){
    //         deleted[s] = 0;
    //         return true;
    //     }

    //     i++;
    // }

    // // if we found a failing neighbor, perform visit from t
    // if(!neigh_value){
    //     vector<bool> good_neigh = check_neighbors(s, t);

    //     // at this point, resume where we left off to recurse in good neighbors
    //     for (; i < curr_neigh.size(); i++)
    //     {
    //         if(good_neigh[i]){
    //             bool test = paths(curr_neigh[i], t);
    //             ret_value = ret_value || test;


    //             if(calls_performed >= MAX_CALLS){
    //                 deleted[s] = 0;
    //                 return true;
    //             }
    //         }
                
    //     }
    // }


    // deleted[s] = 0;


    // if(!ret_value){
    //     dead_diff_len++;
    // }

    // if(ret_value)
    //     good_diff_len++;


    // // we need to decrease current path length IN ANY CASE when returning
    // curr_path_len--;

    // return ret_value;
}

void enumerate_paths(int s, int t){
    count_paths = 0;
    total_length = 0;
    dead_ends = 0;
    calls_performed = 0;
    curr_path_len = -1;
    good_diff_len = 0;
    dead_diff_len = 0;
    dead_total_len = 0;
    paths_05(s,t);
    good_diff_len--; // source returned true and thus added one 

    return;
}

int main(){ 
    char* input_filename = "tvshows.txt";
    create_graph(input_filename);

    // printGraph();


    // find max degree of graph
    int maxdeg = 0;
    for(int u=0; u < G.size(); u++){
        if(maxdeg< degree(u)){
            maxdeg = degree(u);
        }
    }

    // here we also find the number of edges
    int numedges = 0;
    int numnodes = G.size();

    for(int node = 0; node < G.size(); node++){
        numedges += G[node].size();
    }
    numedges = numedges/2;

    cout << "Graph has maximum degree " << maxdeg << endl; 

    clock_t start;
    double duration;

    start = clock();

    // standard: s = 0, t=last node
    enumerate_paths(0, G.size()-1);
    // enumerate_paths(0,6); // for small example
    duration = (clock() - start) / (double) CLOCKS_PER_SEC;

    cout <<  "Elapsed time: " << duration << " sec; calls performed are " << calls_performed << endl;

    cout << "Paths found are " <<count_paths << "; their total length is "<< total_length << " and their partial length is " << good_diff_len << endl;
    cout << "Dead ends are " << dead_ends << "; their total length is " << dead_total_len << " and their partial length is " << dead_diff_len <<endl;

    // reporting to file
    // ofstream output_file; 
    // output_file.open("output-b0.txt", ios::app);
    // output_file << "-----------------------------------------------------"<< endl;
    // output_file << "Output for graph with " << numnodes << " nodes, " << numedges << " edges and max degree " << maxdeg << " (" << input_filename << ")"<< endl;
    // output_file << calls_performed << " calls performed in " << duration << " secs (MAX_CALLS = " << MAX_CALLS << ")" << endl;
    // output_file << "Paths found are " <<count_paths << " for a total length of " << total_length << " and a partial length of " << good_diff_len << endl;
    // output_file<< "Dead ends are " << dead_ends << " for a total length of "<< dead_total_len << " and a partial length of " << dead_diff_len <<endl;
    // output_file << "-----------------------------------------------------"<< endl<<endl<<endl;
    // output_file.close();

    return 0;
}
