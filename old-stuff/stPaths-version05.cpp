#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stack>

using namespace std;
vector<int> deleted;
vector<int> current_degree; // keep global degree vector, updating it as deletions and insertions go

stack<int> del_stack; // stack of nodes that have been deleted
// vector<int> current_sol;

typedef vector<vector<int>> graph;
graph G;

vector<int> reachable; // marks nodes that have been visited by the DFS


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

int visits_performed;

// starts a visit from t, and marks as reachable the nodes that
// are reached through the visit.
void reachability_check(int t){
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
            // cout << "[reachability removed "<< u<< "]" << endl;
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
const int MAX_CALLS = 100000000000; 
int calls_performed;
bool lampadina;

// paths must return the status, either success or fail
// we do so by returning true/false: true = success
bool paths_05(int u, int t){
    // current_sol.push_back(u);
    // cout << u << " ";
    calls_performed++;
    curr_path_len++;

    if(calls_performed >= MAX_CALLS)
        return true;

    // base case
    if(u== t){
        count_paths++;
        total_length+=curr_path_len;
        curr_path_len--;
        good_diff_len++;
        // cout << "S" << endl;
        
        // cout << "Sol ";
        // for(auto x : current_sol)
        //     cout << x << " ";
        // cout << endl;

        // current_sol.pop_back();
        return true;
    }

    // printGraph();

    bool first_time = true;
    // we have non-deleted neighbors to explore
    if(degree(u) > 0){
        remove_node(u); // also adds to stack

        bool success = true; 
        for(auto v: G[u]){ 
            if(!deleted[v]){ // we take the next non-deleted element of G[u], noting that these deleted elements dynamically change during the for loop
                // if(!first_time)
                //     cout << u << " ";
                success = paths_05(v, t);
                first_time = false;
                // if(!success && !reachable[u])
                if(degree(u) == 0 && lampadina){ // ALTERNATIVE
                    curr_path_len--;
                    dead_diff_len++;
                    // current_sol.pop_back();
                    return false; // return at first failing neighbor
                }
                // if(!success && reachable[u])
                if(degree(u)>0 && lampadina){ //  HERE WE ARE AT THE ARTICULATION POINT
                    // cout << "Back to " << u << endl;
                    lampadina=false;
                }
            }
        }

        // rimettere le cose a posto       
        // pop stack until u (included) and mark as not deleted
        while(del_stack.top() != u){
            // deleted[del_stack.top()] = 0;
            // del_stack.pop();
            reinsert_node();
        }
        // deleted[u] = 0;
        // del_stack.pop();
        reinsert_node(); // here we are inserting u 

        curr_path_len--;
        good_diff_len++;
        // current_sol.pop_back();
        return true;
    }



    // here we are in the case where degree(u) = 0. If lampadina, we just return; else we perform the visit
    // if(degree(u) == 0 && !lampadina){ 
    if(lampadina){
        curr_path_len--;
        dead_diff_len++;
        // current_sol.pop_back();
        return false;
    }

    // cout << "X" << endl;
    // here we are just arriving in a node of degree zero
    // VISITA: marca cancellati i nodi non raggiungibili da t + vettore reachable
    reachability_check(t);
    // accendo la lampadina
    lampadina = true;
    
    dead_ends++;
    dead_total_len+=curr_path_len;
    dead_diff_len++;
    curr_path_len--;
    // current_sol.pop_back();
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
    visits_performed= 0;
    paths_05(s,t);
    good_diff_len--; // source returned true and thus added one 

    return;
}

int main(){ 
    char* input_filename = "visit-comma.txt";
    create_graph(input_filename); // initialize 

    printGraph();
    int s  = 0;
    int t = G.size() -1;

    // initialize all nodes as non-reachable
    for(int i = 0; i< reachable.size(); i++)
            reachable[i] = 0;

    // chech reachability of s from t
    DFS(t);
    if(!reachable[s]){
        cout << "Node s not reachable from t" << endl;
        return 0;
    }

    // mark as deleted the non-reachable nodes
    for(int i =0;i < G.size();i++){
        if(!reachable[i])
            deleted[i]= 1;
    }

    // find max degree of graph
    int maxdeg = 0;
    for(int u=0; u < G.size(); u++){
        if(maxdeg< degree(u)){
            maxdeg = degree(u);
        }
    }

    // here we also find the number of edges
    int numedges = 0;
    int numnodes = 0;

    for(int node = 0; node < G.size(); node++){
        if(!deleted[node]){
            numnodes++;
            numedges += G[node].size();
        }
    }
    numedges = numedges/2;

    cout << "Graph has maximum degree " << maxdeg << endl; 
    cout << "Nodes left are " << numnodes << ", and edges left are " << numedges << endl;

    clock_t start;
    double duration;

    start = clock();

    // standard: s = 0, t=last node
    enumerate_paths(0, G.size()-1);
    // enumerate_paths(0,6); // for small example
    duration = (clock() - start) / (double) CLOCKS_PER_SEC;

    cout <<  "Elapsed time: " << duration << " sec; calls performed are " << calls_performed << endl;
    cout << "Visits performed are  " << visits_performed << endl;

    cout << "Paths found are " <<count_paths << "; their total length is "<< total_length << " and their partial length is " << good_diff_len << endl;
    cout << "Dead ends are " << dead_ends << "; their total length is " << dead_total_len << " and their partial length is " << dead_diff_len <<endl;

    // reporting to file
    ofstream output_file; 
    // output_file.open("output-v05.txt", ios::app);
    // output_file << "-----------------------------------------------------"<< endl;
    // output_file << "Output for graph with " << numnodes << " nodes, " << numedges << " edges and max degree " << maxdeg << " (" << input_filename << ")"<< endl;
    // output_file << calls_performed << " calls performed in " << duration << " secs (MAX_CALLS = " << MAX_CALLS << ")" << endl;
    // output_file << "Visits of the graph performed are  " << visits_performed << endl;
    // output_file << "Paths found are " <<count_paths << " for a total length of " << total_length << " and a partial length of " << good_diff_len << endl;
    // output_file<< "Dead ends are " << dead_ends << " for a total length of "<< dead_total_len << " and a partial length of " << dead_diff_len <<endl;
    // output_file << "-----------------------------------------------------"<< endl<<endl<<endl;
    // output_file.close();

    return 0;
}
