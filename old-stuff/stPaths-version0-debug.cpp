#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;
vector<int> deleted;

typedef vector<vector<int>> graph;
graph G;


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
vector<bool> check_neighbors(int s, int t){
    vector<bool> visited(G.size());

    // DELETION OF S PERFORMED BEFORE CALL TO FUNCTION
    // before starting, mark s as deleted
    // deleted[s] = 1;

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

    vector<int> neigh = neighbors(s);
    // find out which neighbors of s have been visited, and output them
    vector<bool> good_neighbors(neigh.size());
    for(int i = 0; i < neigh.size(); i++){
        if(visited[neigh[i]])
            good_neighbors[i] = true;
            // good_neighbors.push_back(G[s][i]);
        else
            good_neighbors[i] = false;
            
    }
    
    // cout << "Before outputting good neighbors: size is " << good_neighbors.size() << ", while size of neighbors is "<< neigh.size() << endl<< flush;

    // undo deletion of s 
    // deleted[s] = 0;

    return good_neighbors;
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

// paths must return the status, either success or fail
// we do so by returning true/false: true = success
bool paths(int s, int t){
    calls_performed++;
    curr_path_len++;
    // cout << endl << "Inside call with s=" << s << " and t=" << t << "; call number " << calls_performed <<  endl<< flush;
    // cout << "Current path length is "<< curr_path_len << "; total length is " << total_length << endl; 
    // cout << "Total dead ends' length so far is "<< dead_total_len << endl<< endl;
    // if(calls_performed % 1000 == 0){
    //     cout << "Call number " << calls_performed <<  flush;
    //     cout << "; so far, paths found are " << count_paths << " and dead ends are "<< dead_ends << endl << flush;
    // }

    if(calls_performed >= MAX_CALLS)
        return true;
    
    // if(calls_performed % 1000 == 0)
    //     cout << "Performed a thousand calls"<< endl << flush;

    if(s == t){
        count_paths++;
        good_diff_len++;
        // cout << "Exiting function and returning true " << endl << endl;

        // every time we arrive at t, we sum to the total length the current path length
        // we also need to decrease the current path length
        total_length += curr_path_len;
        curr_path_len--;
        return true;
    }
    
    vector<int> curr_neigh = neighbors(s);
    // cout << "Number of neighbors of s are " << curr_neigh.size()<< endl<<flush;
    // cout << "Current neighbors of s are: ";
    // for(auto x : curr_neigh)
    //     cout << x << ", ";
    // cout << endl << flush;

    if(curr_neigh.size() == 0){
        cout << "Exiting function and returning false for s=" <<s << ": INCREASING TOTAL DEAD ENDS " << endl << endl;
        // dead ends is increased: we failed on a node
        dead_ends++;

        // cout << "Adding " << curr_path_len << " to total dead len" << endl;


        // increase total dead ends' length
        dead_total_len = dead_total_len + curr_path_len;
        dead_diff_len++;

        // decrease current path length as we are backtracking
        curr_path_len--;

        
        return false;
    }
        


    deleted[s] = 1;

    // printGraph();

    // the return value is the OR of the values for the neighbors
    // bool ret_value = false;
    bool neigh_value = true;
    bool ret_value = false;
    bool first_good = false;
    // int num_good_neigh = 0; // counter needed for good_diff_len: the latter is increased only if exactly one good neighbor
    int i = 0;
    while(neigh_value && i < curr_neigh.size()){
        // cout << "Inside while loop for s=" << s << "; neighbor is " << curr_neigh[i] << endl;
        neigh_value = paths(curr_neigh[i], t);
        ret_value = ret_value || neigh_value;


        // if the call returned true, and I am the first neighbor to do so, increase good diff len
        // if(neigh_value && !first_good){
        //     cout << "A)Increasing partial good length: s=" << s << " has first good neighbor " << curr_neigh[i] << endl;
        //     good_diff_len++;
        // }

        // as soon as one neighbor is good, first_good becomes true
        // if(neigh_value)
        //     first_good= true;
        
        // the check needs to be performed here: after we return the first good
        // neighbor, we increase the good explored part 
        // if(neigh_value && num_good_neigh==1){
        //     cout << "Increasing partial good length: s=" << s << " has first good neighbor " << curr_neigh[i] << endl;
        //     good_diff_len++;
        // }
            

        if(calls_performed >= MAX_CALLS){
            deleted[s] = 0;
            return true;
        }

        i++;
    }

    // if we found a failing neighbor, perform visit from t
    if(!neigh_value){
        // cout << "Found a failing neighbor for " << s << endl;
        // printGraph();
        // cout << "Non-deleted neighbors are: ";
        // for(auto x : neighbors(s)) 
        //     cout << x <<" ";
        // cout << endl;    

        // cout << " s = " << s << endl << flush; 
        // cout << "Number of neighbors of s are " << curr_neigh.size()<< endl<<flush;
        // cout << "The supposed neighbors are: "<< flush;
        // for(auto nn : curr_neigh)
        //     cout << nn << " ";
        // cout << endl;

        // cout << "Recomputing neighbors we obtain they are "<< neighbors(s).size() << endl << flush;
        // cout << "about to compute good" << endl << flush;


        vector<bool> good_neigh = check_neighbors(s, t);
        // cout << "Good neighbors array: ";
        // for(auto x : good_neigh) 
        //     cout << x <<" ";
        // cout << endl;

        // cout << "Good neighbors of s: ";
        // for(int j = 0; j< curr_neigh.size(); j++){
        //     if(good_neigh[j])
        //         cout << curr_neigh[j] <<" ";
        // }
        // cout << endl;

        // cout << "found good neighbors; they are " << curr_neigh.size() << endl << flush;
        // cout << "size of good neigh: " << good_neigh.size() << endl << flush;

        // at this point, resume where we left off to recurse in good neighbors
        for (; i < curr_neigh.size(); i++)
        {
            // cout << "Index i =" << i << endl;

            // cout << "good neigh at index i is" << good_neigh[i]<< endl << flush;

            if(good_neigh[i]){
                // cout << "Inside loop for good neighbors; neighbor is " << curr_neigh[i] << endl<< flush;
                bool test = paths(curr_neigh[i], t);
                // cout << "exited here B" << endl << flush;
                ret_value = ret_value || test;
                // ret_value = ret_value || paths(curr_neigh[i], t);

               // if the call returned true, and I am the first neighbor to do so, increase good diff len
                // if(test && !first_good){
                //     cout << "B)Increasing partial good length: s=" << s << " has first good neighbor " << curr_neigh[i] << endl;
                //     good_diff_len++;
                // }

                // as soon as one neighbor is good, first_good becomes true
                // if(test)
                //     first_good= true;

                if(calls_performed >= MAX_CALLS){
                    deleted[s] = 0;
                    return true;
                }
            }
                
        }
    }


    deleted[s] = 0;
    // cout << "Reinserting s=" << s <<endl;
    // printGraph();
    // cout << "Exiting function and returning " << ret_value << " for s =" << s << endl;

    if(!ret_value){
        // cout << "About to return false, INCREASING TOTAL DEAD ENDS" << endl;
        // curr_path_len--;
        dead_diff_len++;
        // cout << "Adding " << curr_path_len << " to total dead len" << endl;
        // dead_total_len = dead_total_len + curr_path_len;
    }

    if(ret_value)
        good_diff_len++;


    // we need to decrease current path length IN ANY CASE when returning
    curr_path_len--;

    return ret_value;
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
    paths(s,t);
    good_diff_len--;

    return;
}

int main(){ 
    char* input_filename = "test2.txt";
    create_graph(input_filename);

    printGraph();


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

    // dead_ends = 0;
    // calls_performed = 0;
    // total_length = 0;

    // standard: s = 0, t=last node
    // enumerate_paths(0, G.size()-1);
    enumerate_paths(0,6); // for small example
    duration = (clock() - start) / (double) CLOCKS_PER_SEC;

    cout <<  "Elapsed time: " << duration << " sec; calls performed are " << calls_performed << endl;

    cout << "Paths found are " <<count_paths << "; their total length is "<< total_length << " and their partial length is " << good_diff_len << endl;
    cout << "Dead ends are " << dead_ends << "; their total length is " << dead_total_len << " and their partial length is " << dead_diff_len <<endl;

    // reporting to file
    ofstream output_file; 
    // output_file.open("smol-output.txt", ios::app);
    // output_file << "-----------------------------------------------------"<< endl;
    // output_file << "Output for graph with " << numnodes << " nodes, " << numedges << " edges and max degree " << maxdeg << " (" << input_filename << ")"<< endl;
    // output_file << calls_performed << " calls performed in " << duration << " secs (MAX_CALLS = " << MAX_CALLS << ")" << endl;
    // output_file << "Paths found are " <<count_paths << " for a total length of " << total_length << endl;
    // output_file<< "Dead ends are " << dead_ends << " for a total length of "<< dead_total_len <<endl;
    // output_file << "-----------------------------------------------------"<< endl<<endl<<endl;
    // output_file.close();

    return 0;
}
