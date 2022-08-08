#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cstdint>

using namespace std;
vector<int> deleted;
vector<int> current_degree; // keep global degree vector, updating it as deletions and insertions go
vector<int> reachable; // marks nodes that have been visited by the DFS

vector<vector<int>> G;

bool is_edge(int u, int v);

// global variable used to count the number of paths
// also count the total length of the paths up to now 
// (can be substituted with full enumeration)
unsigned long count_paths;
unsigned long long total_length;
int curr_path_len;
unsigned long good_diff_len;

// global variable used to find how many dead ends there are
unsigned long dead_ends;
unsigned long long dead_total_len; // total length of dead ends
unsigned long dead_diff_len; // edges only belonging to dead ends; increase by 1 every time we backtrack

long MAX_TIME; 
int calls_performed;
uint64_t start_time;


uint64_t timeMs() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}


// create graph from file filename (WE REMOVE MULTI-EDGES AND SELF LOOPS)
void create_graph(char* filename)
{
    FILE* input_graph = fopen(filename, "r");
    int N;

    // file contains number of nodes, number of edges at fist line
    // and then one edge per line
    fscanf(input_graph, "%d", &N);

    int real_edges = 0;

    G.resize(N);
    deleted.resize(N);
    current_degree.resize(N, 0);
    reachable.resize(N, 1);

    // initialize reachable vector to be all 1 and degree to be zero
    for(int i = 0; i < G.size() ; i++){
        current_degree[i] = 0;
        reachable[i]=1;
    }
    
    int u, v;
    // we need to skip the first N rows
    for(int i = 0; i < N; i++)
        fscanf(input_graph, "%d %d", &u, &v);
    
    // for(int i=0; i<M; i++)
    while(fscanf(input_graph,"%d %d",&u, &v) != EOF)
    {
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
    return;
}
 

// create graph from file filename 
// WE REMOVE SELF LOOPS
void create_graph_old(char* filename)
{
    FILE* input_graph = fopen(filename, "r");
    // input_graph.open(filename);
    
    int N, M;
    cout << "Inside read graph" << endl << flush;
    // file contains number of nodes, number of edges at fist line
    // and then one edge per line
    // input_graph >> N >> M;
    fscanf(input_graph, "%d %d", &N, &M);

    int real_edges = 0;

    G.resize(N);
    deleted.resize(N);
    current_degree.resize(N, 0);
    reachable.resize(N, 1);

    // initialize reachable vector to be all 1 and degree to be zero
    for(int i = 0; i < G.size() ; i++){
        reachable[i] = 1;
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
    return;
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


// ++++++++++++++++++++++++++++++++++ GRAPH MODIFIERS ++++++++++++++++++++++++++++++++

// remove node u from the graph, by marking deleted vector to 0
inline void remove_node(int u)
{
    deleted[u] = 1;
    return;
}


// ++++++++++++++++++++++++++++++++ GRAPH VISITS +++++++++++++++++++++++++++++++

// recursive DFS procedure from node u
// void DFS(int u, vector<bool> &visited){
//     // cout << "Entering DFS for " << u << endl << flush;
//     visited[u] = true;

//     for(int i = 0; i < G[u].size(); i++)
//     {
//         if(!visited[G[u][i]] && !deleted[G[u][i]])
//             DFS(G[u][i], visited);
//     }

//     return;
// }

// recursive DFS procedure from node u
void DFS(int u){
    // cout << "Entering DFS for " << u << endl << flush;
    reachable[u] = 1;

    for(int i = 0; i < G[u].size(); i++)
    {
        // reachable being equal to zero means that the node has not been deleted nor already visited
        if(!reachable[G[u][i]])
            DFS(G[u][i]);
    }

    return;
}

int visits_performed;

// starts a visit from t, and marks as good neighbors the neighbors of s that
// are reached through the visit. Outputs the vector of these good neighbors.
vector<bool> check_neighbors(int s, int t){
    // vector<bool> visited(G.size());
    // visits_performed++;

    // // initialize all deleted nodes as visited
    // for(int i = 0; i< visited.size(); i++){
    //     if(deleted[i])
    //         visited[i] = true;
    //     else
    //         visited[i] = false;
    // }
    reachable.resize(G.size());
    visits_performed++;

    // initialize all deleted nodes as visited
    for(int i = 0; i< reachable.size(); i++){
        if(deleted[i])
            reachable[i] = 1;
        else
            reachable[i] = 0;
    }

    // launch DFS from node t
    DFS(t);

    vector<int> neigh = neighbors(s);
    // find out which neighbors of s have been visited, and output them
    vector<bool> good_neighbors(neigh.size());
    for(int i = 0; i < neigh.size(); i++){
        if(reachable[neigh[i]])
            good_neighbors[i] = true;
        else
            good_neighbors[i] = false;
            
    }
    return good_neighbors;
}


// paths must return the status, either success or fail
// we do so by returning true/false: true = success
bool paths_0(int s, int t){
    curr_path_len++;

    // if(calls_performed >= MAX_CALLS)
    //     return true;
    
    if(timeMs() - start_time >= MAX_TIME)
        return true;

    calls_performed++;
    
    // if(calls_performed % 1000000 == 0)
    //     cout << "*" << flush;

    if(s == t){
        count_paths++;
        good_diff_len++;

        // every time we arrive at t, we sum to the total length the current path length
        // we also need to decrease the current path length
        total_length += curr_path_len;
        curr_path_len--;
        return true;
    }
    
    vector<int> curr_neigh = neighbors(s);
    

    if(curr_neigh.size() == 0){
        // dead ends is increased: we failed on a node
        dead_ends++;

        // increase total dead ends' length
        dead_total_len = dead_total_len + curr_path_len;
        dead_diff_len++;

        // decrease current path length as we are backtracking
        curr_path_len--;
        return false;
    }
    deleted[s] = 1;

    bool neigh_value = true;
    bool ret_value = false;
    bool first_good = false;
    // int num_good_neigh = 0; // counter needed for good_diff_len: the latter is increased only if exactly one good neighbor
    int i = 0;
    while(neigh_value && i < curr_neigh.size()){
        neigh_value = paths_0(curr_neigh[i], t);
        ret_value = ret_value || neigh_value;
        i++;
    }

    // if we found a failing neighbor, perform visit from t
    if(!neigh_value){
        vector<bool> good_neigh = check_neighbors(s, t);

        // at this point, resume where we left off to recurse in good neighbors
        for (; i < curr_neigh.size(); i++)
        {
            if(good_neigh[i]){
                bool test = paths_0(curr_neigh[i], t);
                ret_value = ret_value || test;
            }
                
        }
    }
    deleted[s] = 0;

    if(!ret_value)
        dead_diff_len++;

    if(ret_value)
        good_diff_len++;

    // we need to decrease current path length IN ANY CASE when returning
    curr_path_len--;
    return ret_value;
}

void enumerate_paths_0(int s, int t){
    count_paths = 0;
    total_length = 0;
    dead_ends = 0;
    calls_performed = 0;
    curr_path_len = -1;
    good_diff_len = 0;
    dead_diff_len = 0;
    dead_total_len = 0;
    visits_performed=0;
    paths_0(s,t);
    good_diff_len--; // source returned true and thus added one 
    return;
}

int main(int argc, char* argv[]){ 

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <FILENAME>" << std::endl;
        return 1;
    }

    char * input_filename = argv[1];
    create_graph(input_filename);

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

    int s , t;
    cout << "Insert value for s from 0 to " << numnodes-1 << ": ";
    cin >> s;
    cout << "Insert value for t from 0 to " << numnodes-1 << ": ";
    cin >> t;

    // initialize all nodes as non-reachable
    for(int i = 0; i< reachable.size(); i++)
        reachable[i] = 0;

    // chech reachability of s from t
    DFS(t);

    if(!reachable[s]){
        cout << "Node s is not reachable from t" << endl;
        return 0;
    }

    // mark as deleted the non-reachable nodes
    for(int i =0;i < G.size();i++){
        if(!reachable[i])
            deleted[i]= 1;
    }

    cout << "Insert max time (s): ";
    cin >> MAX_TIME;

    MAX_TIME = MAX_TIME*1000;

    char foutput;
    cout << "Want file output? (y/n) ";
    cin >> foutput;

    start_time = timeMs();
    // standard: s = 0, t=last node
    enumerate_paths_0(s, t);
    uint64_t duration = (timeMs() - start_time);

    cout << endl;
    cout << "File: "<< input_filename;
    cout << "\ts= " << s;
    cout << "\tt= " << t<< endl;
    cout << "Time (ms): " << duration<< endl;
    cout << "Rec calls: " << calls_performed;
    cout << "\tVisits: " << visits_performed << endl;
    cout << "Paths found: " <<count_paths;
    cout << "\tDead ends: " << dead_ends << endl;

    if(foutput == 'y' || foutput == 'Y'){
        // reporting to file
        ofstream output_file; 
        output_file.open("output-v0.txt", ios::app);
        output_file << "-----------------------------------------------------"<< endl;
        output_file << "Output for graph with " << numnodes << " nodes, " << numedges << " edges and max degree " << maxdeg << " (" << input_filename << ")"<< endl;
        output_file << calls_performed << " calls performed in " << duration << " ms" << endl;
        output_file << "Visits of the graph performed are  " << visits_performed << endl;
        output_file << "Paths from s="<< s <<" to t="<< t << " found are " <<count_paths << " for a total length of " << total_length << " and a partial length of " << good_diff_len << endl;
        output_file<< "Dead ends are " << dead_ends << " for a total length of "<< dead_total_len << " and a partial length of " << dead_diff_len << endl;
        output_file << "-----------------------------------------------------"<< endl<<endl<<endl;
        output_file.close();
    }

    return 0;
}
