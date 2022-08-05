#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;
string outname;



// script to create a file containing a random sparse graph of size (both nodes and edges) given in input
void create_sparse(int N, int M){
    ofstream output_file; 
    output_file.open(outname);
    output_file << N << " " << M << endl;
    
    // for M times, extract a random pair in (0,N-1)
    for(int i = 0; i < M ; i++){
        int u = rand() % N; // u in the range 0 to N-1
        int v = rand() % N;
        while(v == u)
            v = rand() % N;

        output_file << u << ", " << v << endl;
    }

    output_file.close();
}


int main(){
    srand (time(NULL));
    int N,M;
    cout << "Insert number of nodes and edges of the graph: ";
    cin >> N >> M;
    cout << endl << "Insert name of destination file: ";
    cin >> outname;

    // N = 15;
    // M = 30;
    // outname = "graph-15-30.txt";

    create_sparse(N,M);

    return 0;
}