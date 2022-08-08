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
vector<vector<int>> G;

vector<int> reachable; // marks nodes that have been visited by the DFS
long visits_performed;

// global variable used to count the number of paths
// also count the total length of the paths up to now 
// (can be substituted with full enumeration)
long long count_paths;
long long total_length;
int curr_path_len;
long long good_diff_len;
long deleted_w_caterpillar;
long time_reachability;
long time_caterpillar;

long MAX_TIME;

uint64_t start_time;
int calls_performed;


// vector<vector<int>> BCC_stack;
// vector<int> target_stack;

bool is_edge(int u, int v);


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
        reachable[i] = 1;
        current_degree[i] = 0;
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


inline bool is_neighbor(int u, int v){
    if(deleted[v])
        return false;
    
    return find(G[u].begin(), G[u].end(), v) != G[u].end();
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



inline void remove_simple(int u){
    deleted[u] = 1;

    for (auto v : G[u])
        current_degree[v]--;
    
    // cout << "Removing simple " << u << endl;
    return;
}

inline void reinsert_simple(int u){
    deleted[u] = 0;

    for (auto v : G[u])
        current_degree[v]++;
    
    // cout << "Reinserting simple " << u << endl;
    return;
}


// ++++++++++++++++++++++++++++++++ GRAPH VISITS +++++++++++++++++++++++++++++++


vector<bool> visited;
vector<int> disc;
vector<int> low;
vector<int> parent; 
vector<int> cat_stack;
int visit_time;
int current_s;
int last_art;
bool found_s;
bool root_art_pt;

void find_artpts(int s, int u)
{
    // cout << "Inserting "<< u << " in cat stack"<< endl<<flush;
    cat_stack.push_back(u);
    // Count of children in DFS Tree
    int children = 0;
 
    // Mark the current node as visited
    visited[u] = true;

    if(u== s)
        found_s = true;
 
    // Initialize discovery time and lowpoint value
    disc[u] = low[u] = ++visit_time;
    
    bool root_found = false; // becomes true when the root finds s
    bool good_for_current_BCC;    

    // Go through all non-deleted neighbors of u
    for (auto v : G[u]) {
        // cout <<v << " is a neighbor of " << u << endl;
        if(!deleted[v]){
            // If v is not visited yet, then make it a child of u
            // in DFS tree and recur for it
            if (!visited[v]) {
                good_for_current_BCC = false;
                parent[v] = u;
                children++;
                find_artpts(s, v);

                // if we are the root and we just found s, v is the only good neighbor
                if(parent[u] == -1 && found_s){
                    // if(!root_found && found_s){ // the nodes in the stack right now are the 'correct BCC' for t
                    // BCC_stack.push_back({v});

                    // it could be the second BCC for t so we need to unstack until v
                    int x = cat_stack.back();
                    while(x != v){ // If we are in the "right" BCC add it to the vector   
                        // BCC_stack.back().push_back(x);
                        cat_stack.pop_back();
                        x = cat_stack.back();
                    }
                    cat_stack.pop_back(); 
                    // int ind = cat_stack.size() -1;
                    // while(cat_stack[ind] != v)
                    // {   
                    //     // if the element of the stack is a neighbor of t 
                    //     // if (find(G[u].begin(), G[u].end(), cat_stack[ind])!= G[u].end())
                    //     if (is_neighbor(u, cat_stack[ind])){
                    //         root_correct_neigh.push_back(cat_stack[ind]);
                    //     }  
                    //     ind--;
                    // }
                    // root_found = true;
                    
                    // WE SHOULD BE ABLE TO RETURN HERE!
                    return;
                
                }
    
                // Check if the subtree rooted with v has
                // a connection to one of the ancestors of u
                low[u] = min(low[u], low[v]);
    
                // If u is not root and low value of one of
                // its child is more than discovery value of u.
                if (parent[u] != -1 && low[v] >= disc[u]){ // here is where I close my articulation point
                    // cout << "Closed an articulation point " << endl << flush;
                    int check = cat_stack.size()-1;
                    // first, find out if good for current BCC by scanning only the current BCC
                    while(cat_stack[check] != v)
                    {
                        if (cat_stack[check]== current_s){
                            good_for_current_BCC = true;
                        }  
                        check--;
                    }
                    if(v == current_s) 
                        good_for_current_BCC = true;
                    
                    // NEW: SET THE CURRENT ART POINT AS S!
                    if(good_for_current_BCC){
                        // if at this point the current source was s, this is the last art point
                        // NEED TO CHECK THAT IT IS DIFFERENT FROM S
                        // if(current_s == s && u != s)
                        //     last_art = u;
                        
                        // BCC_stack.push_back({v}); // initialize the BCC as v

                        // remove stuff from stack
                        int x = cat_stack.back();
                        while(x != v){ // If we are in the "right" BCC add it to the vector                            
                            // BCC_stack.back().push_back(x);
                            cat_stack.pop_back();
                            x = cat_stack.back();
                        }
                        cat_stack.pop_back(); 

                        // if(!is_neighbor(u, v))
                        //     throw logic_error("v should be a neighbor of u!!!");

                        current_s = u; 

                        // push the articulation point to the stack, and its vector of good neighbors in the corresponding
                        // position of the target_neighbors vector. 
                        // temp_target_stack.push_back(u);
                        // temp_target_neigh.push_back(good_neigh_current_target);
                        // target_stack.push_back(u);
                    }
                        
                    if(!good_for_current_BCC){ // if I am not good, I need to delete my neighbors that have discovery time greater than v
                        // remove stuff and delete
                        int x = cat_stack.back();
                        while(x != v){ // NEW: ONLY DELETE NEIGHBORS, SO MORE CHECKS BUT LESS WORK OVERALL
                            cat_stack.pop_back();
                            if(is_neighbor(u,x)){
                                remove_node(x);
                                // deleted_w_caterpillar++;
                                // cout << "[caterpillar removed " << x << "]" << endl;
                            }
                            
                            x = cat_stack.back();
                        }
                        cat_stack.pop_back();

                        if(!is_neighbor(u, v))
                            throw logic_error("v should be a neighbor of u!!!");

                        remove_node(x); // v is always neighbor
                        // deleted_w_caterpillar++;
                        // cout << "[caterpillar removed " << x << "]" << endl;
                    }
                }
            }
    
            // Update low value of u for parent function calls.
            else if (v != parent[u])
                low[u] = min(low[u], disc[v]);
        }
    }
    
    return;
    // If u is root of DFS tree and has two or more children.
    // In this case, we don't actually care about removing other BCCs, as they will never be explored
    // if (parent[u] == -1 && children > 1){
    //     root_art_pt = true;

    //     // remove neighbors of t that are bad, i.e. the neighbors not in root_correct_neigh
    //     // these can be removed, as they do not change as we go deeper in the recursion, only when we return normally. 
    //     // for(auto neigh : neighbors(u)){
    //     //     // cout << "Considering " << neigh << endl;
    //     //     if(find(root_correct_neigh.begin(), root_correct_neigh.end(), neigh) == root_correct_neigh.end()){ // need to check if neighbor's parent is t
    //     //         // cout << "Removing " << neigh << endl;
    //     //         remove_node(neigh);
    //     //         deleted_w_caterpillar++;
    //     //         // cout << "[caterpillar removed " << neigh << "]" << endl;
    //     //     }
    //     // }
    // }
}

// start visit for finding articulation points from t
void find_caterpillar(int s, int t)
{   
    cat_stack.erase(cat_stack.begin(), cat_stack.end());
    visit_time = 0;
    visits_performed++;
    found_s = false;
    current_s = s;
    last_art = -1;

    for(int i = 0; i < G.size(); i++){
        visited[i] = false;
        parent[i] = -2;
    }
    
    parent[t] = -1;

    // cout << "About to find art points from "<< s << " to "<< t;
    // printGraph();
    uint64_t start = timeMs();

    // sizes of the stacks
    // int size_t_stack = target_stack.size();
    // int size_C_stack = BCC_stack.size();


     // only interested in the ones from s to t = caterpillar
    find_artpts(s, t);

    // after the artpoints call, we added stuff IN REVERSE ORDER from size_C_stack+1 and size_t_stack+1 onwards.    
    // reverse(target_stack.begin() + size_t_stack, target_stack.end());
    // reverse(BCC_stack.begin() + size_C_stack, BCC_stack.end());

    // if(target_stack.size() != BCC_stack.size()-1)
    //     throw logic_error("Wrong number of targets wrt BCCs when exiting caterpillar");


    // time_caterpillar += (timeMs() - start);
    return;
}



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
void paths_classic_BCC(int u, int t){
    curr_path_len++;

    // if(calls_performed >= MAX_CALLS)
    //     return;

    if(timeMs() - start_time >= MAX_TIME)
        return;

    calls_performed++;

    // if(calls_performed % 1000000 == 0)
    //     cout << "*" << flush;

    // base case
    if(u== t){
        count_paths++;
        total_length+=curr_path_len;
        curr_path_len--;
        good_diff_len++;
        return;
    }

    // at every step, we perform the visit which deletes non-reachable nodes
    remove_node(u); // I need to remove the node to ensure correct placement on deleted stack

    reinsert_simple(u);
    find_caterpillar(u, t);
    remove_simple(u);

    if(degree(u) == 0){
        throw logic_error("Error: Found a dead end in correct algorithm");
    }

    // we have non-deleted neighbors; explore them
    // if(degree(u) > 0){ // don't need this anymore
    for(auto v: G[u]){ 
        if(!deleted[v]){ // we take the next non-deleted element of G[u], noting that these deleted elements dynamically change during the for loop
            paths_classic_BCC(v, t);
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

void enumerate_paths_classical(int s, int t){
    count_paths = 0;
    total_length = 0;
    calls_performed = 0;
    curr_path_len = -1;
    good_diff_len = 0;
    visits_performed= 0;
    paths_classic_BCC(s,t);
    good_diff_len--; // source returned true and thus added one 

    return;
}


int main(int argc, char* argv[]){ 

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <FILENAME>" << std::endl;
        return 1;
    }

    char * input_filename = argv[1];
    create_graph_old(input_filename); // initialize 

    disc.resize(G.size());
    low.resize(G.size());
    visited.resize(G.size());
    parent.resize(G.size());

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

    // target_stack.erase(target_stack.begin(), target_stack.end());
    // BCC_stack.erase(BCC_stack.begin(), BCC_stack.end());
    // target_stack.push_back(t);

    deleted_w_caterpillar = 0;
    find_caterpillar(s,t); // last art is now set up
    // cout << "Last art point is " << last_art << endl; 

    cout << "First caterpillar removed " << deleted_w_caterpillar << " nodes. " << endl;
    cout << "Nodes left are " << numnodes << ", and edges left are " << numedges << endl;
    

    time_reachability = 0;
    time_caterpillar = 0;

    // long MAX_CALLS; 

    // cout << "Insert max number of recursive calls: ";
    // cin >> MAX_CALLS;

    cout << "Insert max time (s): ";
    cin >> MAX_TIME;

    MAX_TIME = MAX_TIME*1000;

    char foutput;
    cout << "Want file output? (y/n) ";
    cin >> foutput;

    start_time = timeMs();

    // standard: s = 0, t=last node
    enumerate_paths_classical(s, t);
    uint64_t duration = (timeMs() - start_time);

    cout << endl;
    cout << "File: "<< input_filename;
    cout << "\ts= " << s;
    cout << "\tt= " << t<< endl;
    cout << "Time (ms): " << duration<< endl;
    cout << "Rec calls: " << calls_performed;
    cout << "\tVisits: " << visits_performed << endl;
    cout << "Paths found: " <<count_paths << endl;
    // " ; their total length is "<< total_length << " and their partial length is " << good_diff_len << endl;
    // cout << "Dead ends are " << dead_ends << "; their total length is " << dead_total_len << " and their partial length is " << dead_diff_len <<endl;


    if(foutput == 'y' || foutput == 'Y'){
        // reporting to file
        ofstream output_file; 
        output_file.open("output-classical.txt", ios::app);
        output_file << "-----------------------------------------------------"<< endl;
        output_file << "Output for graph with " << numnodes << " nodes, " << numedges << " edges and max degree " << maxdeg << " (" << input_filename << ")"<< endl;
        output_file << calls_performed << " calls performed in " << duration << " ms" << endl;
        output_file << "Visits of the graph performed are  " << visits_performed << endl;
        output_file << "Paths from s="<< s <<" to t="<< t << " found are " <<count_paths << " for a total length of " << total_length << " and a partial length of " << good_diff_len << endl;
        // output_file<< "Dead ends are " << dead_ends << " for a total length of "<< dead_total_len << " and a partial length of " << dead_diff_len <<endl;
        output_file << "-----------------------------------------------------"<< endl<<endl<<endl;
        output_file.close();
    }
    

    return 0;
}
