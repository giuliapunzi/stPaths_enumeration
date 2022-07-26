#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;
vector<bool> deleted;

typedef vector<vector<int>> graph;
graph G;

vector<int> current_degree; // keep global degree vector, updating it as deletions and insertions go
vector<bool> visited; // for the DFS
// vector<bool> good_neighbor; // support vector to identify good neighbors of a node during recursive call

char* input_filename = "tvshows.txt";

int visits_performed;

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
    current_degree.resize(N);
    visited.resize(G.size());

    // initialize degree to be zero
    for(int i = 0; i < G.size() ; i++){
        current_degree[i] = 0;
    }

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
            current_degree[u]++;
            current_degree[v]++;
        }
        
    }

    cout << "Input graph has " << N << " nodes and " << real_edges << " edges. "<< endl;

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
inline int degree(int u) {
    return current_degree[u];
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

// remove node u from the graph, by marking deleted vector to 0 and decreasing the degree of neighbors
inline void remove_node(int u)
{   
    // cout << "Before removing u=" << u << " its neighbors have degrees: ";
    // for (auto v : G[u])
    //     cout << current_degree[v] << " ";
    // cout << endl;

    deleted[u] = true;
    for (auto v : G[u])
        current_degree[v]--;

    // cout << "After removing u=" << u << " its neighbors have degrees: ";
    // for (auto v : G[u])
    //     cout << current_degree[v] << " ";
    // cout << endl<< endl;
    return;
}

inline void reinsert_node(int u){
    deleted[u] = false;
    for (auto v : G[u])
        current_degree[v]++;

    return;
}

// ++++++++++++++++++++++++++++++++ GRAPH VISITS +++++++++++++++++++++++++++++++

// recursive DFS procedure from node u
void DFS(int u){
    // cout << "Entering DFS for " << u << endl << flush;
    visited[u] = true;

    for(int i = 0; i < G[u].size(); i++)
    {
        if(!visited[G[u][i]] && !deleted[G[u][i]])
            DFS(G[u][i]);
    }

    return;
}


// starts a visit from t; we don't need to mark the neighbors
// as they will simply be the visited non deleted nodes.
void check_neighbors(int u, int t){
    visits_performed++;

    // initialize all deleted nodes as visited
    for(int i = 0; i< visited.size(); i++){
        if(deleted[i])
            visited[i] = true;
        else
            visited[i] = false;
    }

    // launch DFS from node t
    DFS(t);

    return;
}


// paths must return the status, either success or fail
// we do so by returning true/false: true = success
bool paths_0(int u, int t){
    // cout << "Inside call with u=" << u << " and t=" << t << endl; 
    curr_path_len++;

    if(calls_performed >= MAX_CALLS)
        return true;
    
    calls_performed++;

    if(calls_performed % 10000 == 0)
        cout << "*" << flush;

    //base case
    if(u == t){
        // cout << "We are at the base case, about to return true" << endl << endl;
        count_paths++;
        good_diff_len++;
        // every time we arrive at t, we sum to the total length the current path length
        // we also need to decrease the current path length
        total_length += curr_path_len;
        curr_path_len--;
        return true;
    }

    bool ret_value = false;
    bool lampadina = true; // this will signify whether we already performed a visit for a neighbor of u

    // cout << "Degree(u)=" << degree(u) << endl;

    if(degree(u) > 0){ // we need to visit its neighbors
        remove_node(u);
        // cout << "u="<< u << " has non-deleted neighbors" << endl;
        bool neigh_success = false; 

        // // mark the neighbors of u as all good
        // for (auto v : G[u])
        //     good_neighbor[v] = true; // NOTE: we could only mark non-deleted ones, but we still need to check deleted in the for loop, as they dynamically change

        // EVERY RECURSIVE CALL MUST NECESSARILY KEEP ITS OWN GOOD NEIGHBORS' VECTOR AS GLOBALLY THEY CHANGE IN SUCCESSIVE REC CALLS
        vector<bool> good_neighbor(G[u].size());
        for(int i =0 ; i < G[u].size(); i++){
            if(deleted[G[u][i]])
                good_neighbor[i] = false;
            else
                good_neighbor[i] = true;
        }

        for(int i =0 ; i < G[u].size(); i++){ 
            if(good_neighbor[i]){ // we take the next good (automatically non-deleted) element of G[u], noting that these deleted elements dynamically change during the for loop
                neigh_success = paths_0(G[u][i], t); // G[u][i] is the current neighbor, while i is its index in the neighbors'array
                ret_value = neigh_success || ret_value; // if neighbor returns true, we don't have to do anything else
                // if a neighbor returned false, we need to perform a visit (which will also delete other neighbors of u)
                if(!neigh_success && lampadina){ // if we found a failing neighbor and we have lampadina
                    // cout << "First failing neighbor for u=" <<u << " is v=" << G[u][i] << endl;
                    check_neighbors(u, t); // this DOES NOT mark neighbors as good/bad
                    // mark as good neighbors only the non-deleted visited ones
                    for (int j = 0; j<G[u].size(); j++){  // v :G[u])
                        if(!visited[G[u][j]]) 
                            good_neighbor[j] = false;
                    }
                    // cout << "Good neighbors after visit: ";
                    // for (int j = 0; j<G[u].size(); j++){
                    //     if(good_neighbor[j])
                    //         cout << G[u][j] << " ";
                    // }
                    // cout << endl;
                    lampadina = false; // visit will never be performed again for u
                }
            }
            
        }

        reinsert_node(u);

        // update counters
        if(ret_value)
            good_diff_len++;
        else
            dead_diff_len++;

        // we need to decrease current path length IN ANY CASE when returning
        curr_path_len--;
        // cout << "About to return for u=" << u << endl<<endl;
        return ret_value;
    }
        

    // if we are here, degree(u)= 0: we found a dead end
    dead_ends++;
    dead_total_len = dead_total_len + curr_path_len;
    dead_diff_len++;
    // decrease current path length before returning
    curr_path_len--;
    // cout << "About to return false for u=" << u << endl<< endl;
    return false;
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
    visits_performed = 0;
    paths_0(s,t);
    good_diff_len--; // source returned true and thus added one 

    return;
}

int main(){ 
    create_graph(input_filename);

    // printGraph();

    // find max degree of graph
    int maxdeg = 0;
    for(int u=0; u < G.size(); u++){
        if(maxdeg< degree(u)){
            maxdeg = degree(u);
        }
    }
    cout << "Graph has maximum degree " << maxdeg << endl; 

    // here we also find the number of edges
    int numedges = 0;
    int numnodes = G.size();

    for(int node = 0; node < G.size(); node++){
        numedges += G[node].size();
    }
    numedges = numedges/2;

    clock_t start;
    double duration;
    start = clock();

    // standard: s = 0, t=last node
    enumerate_paths(0, G.size()-1);
    // enumerate_paths(0,6); // for small example
    duration = (clock() - start) / (double) CLOCKS_PER_SEC;

    cout <<  endl << "Elapsed time: " << duration << " sec; calls performed are " << calls_performed << endl;
    cout << "Visits performed are  " << visits_performed << endl;
    cout << "Paths found are " <<count_paths << "; their total length is "<< total_length << " and their partial length is " << good_diff_len << endl;
    cout << "Dead ends are " << dead_ends << "; their total length is " << dead_total_len << " and their partial length is " << dead_diff_len <<endl;

    // reporting to file
    ofstream output_file; 
    output_file.open("output-v0-optimized.txt", ios::app);
    output_file << "-----------------------------------------------------"<< endl;
    output_file << "Output for graph with " << numnodes << " nodes, " << numedges << " edges and max degree " << maxdeg << " (" << input_filename << ")"<< endl;
    output_file << calls_performed << " calls performed in " << duration << " secs (MAX_CALLS = " << MAX_CALLS << ")" << endl;
    output_file  << "Visits performed are  " << visits_performed << endl;
    output_file << "Paths found are " <<count_paths << " for a total length of " << total_length << " and a partial length of " << good_diff_len << endl;
    output_file<< "Dead ends are " << dead_ends << " for a total length of "<< dead_total_len << " and a partial length of " << dead_diff_len <<endl;
    output_file << "-----------------------------------------------------"<< endl<<endl<<endl;
    output_file.close();

    return 0;
}
