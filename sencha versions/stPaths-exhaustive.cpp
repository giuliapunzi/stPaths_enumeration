#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stack>
#include <stdexcept>
#include <chrono>
#include <cstdint>

using namespace std;
vector<int> deleted;
vector<int> current_degree; // keep global degree vector, updating it as deletions and insertions go

stack<int> del_stack; // stack of nodes that have been deleted

typedef vector<vector<int>> graph;
graph G;

vector<int> reachable; // marks nodes that have been visited by the DFS

long visits_performed;

// global variable used to count the number of paths
// also count the total length of the paths up to now 
// (can be substituted with full enumeration)
long long count_paths;
long long total_length;
int curr_path_len;
long long good_diff_len;

// constant which limits the number of function calls
// plus global variable that takes into account the number of calls performed
const int MAX_CALLS = 500000000000; 
const int MAX_TIME = 60000;
uint64_t start_time;
int calls_performed;

char* input_filename = "tvshows.txt";

bool is_edge(int u, int v);


uint64_t timeMs() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

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


// returns degree of node u; if node is deleted return zero 
// remember: need to take into account deleted edges
inline int olddegree(int u)
{
    // NOTE: IN OUR RECURSIVE CALLS WE ASK FOR THE DEGREE OF A NODE THAT IS CURRENTLY REMOVED
    int deg = 0;
    // if(deleted[u])
    //     return 0;
    // else{
        for(int i = 0; i< G[u].size(); i++){
            if(!deleted[G[u][i]])
                deg++;
        }
    // }

    return deg;
    // return current_degree[u]; 
}

inline int degree(int u){
    return current_degree[u];
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

    for (auto v : G[u])
        current_degree[v]--;
    
    return;
}

// reinserts top of the stack node in the graph
inline void reinsert_node()
{
    int u = del_stack.top();
    del_stack.pop();
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


// starts a visit from t, and marks as reachable the nodes that
// are reached through the visit.
void reachability_check(int t){
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

    // go through all nodes of the graph and deleted the ones with reachable value = 0
    // delete means both mark deleted[u] = 0 and add them to the stack of deleted nodes
    for(int u = 0; u < G.size(); u++){
        // here we need the differentiation between -1 and 0: otherwise we add to the stack nodes already removed
        // NO: CHECK IF DELETED BY USING REGULAR VECTOR!
        if(!reachable[u] && !deleted[u]){
            // deleted[u] = true;
            // del_stack.push(u);
            remove_node(u);
        }
    }

    return;
}


// paths must return the status, either success or fail
// we do so by returning true/false: true = success
void paths_ex(int u, int t){
    curr_path_len++;

    if(calls_performed >= MAX_CALLS)
        return;

    if(timeMs() - start_time >= MAX_TIME)
        return;

    calls_performed++;

    if(calls_performed % 1000000 == 0)
        cout << "*" << flush;

    // base case
    if(u== t){
        count_paths++;
        total_length+=curr_path_len;
        curr_path_len--;
        good_diff_len++;
        return;
    }

    // at every step, we perform the visit which deletes non-reachable nodes
    remove_node(u);
    reachability_check(t);

    if(degree(u) == 0){
        throw logic_error("Error: Found a dead end in correct algorithm");
    }

    // we have non-deleted neighbors; explore them
    // if(degree(u) > 0){ // don't need this anymore
    for(auto v: G[u]){ 
        if(!deleted[v]){ // we take the next non-deleted element of G[u], noting that these deleted elements dynamically change during the for loop
            paths_ex(v, t);
        }
        
    }

    // rimettere le cose a posto       
    // pop stack until u (included) and mark as not deleted
    while(del_stack.top() != u)
        reinsert_node();
    
    reinsert_node(); // here we are inserting u 

    curr_path_len--;
    good_diff_len++;
    return;
}

void enumerate_paths(int s, int t){
    count_paths = 0;
    total_length = 0;
    calls_performed = 0;
    curr_path_len = -1;
    good_diff_len = 0;
    visits_performed= 0;
    paths_ex(s,t);
    good_diff_len--; // source returned true and thus added one 

    return;
}

int main(){ 
    // char* input_filename = "artist.txt";
    create_graph(input_filename); // initialize 

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

    start_time = timeMs();

    // standard: s = 0, t=last node
    enumerate_paths(0, G.size()-1);
    uint64_t duration = (timeMs() - start_time);

    cout << endl<<   "Elapsed time: " << duration << " sec; calls performed are " << calls_performed << endl;
    cout << "Visits performed are  " << visits_performed << endl;

    cout << "Paths found are " <<count_paths << " ; their total length is "<< total_length << " and their partial length is " << good_diff_len << endl;
    // cout << "Dead ends are " << dead_ends << "; their total length is " << dead_total_len << " and their partial length is " << dead_diff_len <<endl;

    // reporting to file
    ofstream output_file; 
    // output_file.open("output-exhaust.txt", ios::app);
    // output_file << "-----------------------------------------------------"<< endl;
    // output_file << "Output for graph with " << numnodes << " nodes, " << numedges << " edges and max degree " << maxdeg << " (" << input_filename << ")"<< endl;
    // output_file << calls_performed << " calls performed in " << duration << " secs (MAX_CALLS = " << MAX_CALLS << ")" << endl;
    // output_file << "Visits of the graph performed are  " << visits_performed << endl;
    // output_file << "Paths found are " <<count_paths << " for a total length of " << total_length << " and a partial length of " << good_diff_len << endl;
    // // output_file<< "Dead ends are " << dead_ends << " for a total length of "<< dead_total_len << " and a partial length of " << dead_diff_len <<endl;
    // output_file << "-----------------------------------------------------"<< endl<<endl<<endl;
    // output_file.close();


    // reporting to file for exhaustive
    output_file.open("exhaustive-comparison.txt", ios::app);
    output_file << "------------------------VERSION EXH-----------------------------"<< endl;
    output_file << "Output for graph with " << numnodes << " nodes, " << numedges << " edges and max degree " << maxdeg << " (" << input_filename << ")"<< endl;
    output_file << calls_performed << " calls performed in " << duration << " secs (MAX_CALLS = " << MAX_CALLS << ")" << endl;
    output_file << "Visits of the graph performed are  " << visits_performed << endl;
    output_file << "Paths found are " <<count_paths << " for a total length of " << total_length << " and a partial length of " << good_diff_len << endl;
    output_file << "-----------------------------------------------------"<< endl<<endl<<endl;
    output_file.close();

    return 0;
}
