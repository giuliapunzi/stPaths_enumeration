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
vector<int> current_sol; // stack of current solution
vector<int> node_barriers;

long barrier_updates_num;
long barrier_update_time;

vector<vector<int>> G;

bool is_edge(int u, int v);

// global variable used to count the number of paths
// also count the total length of the paths up to now 
// (can be substituted with full enumeration)
unsigned long count_paths;
unsigned long long total_length;
long curr_path_len;
unsigned long good_diff_len;

// global variable used to find how many dead ends there are
unsigned long dead_ends;
unsigned long long dead_total_len; // total length of dead ends
unsigned long dead_diff_len; // edges only belonging to dead ends; increase by 1 every time we backtrack

long MAX_DIST;
long MAX_TIME; 
unsigned long calls_performed;
unsigned long visits_performed;
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
// inline int degree(int u)
// {
//     int deg = 0;
//     if(deleted[u])
//         return -1;
//     else{
//         for(int i = 0; i< G[u].size(); i++){
//             if(!deleted[G[u][i]])
//                 deg++;
//         }
//     }

//     return deg;
// }

inline int degree(int u){
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


// ++++++++++++++++++++++++++++++++++ GRAPH MODIFIERS ++++++++++++++++++++++++++++++++

// remove node u from the graph, by marking deleted vector to 0
inline void remove_node(int u)
{
    deleted[u] = 1;

    for (auto v : G[u])
        current_degree[v]--;

    return;
}


inline void reinsert_node(int u){
    deleted[u] = 0;

    for (auto v : G[u])
        current_degree[v]++;
    
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
        if(!reachable[G[u][i]])
            DFS(G[u][i]);
    }

    return;
}

// recursive DFS procedure from node u
void update_barrier(int u, int l){
    cout << "Entering barrier update for " << u << " with l="<< l << endl << flush;
    if(node_barriers[u]>l){
        node_barriers[u]= l;
        cout << "Updated barrier for " << u << endl;
        for (auto v : G[u])
        {
            // if neighbor is not in current stack, recurse
            if(find(current_sol.begin(), current_sol.end(), v) == current_sol.end())
                update_barrier(v, l+1);
        }
        
    }

    return;
}


// paths must return the status, either success or fail
// we do so by returning true/false: true = success
int BC_DFS(int u, int t){
    curr_path_len++;
    // cout << "Call for " << u << endl;
    // if(calls_performed >= MAX_CALLS)
    //     return true;
    cout << "At the start of call for "<<u<<"; barrier vals: ";
    for (auto x:node_barriers)
        cout << x << " ";
    cout << endl;
    
    if(timeMs() - start_time >= MAX_TIME)
        return MAX_DIST;

    calls_performed++;
    
    // if(calls_performed % 1000000 == 0)
    //     cout << "*" << flush;

    if(u == t){
        cout << "Arrived at t " << endl;
        count_paths++;
        good_diff_len++;

        // every time we arrive at t, we sum to the total length the current path length
        // we also need to decrease the current path length
        total_length += curr_path_len;
        curr_path_len--;
        return 0;
    }
    
    
    int currF = MAX_DIST;
    current_sol.push_back(u);
    int neighf;

    if(current_sol.size() != curr_path_len)
        throw logic_error("Partial solution and its length do not coincide!");

    int num_visited_neigh = 0;
    for(auto v : G[u]){
        // if v is not in the current stack, recurse
        if(find(current_sol.begin(), current_sol.end(), v) == current_sol.end()){
            num_visited_neigh++;
            neighf = BC_DFS(v, t);
            if(neighf != MAX_DIST){
                currF = std::min(currF, neighf + 1);
                cout << "Updating currF for " << u << " to " << currF  << endl;
            }
        }
    }

    // we have a dead end if no neighbor was explored
    if(num_visited_neigh == 0){
        cout << "Dead end in " << u << endl;
        if(currF != MAX_DIST)
            throw logic_error("No neighbors visited but updated F value!");
        dead_ends++;
    }
        
    if (currF == MAX_DIST){
        cout << "currF was not updated for " << u << endl;
        node_barriers[u] = MAX_DIST;
        dead_diff_len++;
    }
    else{
        cout << "currF was updated for " << u << "; performing update" << endl;
        cout << "Current sol stack: ";
        for (auto x:current_sol)
            cout << x << " ";
        cout << endl;
        uint64_t start_bar = timeMs();
        update_barrier(u, currF);
        barrier_update_time+= (timeMs() - start_bar);
        barrier_updates_num++;
        good_diff_len++;
    }

    current_sol.pop_back();

    // we need to decrease current path length IN ANY CASE when returning
    curr_path_len--;

    cout << "About to return from "<<u<<"; barrier vals: ";
    for (auto x:node_barriers)
        cout << x << " ";
    cout << endl;

    cout << "Current sol stack: ";
    for (auto x:current_sol)
        cout << x << " ";
    cout << endl;
    
    return currF;
}


int main(int argc, char* argv[]){ 

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <FILENAME>" << std::endl;
        return 1;
    }

    char * input_filename = argv[1];
    create_graph_old(input_filename);

    reachable.resize(G.size());
    node_barriers.resize(G.size());

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

    MAX_DIST = numnodes + 1;

    for (int i = 0; i < G.size(); i++)
        node_barriers[i] = 0;
    

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
    BC_DFS(s, t);
    uint64_t duration = (timeMs() - start_time);

    cout << endl;
    cout << "File: "<< input_filename;
    cout << "\ts= " << s;
    cout << "\tt= " << t<< endl;
    cout << "Time (ms): " << duration<< endl;
    cout << "Rec calls: " << calls_performed;
    cout << "\tVisits: " << barrier_updates_num << endl;
    cout << "Paths found: " <<count_paths;
    cout << "\tDead ends: " << dead_ends << endl;
    cout << "Time in barrier updates : " <<barrier_update_time << endl;

    if(foutput == 'y' || foutput == 'Y'){
        // reporting to file
        ofstream output_file; 
        output_file.open("output-BC-DFS.txt", ios::app);
        output_file << "-----------------------------------------------------"<< endl;
        output_file << "Output for graph with " << numnodes << " nodes, " << numedges << " edges and max degree " << maxdeg << " (" << input_filename << ")"<< endl;
        output_file << calls_performed << " calls performed in " << duration << " ms" << endl;
        output_file << "Visits of the graph performed are  " << barrier_updates_num << " for a total time of " << barrier_update_time << endl;
        output_file << "Paths from s="<< s <<" to t="<< t << " found are " <<count_paths << " for a total length of " << total_length << " and a partial length of " << good_diff_len << endl;
        output_file<< "Dead ends are " << dead_ends << " for a total length of "<< dead_total_len << " and a partial length of " << dead_diff_len << endl;
        output_file << "-----------------------------------------------------"<< endl<<endl<<endl;
        output_file.close();
    }

    return 0;
}
