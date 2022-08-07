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

typedef vector<vector<int>> graph;
graph G;

vector<int> reachable; // marks nodes that have been visited by the DFS
vector<int> good_neigh_first_t;
vector<int> bad_neigh_first_t;

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
const long MAX_CALLS = 50000000000;
// const long MAX_TIME = 60000;
long MAX_TIME; 
int calls_performed;
bool lampadina;
uint64_t  start_time;

long long deleted_w_caterpillar;

char* input_filename = "graph-40-80.txt";

bool is_edge(int u, int v);

int print_count;
long visits_performed_reach;
long visits_performed_cat_intermediate;
long visits_performed_cat_og;

uint64_t time_reachability;
uint64_t time_caterpillar;


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



vector<bool> visited;
vector<int> disc;
vector<int> low;
vector<bool> is_art;
vector<bool> good_art;
vector<int> parent; 
vector<int> cat_stack;
vector<int> root_correct_neigh;
int visit_time;
int current_s;
int last_art;
bool found_s;

void find_artpts(int s, int u)
{
    // cout << "Inserting "<< u << " in cat stack"<< endl;
    cat_stack.push_back(u);
    // Count of children in DFS Tree
    int children = 0;
 
    // Mark the current node as visited
    visited[u] = true;

    if(u== s)
        found_s = true;
 
    // Initialize discovery time and low value
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
                // cout <<v << " is not deleted and not visited! About to recur "<<endl;
                good_for_current_BCC = false;
                parent[v] = u;
                children++;
                find_artpts(s, v);

                // if we are the root and we just found s, v is the only good neighbor
                if(parent[u] == -1){
                    if(!root_found && found_s){ // the nodes in the stack right now are the 'correct BCC' for t
                        root_correct_neigh.push_back(v);
                        // cout <<"Just found correct bcc for target: with neighbor "<< v << endl;
                        
                        // cout<< "Current BCC stack: ";
                        // for(int i= 0; i< cat_stack.size(); i++)
                        //     cout << cat_stack[i] << " " ;
                        // cout << endl;

                        // cout << "neighbors of t : ";
                        // for (auto nn : neighbors(u))
                        // {
                        //     cout << nn << " ";
                        // }
                        // cout << endl;
                        

                        // it could  be the second BCC for t so we need to unstack until v
                        int ind = cat_stack.size() -1;
                        while(cat_stack[ind] != v)
                        {   
                            // cout << "considering element of stack " << cat_stack[ind] << endl;
                            // if the element of the stack is a neighbor of t 
                            if (find(G[u].begin(), G[u].end(), cat_stack[ind])!= G[u].end()){
                                root_correct_neigh.push_back(cat_stack[ind]);
                                // cout <<"Found "<< cat_stack[ind] << " to be a neighbor of target" << endl;
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
                    // cout <<u << " is an art point because of child " << v << endl;
                    
                    int check = cat_stack.size()-1;
                    // first, find out if good for current BCC by scanning only the current BCC
                    while(cat_stack[check] != v)
                    {
                        if (cat_stack[check]== current_s)
                        {
                            good_for_current_BCC = true;
                        }  
                        check--;
                    }
                    if(v == current_s) 
                        good_for_current_BCC = true;
                    

                    // NEW: SET THE CURRENT ART POINT AS S!
                    if(good_for_current_BCC){
                        good_art[u] = true;
                        // cout <<u << " is a good art point with child " << v << endl;
                        // if at this point the current source was s, this is the last art point
                        // NEED TO CHECK THAT IT IS DIFFERENT FROM S
                        if(current_s == s && u != s){
                            last_art = u;
                            good_neigh_first_t.push_back(v);
                        }

                        // remove stuff from stack
                        int x = cat_stack.back();
                        while(x != v){ // If we are in the "right" BCC and the node is a neighbor of u, add it to good neighbors
                            // cout <<"Removing " << x << " from cat stack "<< endl;
                            
                            if(current_s == s && u != s && find(G[u].begin(), G[u].end(), x) != G[u].end()){
                                good_neigh_first_t.push_back(x);
                                // cout <<" And adding it to good neighbors" << endl;
                            }

                            cat_stack.pop_back();
                            x = cat_stack.back();
                        }
                        cat_stack.pop_back(); 
                        // cout <<"Removing " << x << " from cat stack "<< endl;


                        // cout<< "Current BCC stack: ";
                        // for(int i= 0; i< cat_stack.size(); i++)
                        //     cout << cat_stack[i] << " " ;
                        // cout << endl;

                        if(find(G[u].begin(), G[u].end(), v) == G[u].end())
                            throw logic_error("v should be a neighbor of u!!!");

                        current_s = u;
                    }
                        

                    if(!good_for_current_BCC){ // if I am not good, I need to delete my neighbors that have discovery time greater than v
                        // remove stuff and delete
                        // cout <<"Found a bad art point; destack and delete "<< endl;
                        int x = cat_stack.back();
                        while(x != v){
                            cat_stack.pop_back();
                            remove_node(x);
                            deleted_w_caterpillar++;
                            // cout << "[caterpillar removed " << x << "]" << endl;
                            x = cat_stack.back();
                        }
                        cat_stack.pop_back();
                        remove_node(x);
                        deleted_w_caterpillar++;
                        // cout << "[caterpillar removed " << x << "]" << endl;
                        // cout<< "Current BCC stack: ";
                        // for(int i= 0; i< cat_stack.size(); i++)
                        //     cout << cat_stack[i] << " " ;
                        // cout << endl;
                    }
                }
            }
    
            // Update low value of u for parent function calls.
            else if (v != parent[u])
                low[u] = min(low[u], disc[v]);
        }
    }
 
    // If u is root of DFS tree and has two or more children.
    // In this case, we don't actually care about removing other bccs, as they will never be explored
    if (parent[u] == -1 && children > 1){
        is_art[u] = true;
        good_art[u] = true; // root is always good
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
    good_neigh_first_t.erase(good_neigh_first_t.begin(), good_neigh_first_t.end());
    root_correct_neigh.erase(root_correct_neigh.begin(), root_correct_neigh.end());
    visit_time = 0;
    found_s = false;
    // visits_performed_cat++;
    current_s = s;
    last_art = -1;

    for(int i = 0; i < G.size(); i++){
        visited[i] = false;
        is_art[i] = false;
        good_art[i] = false;
        parent[i] = -2;
    }
    
    parent[t] = -1;

    // cout << "About to find art points from "<< s << " to "<< t;
    // printGraph();

    for (auto x : bad_neigh_first_t){
        remove_simple(x);
        // cout <<"removing bad neighbor " << x << " of " << t; 
        }

    // printGraph();
    uint64_t start = timeMs();
     // only interested in the ones from s to t = caterpillar
    find_artpts(s, t);

    for (auto x : bad_neigh_first_t)
        reinsert_simple(x);

    bad_neigh_first_t.erase(bad_neigh_first_t.begin(), bad_neigh_first_t.end());


    // cout<< "BCC stack at the end: ";
    // for(int i= 0; i< cat_stack.size(); i++)
    //     cout << cat_stack[i] << " " ;
    // cout << endl;


    // SET UP BAD NEIGHBORS 

    // if this happened, then s and t are in the same BCC
    if (last_art== -1){
        last_art = t;
        good_neigh_first_t = root_correct_neigh;
    }

    // cout <<"Before setting bad neighbors: "<< endl;
    // cout<< "Good neighbors of last art "<< last_art <<": ";
    // for(int i= 0; i< good_neigh_first_t.size(); i++)
    //     cout << good_neigh_first_t[i] << " " ;
    // cout << endl;

    for (auto x : G[last_art]){   
        // cout << "considering neighbor " << x << " of last art "<< last_art << endl;
        // if it is not deleted and it does not appear in the good neighbors, it is a bad neighbor
        if(!deleted[x] && find(good_neigh_first_t.begin(),good_neigh_first_t.end(),x) == good_neigh_first_t.end()){
            bad_neigh_first_t.push_back(x);
            // cout << "It is a bad neighbor!"<<endl;
        }
    }

    // cout<< "Bad neighbors of last art "<< last_art <<": ";
    // for(int i= 0; i< bad_neigh_first_t.size(); i++)
    //     cout << bad_neigh_first_t[i] << " " ;
    // cout << endl;
    

    // cout << "Current last art: " << last_art<< endl;
    // printGraph();

    // cout << "good neighbors are "; 
    // for(auto x : good_neigh_first_t)
    //     cout << x << " ";
    // cout << endl; 

    // cout << "bad neighbors are "; 
    // for(auto x : bad_neigh_first_t)
    //     cout << x << " ";
    // cout << endl;  
    
    time_caterpillar += (timeMs() - start);
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

int visits_performed;

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
    // DFS(t);

    // cout << "Reachable nodes from t: ";
    // for(int i = 0; i < G.size(); i++)
    //     if(reachable[i])
    //         cout << i << " ";
    // cout << endl;


    // launch DFS from node t after having removed its bad neighbors
    for(auto x: bad_neigh_first_t)
        remove_simple(x);
    
    DFS(t);

    for(auto x: bad_neigh_first_t)
        reinsert_simple(x);

    

    // launch DFS from all good neighbors of node t, WHICH WE MAY NEED TO REINSERT SIMPLE
    // for (auto x : good_neigh_first_t)
    // {
    //     cout << "performing reachability from "<< x << endl;
    //     DFS(x);
    //     cout << "Reachable nodes: ";
    //     for(int i = 0; i < G.size(); i++)
    //         if(reachable[i])
    //             cout << i << " ";
    //     cout << endl;
    // }


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

    time_reachability += (timeMs() - start);

    return;
}




// paths must return the status, either success or fail
// we do so by returning true/false: true = success
bool paths_095(int u, int first_t, int t){
    // current_sol.push_back(u);
    curr_path_len++;

    if(calls_performed >= MAX_CALLS)
        return true;

    if(timeMs() - start_time >= MAX_TIME)
        return true;

    calls_performed++;

    if(calls_performed % 1000000 == 0)
        cout << "*" << flush;

    // base cases
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

    
    // cout << "Current node "<< u << endl;

    // we have non-deleted neighbors to explore
    if(degree(u) > 0){
        remove_node(u); // also adds to stack
        // if we get to the intermediate target, we update the target to t
        // we also need to keep copies of what the good/bad neighbors were to reconstitute them
        vector<int> old_bad_neigh = bad_neigh_first_t;
        vector<int> old_good_neigh = good_neigh_first_t;

        if(u== first_t){ // move directly to t
            // first_t = t;
            
            bad_neigh_first_t.erase(bad_neigh_first_t.begin(), bad_neigh_first_t.end());
            good_neigh_first_t = neighbors(t);
            
            // THESE FOLLOWING LINES ARE AN ALTERNATIVE TO THE FIRST ONE
            reinsert_simple(u);
            find_caterpillar(u, t); // compute caterpillar to delete useless neighbors and recompute next target
            remove_simple(u);
            first_t = last_art;
            visits_performed_cat_intermediate++;
        }

        bool success = true;
        int num_good_children = 0;
        for(auto v: G[u]){ 
            if(!deleted[v]){ // we take the next non-deleted element of G[u], noting that these deleted elements dynamically change during the for loop
                success = paths_095(v, first_t, t);

                if(success)
                    num_good_children++;

                // I need to make sure that if we are in the second condition, we de-stack and reinsert stuff
                if(degree(u) == 0 && lampadina){ // ALTERNATIVE
                    // cout <<"Backtracking"<<endl;
                    curr_path_len--;
                    dead_diff_len++;
                    // current_sol.pop_back();
                    // bad_neigh_first_t = old_bad_neigh;
                    // good_neigh_first_t = old_good_neigh;
                    return false; // return at first failing neighbor
                }

                // here we can go back, but re-inserting deleted nodes
                if(degree(u) == num_good_children && lampadina){ // NOTE: Degree is now > 0
                    // rimettere le cose a posto       
                    // pop stack until u (included) and mark as not deleted
                    // cout <<"Backtracking (no more good children)"<<endl;
                    while(del_stack.top() != u){
                        reinsert_node();
                    }
                    reinsert_node(); // here we are inserting u 

                    curr_path_len--;
                    good_diff_len++;

                    bad_neigh_first_t = old_bad_neigh;
                    good_neigh_first_t = old_good_neigh;

                    // current_sol.pop_back();
                    return true; // at least one child was good at this point
                }

                // SAME AS degree(u)>num_good_children AND LAMPADINA
                if(degree(u)>0 && lampadina){ //  HERE WE ARE AT THE ARTICULATION POINT
                    // cout <<"We are at articulation point; ";
                    lampadina=false;


                    // cout <<"about to call caterpillar from "<< u << " to " << first_t << endl;
                    
                    reinsert_simple(u);
                    // printGraph();
                    find_caterpillar(u, first_t); // compute caterpillar to delete useless neighbors
                    remove_simple(u);
                    first_t = last_art;
                    visits_performed_cat_og++;
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
        // current_sol.pop_back();

        // reconstitute neighbors and art point for parent call
        bad_neigh_first_t = old_bad_neigh;
        good_neigh_first_t = old_good_neigh;

        return true;
    }

    // here we are in the case where degree(u) = 0. If lampadina, we just return; else we perform the visit
    if(lampadina){
        
        // cout <<"Backtracking"<<endl;
        curr_path_len--;
        dead_diff_len++;
        // current_sol.pop_back();
        return false;
    }


    // here we are just arriving in a node of degree zero
    // VISITA: marca cancellati i nodi non raggiungibili da t + vettore reachable
    // cout <<"Reached a dead end in "<< u<<endl;
    // printGraph();
    reachability_check(first_t);
    // accendo la lampadina
    lampadina = true;

    // cout << "Reachable nodes: ";
    // for(int i = 0; i < G.size(); i++)
    //     if(reachable[i])
    //         cout << i << " ";
    // cout << endl;

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
    visits_performed_reach= 0;
    visits_performed_cat_intermediate= 0;
    visits_performed_cat_og = 0;
    paths_095(s, last_art, t);
    good_diff_len--; // source returned true and thus added one 

    return;
}

int main(){ 
    // char* input_filename = "graph-75-100.txt";
    create_graph(input_filename); // initialize 

    // printGraph();

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
    find_caterpillar(s,t); // last art is now set up
    // cout << "Last art point is " << last_art << endl; 

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
    
    // cout<< "Articulation points found: ";
    // for(int i= 0; i< G.size(); i++)
    //     cout << is_art[i] << " " ;
    // cout << endl;

    cout << "First caterpillar removed " << deleted_w_caterpillar << " nodes. " << endl;
    cout << "Nodes left are " << numnodes << ", and edges left are " << numedges << endl;
    
    // cout<< "Deleted nodes' vector: ";
    // for(int i= 0; i< G.size(); i++)
    //     cout << deleted[i] << " " ;
    // cout << endl;

    // cout<< "Bad neighbors of t: ";
    // for(int i= 0; i< bad_neigh_first_t.size(); i++)
    //     cout << bad_neigh_first_t[i] << " " ;
    // cout << endl;


    // printGraph();

    time_reachability = 0;
    time_caterpillar = 0;

    cout << "Insert max time in milliseconds: ";
    cin >> MAX_TIME;

    // MAX_TIME= 1000000;

    start_time = timeMs();

    // standard: s = 0, t=last node
    enumerate_paths(0, G.size()-1);
    uint64_t duration = (timeMs() - start_time);

    cout << endl<< "Elapsed time: " << duration << " ms; calls performed are " << calls_performed << endl;
    cout << "Visits performed are " << visits_performed_reach + visits_performed_cat_intermediate + visits_performed_cat_og <<" ; of which " << visits_performed_reach << " from reachability, " << visits_performed_cat_og << " from og caterpillar, and " << visits_performed_cat_intermediate << " from new cat " << endl;

    cout << "Paths found are " <<count_paths << " ; their total length is "<< total_length << " and their partial length is " << good_diff_len << endl;
    cout << "Dead ends are " << dead_ends << " ; their total length is " << dead_total_len << " and their partial length is " << dead_diff_len <<endl;
    cout << "Nodes removed with caterpillar are "<< deleted_w_caterpillar << endl;
    cout << "Time spent in reachability visits is "<< time_reachability << " ; time spent in caterpillar visits is " << time_caterpillar<< endl;


    // reporting to file
    ofstream output_file; 
    // output_file.open("output-v95.txt", ios::app);
    // output_file << "-----------------------------------------------------"<< endl;
    // output_file << "Output for graph with " << numnodes << " nodes, " << numedges << " edges and max degree " << maxdeg << " (" << input_filename << ")"<< endl;
    // output_file << calls_performed << " calls performed in " << duration << " ms (MAX_CALLS = " << MAX_CALLS << ")" << endl;
    // output_file << "Visits performed are " << visits_performed_reach + visits_performed_cat_intermediate + visits_performed_cat_og <<"; of which " << visits_performed_reach << " from reachability, " << visits_performed_cat_og << " from og caterpillar, and " << visits_performed_cat_intermediate << " from new cat " << endl;    
    // output_file << "Paths found are " <<count_paths << " for a total length of " << total_length << " and a partial length of " << good_diff_len << endl;
    // output_file<< "Dead ends are " << dead_ends << " for a total length of "<< dead_total_len << " and a partial length of " << dead_diff_len <<endl;
    // output_file << "Nodes removed with caterpillar are "<< deleted_w_caterpillar << endl;
    // output_file << "Time spent in reachability visits is "<< time_reachability << "; time spent in caterpillar visits is " << time_caterpillar<< endl;
    // output_file << "-----------------------------------------------------"<< endl<<endl<<endl;
    // output_file.close();

    // output_file.open("exhaustive-comparison.txt", ios::app);
    // output_file << "------------------------ VERSION 0.95 -----------------------------"<< endl;
    // output_file << "Output for graph with " << numnodes << " nodes, " << numedges << " edges and max degree " << maxdeg << " (" << input_filename << ")"<< endl;
    // output_file << calls_performed << " calls performed in " << duration << " ms (MAX_CALLS = " << MAX_CALLS << ")" << endl;
    // output_file << "Visits performed are " << visits_performed_reach + visits_performed_cat_intermediate + visits_performed_cat_og <<"; of which " << visits_performed_reach << " from reachability, " << visits_performed_cat_og << " from og caterpillar, and " << visits_performed_cat_intermediate << " from new cat " << endl;    // output_file << "Paths found are " <<count_paths << " for a total length of " << total_length << " and a partial length of " << good_diff_len << endl;
    // output_file << "Paths found are " <<count_paths << " for a total length of " << total_length << " and a partial length of " << good_diff_len << endl;
    // output_file<< "Dead ends are " << dead_ends << " for a total length of "<< dead_total_len << " and a partial length of " << dead_diff_len << endl;
    // output_file << "Nodes removed with caterpillar are "<< deleted_w_caterpillar << endl;
    // output_file << "Time spent in reachability visits is "<< time_reachability << "; time spent in caterpillar visits is " << time_caterpillar<< endl;
    // output_file << "-----------------------------------------------------"<< endl<<endl<<endl;
    // output_file.close();

    return 0;
}
