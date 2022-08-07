#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <stack>
#include <chrono>
#include <cstdint>

using namespace std;
vector<int> deleted;
vector<int> current_degree; // keep global degree vector, updating it as deletions and insertions go

stack<int> del_stack; // stack of nodes that have been deleted
// vector<int> current_sol;

vector<vector<int>> G;

vector<int> reachable; // marks nodes that have been visited by the DFS

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
// const long MAX_CALLS = 5000000000; 
long MAX_TIME;
int calls_performed;
bool lampadina;
uint64_t start_time;

long long deleted_w_caterpillar;

bool is_edge(int u, int v);

int print_count;

long visits_performed_reach;
long visits_performed_cat;

uint64_t time_reachability;
uint64_t time_caterpillar;


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
    
    return;
}

inline void reinsert_simple(int u){
    deleted[u] = 0;

    for (auto v : G[u])
        current_degree[v]++;
    
    return;
}




// ++++++++++++++++++++++++++++++++ GRAPH VISITS +++++++++++++++++++++++++++++++
// A recursive function that find articulation
// points using DFS traversal
// adj --> Adjacency List representation of the graph
// u --> The vertex to be visited next
// visited --> keeps track of visited vertices
// disc --> Stores discovery times of visited vertices
// low -- >> earliest visited vertex (the vertex with minimum
// discovery time) that can be reached from subtree
// rooted with current vertex
// parent --> Stores the parent vertex in DFS tree
// is_art --> Stores articulation points
vector<bool> visited;
vector<int> disc;
vector<int> low;
vector<bool> is_art;
vector<bool> good_art;
vector<int> parent; 
vector<int> cat_stack;
vector<int> root_correct_neigh; // to delete neighbors of root
int visit_time;
bool found_s;
int current_s;

void find_artpts(int s, int u)
{
    cat_stack.push_back(u);
    // Count of children in DFS Tree
    int children = 0;
    if(u== s){
        found_s = true;
    }
 
    // Mark the current node as visited
    visited[u] = true;
 
    // Initialize discovery time and low value
    disc[u] = low[u] = ++visit_time;
    
    // int root_correct_neigh = -1;
    bool root_found = false; // becomes true when the root finds s
    bool good_for_current_BCC;    

    // Go through all non-deleted neighbors of u
    for (auto v : G[u]) {
        if(!deleted[v]){
            // If v is not visited yet, then make it a child of u
            // in DFS tree and recur for it
            if (!visited[v]) {
                good_for_current_BCC = false;
                parent[v] = u;
                children++;
                find_artpts(s, v);

                // if we are the root and we just found s, v is the only good neighbor
                // if(parent[u] == -1){
                //     if(!root_found){ 
                //         root_correct_neigh = v; 
                //         root_found = true;
                //     }
                // }
                // if we are the root and we just found s, v is the only good neighbor
                if(parent[u] == -1){
                    if(!root_found && found_s){ // the nodes in the stack right now are the 'correct BCC' for t
                        root_correct_neigh.push_back(v);

                        // it could  be the second BCC for t so we need to unstack until v
                        int ind = cat_stack.size() -1;
                        while(cat_stack[ind] != v)
                        {   
                            // if the element of the stack is a neighbor of t 
                            if (find(G[u].begin(), G[u].end(), cat_stack[ind])!= G[u].end()){
                                root_correct_neigh.push_back(cat_stack[ind]);
                            }  
                            ind--;
                        }
                        root_found = true;
                    }
                }
    
                // Check if the subtree rooted with v has
                // a connection to one of the ancestors of u
                low[u] = min(low[u], low[v]);
    
                // If u is not root and low value of one of
                // its child is more than discovery value of u.
                if (parent[u] != -1 && low[v] >= disc[u]){ // here is where I close my articulation point
                    is_art[u] = true;
                    
                    // I need to pop the stack until v (the last neighbor) If s in inside, I am good. also, delete nodes
                    int x = cat_stack.back();
                    while(x != v){
                        if(x == current_s){
                            good_for_current_BCC = true;
                        }
                        cat_stack.pop_back();
                        x = cat_stack.back();
                    }
                    if(x == current_s){
                        good_for_current_BCC = true;
                    }
                    cat_stack.pop_back(); // remove also v

                    // NEW: SET THE CURRENT ART POINT AS S!
                    if(good_for_current_BCC){
                        good_art[u] = true;
                        current_s = u;
                    }

                    if(!good_for_current_BCC){ // if I am not good, I need to delete my neighbors that have discovery time greater than v
                        for(auto neigh : G[u]){
                            if(visited[neigh] && !deleted[neigh] && disc[neigh] >= disc[v]){
                                remove_node(neigh);
                                deleted_w_caterpillar++;
                                // cout << "[caterpillar removed " << neigh << "]" << endl;
                            }
                        }
                    }
                }
            }
    
            // Update low value of u for parent function calls.
            else if (v != parent[u])
                low[u] = min(low[u], disc[v]);
        }
    }
 
    // If u is root of DFS tree and has two or more children.
    if (parent[u] == -1 && children > 1){
        is_art[u] = true;
        good_art[u] = true; // root is always good

        // remove all neighbors of the root that are not in the root_correct vector
        for (auto x : G[u]){   
        // if it is not deleted and it does not appear in the good neighbors, it is a bad neighbor
        if(!deleted[x] && find(root_correct_neigh.begin(),root_correct_neigh.end(),x) == root_correct_neigh.end()){
            remove_node(x);
            }
        }
    }

    
}

// start visit for finding articulation points from t
void find_caterpillar(int s, int t)
{
    disc.resize(G.size());
    low.resize(G.size());
    visited.resize(G.size());
    is_art.resize(G.size());
    good_art.resize(G.size());
    parent.resize(G.size());
    cat_stack.erase(cat_stack.begin(), cat_stack.end());
    visit_time = 0;
    // found_s = false;
    visits_performed_cat++;
    current_s = s;

    for(int i = 0; i < G.size(); i++){
        visited[i] = false;
        is_art[i] = false;
        good_art[i] = false;
        parent[i] = -2;
    }
    
    parent[t] = -1;

    uint64_t start = timeMs();
    // launch DFS from node t
     // only interested in the ones from s to t = caterpillar
    find_artpts(s,t);
    time_caterpillar += (timeMs() - start) ;

}


// recursive DFS procedure from node u
void DFS(int u){
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
    visits_performed_reach++;

    // initialize all deleted nodes as visited
    for(int i = 0; i< reachable.size(); i++){
        if(deleted[i])
            reachable[i] = 1;
        else
            reachable[i] = 0;
    }

    uint64_t start = timeMs();
    // launch DFS from node t
    DFS(t);
    time_reachability += (timeMs() - start);

    // go through all nodes of the graph and deleted the ones with reachable value = 0
    // delete means both mark deleted[u] = 0 and add them to the stack of deleted nodes
    for(int u = 0; u < G.size(); u++){
        // here we need the differentiation between -1 and 0: otherwise we add to the stack nodes already removed
        // NO: CHECK IF DELETED BY USING REGULAR VECTOR!
        if(!reachable[u] && !deleted[u]){
            remove_node(u);
            // cout << "[reachability removed "<< u<< "]" << endl;
        }
    }
    return;
}



// paths must return the status, either success or fail
// we do so by returning true/false: true = success
bool paths_075(int u, int t){
    // current_sol.push_back(u);
    curr_path_len++;

    // if(calls_performed >= MAX_CALLS)
    //     return true;

    if(timeMs() - start_time >= MAX_TIME)
        return true;

    calls_performed++;

    // if(calls_performed % 1000000 == 0)
    //     cout << "*" << flush;

    // base case
    if(u== t){
        count_paths++;
        total_length+=curr_path_len;
        curr_path_len--;
        good_diff_len++;

        // cout << "Sol ";
        // for(auto x : current_sol)
        //     cout << x << " ";
        // cout << endl;

        // current_sol.pop_back();
        return true;
    }

    // we have non-deleted neighbors to explore
    if(degree(u) > 0){
        remove_node(u); // also adds to stack

        bool success = true; 
        int num_good_children = 0;
        for(auto v: G[u]){ 
            if(!deleted[v]){ // we take the next non-deleted element of G[u], noting that these deleted elements dynamically change during the for loop
                success = paths_075(v, t);

                if(success)
                    num_good_children++;

                // I need to make sure that if we are in the second condition, we de-stack and reinsert stuff
                if(degree(u) == 0 && lampadina){ // ALTERNATIVE
                    curr_path_len--;
                    dead_diff_len++;
                    //current_sol.pop_back();
                    return false; // return at first failing neighbor
                }

                // here we can go back, but re-inserting deleted nodes
                if(degree(u) == num_good_children && lampadina){ // NOTE: Degree is now > 0
                    // rimettere le cose a posto       
                    // pop stack until u (included) and mark as not deleted
                    while(del_stack.top() != u){
                        reinsert_node();
                    }
                    reinsert_node(); // here we are inserting u 

                    curr_path_len--;
                    good_diff_len++;
                    //current_sol.pop_back();
                    return true; // at least one child was good at this point
                }

                // SAME AS degree(u)>num_good_children AND LAMPADINA
                if(degree(u)>0 && lampadina){ //  HERE WE ARE AT THE ARTICULATION POINT
                    lampadina=false;
                    reinsert_simple(u);
                    find_caterpillar(u, t); // compute caterpillar to delete useless neighbors
                    remove_simple(u);
                }
            }
        }

        // rimettere le cose a posto       
        // pop stack until u (included) and mark as not deleted
        while(del_stack.top() != u){
            reinsert_node();
        }
        reinsert_node(); // here we are inserting u 

        curr_path_len--;
        good_diff_len++;
        //current_sol.pop_back();
        return true;
    }

    // here we are in the case where degree(u) = 0. If lampadina, we just return; else we perform the visit
    // if(degree(u) == 0 && !lampadina){ 
    if(lampadina){
        curr_path_len--;
        dead_diff_len++;
        //current_sol.pop_back();
        return false;
    }

    // here we are just arriving in a node of degree zero
    reachability_check(t);
    lampadina = true;
    
    dead_ends++;
    dead_total_len+=curr_path_len;
    dead_diff_len++;
    curr_path_len--;
    //current_sol.pop_back();
    return false;
}

void enumerate_paths_075(int s, int t){
    count_paths = 0;
    total_length = 0;
    dead_ends = 0;
    calls_performed = 0;
    curr_path_len = -1;
    good_diff_len = 0;
    dead_diff_len = 0;
    dead_total_len = 0;
    visits_performed_reach= 0;
    visits_performed_cat = 0;
    paths_075(s,t);
    good_diff_len--; // source returned true and thus added one 

    return;
}

int main(int argc, char* argv[]){ 
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <FILENAME>" << std::endl;
        return 1;
    }

    char * input_filename = argv[1];
    create_graph(input_filename); // initialize 

    int s  = 0;
    int t = G.size() -1;

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

    // find max degree of graph
    int maxdeg = 0;
    for(int u=0; u < G.size(); u++){
        if(maxdeg< degree(u)){
            maxdeg = degree(u);
        }
    }

    print_count = 0;

    deleted_w_caterpillar = 0;
    find_caterpillar(s,t);

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

    cout << "First caterpillar removed " << deleted_w_caterpillar << " nodes. " << endl;
    cout << "Nodes left are " << numnodes << ", and edges left are " << numedges << endl;

    time_reachability = 0;
    time_caterpillar = 0;

    cout << "Insert max time (ms): ";
    cin >> MAX_TIME;

    char foutput;
    cout << "Want file output? (y/n) ";
    cin >> foutput;

    start_time = timeMs();

    // standard: s = 0, t=last node
    enumerate_paths_075(s,t);
    uint64_t duration = (timeMs() - start_time);

    cout << endl;
    cout << "File: "<< input_filename;
    cout << "\tTime (ms): " << duration<< endl;
    cout << "Rec calls: " << calls_performed;
    cout << "\tVisits: " << visits_performed_reach + visits_performed_cat << endl;
    cout << "Paths found: " <<count_paths;
    cout << "\tDead ends: " << dead_ends << endl;
    cout << "Reachability time (ms): "<< time_reachability;
    cout << "\tCaterpillar time (ms): " << time_caterpillar<< endl; 

    // cout << "Nodes removed with caterpillar are "<< deleted_w_caterpillar << endl;

    if(foutput == 'y' || foutput == 'Y'){
        // reporting to file
        ofstream output_file; 
        output_file.open("output-v075.txt", ios::app);
        output_file << "-----------------------------------------------------"<< endl;
        output_file << "Output for graph with " << numnodes << " nodes, " << numedges << " edges and max degree " << maxdeg << " (" << input_filename << ")"<< endl;
        output_file << calls_performed << " calls performed in " << duration << " ms" << endl;
        output_file << "Visits performed are  " << visits_performed_reach + visits_performed_cat <<" ; of which " << visits_performed_reach << " from reachability and " << visits_performed_cat << " from caterpillar."<< endl;
        output_file << "Paths found are " <<count_paths << " for a total length of " << total_length << " and a partial length of " << good_diff_len << endl;
        output_file<< "Dead ends are " << dead_ends << " for a total length of "<< dead_total_len << " and a partial length of " << dead_diff_len <<endl;
        output_file << "Nodes removed with caterpillar are "<< deleted_w_caterpillar << endl;
        output_file << "Time spent in reachability visits is "<< time_reachability << "; time spent in caterpillar visits is " << time_caterpillar<< endl;
        output_file << "-----------------------------------------------------"<< endl<<endl<<endl;
        output_file.close();
    }

    return 0;
}
