#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdint.h>
#include <inttypes.h>
#include <dirent.h>
#include <sstream>

#include <gsl/gsl_rng.h>
#include <random>

#include "variables.h"
#include "network.h"
#include "transactions.h"


using namespace std;
void print_network_params(const network_params& net_params ){
  cout<<"No. of nodes: "<<net_params.n_nodes<<"\n";
  cout<<"No. of avg degree: "<<net_params.degree.average<<"\n";
  cout<<"No. of degree 90th percentile: "<<net_params.degree.percentile_90th<<"\n";
  cout<<"No. of degree 50th percentile: "<<net_params.degree.percentile_50th<<"\n";
  cout<<"No. of degree 10th percentile: "<<net_params.degree.percentile_10th<<"\n";
  cout<<"No. of avg capacity: "<<net_params.capacity.average<<"\n";
  cout<<"No. of capacity 90th percentile: "<<net_params.capacity.percentile_90th<<"\n";
  cout<<"No. of capacity 50th percentile: "<<net_params.capacity.percentile_50th<<"\n";
  cout<<"No. of capacity 10th percentile: "<<net_params.capacity.percentile_10th<<"\n";
}

void read_input(network_params* net_params){

  net_params->n_nodes = 0;
  net_params->degree.average = 0;
  net_params->degree.percentile_90th = 0;
  net_params->degree.percentile_50th = 0;
  net_params->degree.percentile_10th = 0;
  net_params->capacity.average = 0;
  net_params->capacity.percentile_90th = 0;
  net_params->capacity.percentile_50th = 0;
  net_params->capacity.percentile_10th = 0;
  net_params->fees_lower_limit.base_fee = 0;
  net_params->fees_upper_limit.base_fee = 0;
  net_params->fees_lower_limit.proportional_fee_const = 0;
  net_params->fees_upper_limit.proportional_fee_const = 0;
  net_params->fees_lower_limit.imbalance_fee_const = 0;
  net_params->fees_upper_limit.imbalance_fee_const = 0;
  net_params->txn_amount_lower_limit = 1;
  net_params->txn_amount_upper_limit = 1;

  string line;
  ifstream input_file ("input.txt");
  if (input_file.is_open())
  {
    string key;
    while(getline(input_file, key, '=') )
    {
      string value;
      if(getline(input_file, value) ) 
        if(key == "n_nodes")
          net_params->n_nodes = stoi(value);
        else if(key == "degree_average")
          net_params->degree.average = stoi(value);
        else if(key == "degree_90th_percentile")
          net_params->degree.percentile_90th = stoi(value);
        else if(key == "degree_50th_percentile")
          net_params->degree.percentile_50th = stoi(value);
        else if(key == "degree_10th_percentile")
          net_params->degree.percentile_10th = stoi(value);
        else if(key == "capacity_average")
          net_params->capacity.average = stoi(value);
        else if(key == "capacity_90th_percentile")
          net_params->capacity.percentile_90th = stoi(value);
        else if(key == "capacity_50th_percentile")
          net_params->capacity.percentile_50th = stoi(value);
        else if(key == "capacity_10th_percentile")
          net_params->capacity.percentile_10th = stoi(value);
        else if(key == "base_fee_lower")
          net_params->fees_lower_limit.base_fee = stol(value);
        else if(key == "base_fee_upper")
          net_params->fees_upper_limit.base_fee = stol(value);
        else if(key == "proportional_fee_const_lower")
          net_params->fees_lower_limit.proportional_fee_const = stol(value);
        else if(key == "proportional_fee_const_upper")
          net_params->fees_upper_limit.proportional_fee_const = stol(value);
          else if(key == "imbalance_fee_const_lower")
          net_params->fees_lower_limit.imbalance_fee_const = stol(value);
        else if(key == "imbalance_fee_const_upper")
          net_params->fees_upper_limit.imbalance_fee_const = stol(value);
        else if(key == "num_of_txn_sets")
          net_params->num_of_txn_sets = stoi(value);
        else if(key == "num_of_txn")
          net_params->num_of_txn = stoi(value);
        else if(key == "txn_amount_lower_limit")
          net_params->txn_amount_lower_limit = stol(value);
        else if(key == "txn_amount_upper_limit")
          net_params->txn_amount_upper_limit = stol(value);
        else if(key == "faulty_node_probability")
          net_params-> faulty_node_probability = stod(value);
        else if(key == "path_calculation_time_lower_bound")
          net_params-> path_calculation_time_lower_bound = stoi(value);
        else if(key == "path_calculation_time_upper_bound")
          net_params-> path_calculation_time_upper_bound = stoi(value);
        else if(key == "hop_processing_time")
          net_params-> hop_processing_time = stoi(value);
        else if(key == "cooldown_time")
          net_params-> cooldown_time = stoi(value);
        else
          cout<<"Wrong input----------------------\n\n\n\n\n\n\n\n\\n\n\n";
    }
    
    input_file.close();

  cout<<"Avg degree: "<<net_params->degree.average<<" 90th percentile: "<<net_params->degree.percentile_90th<<"\n";
  }
  else cout << "Unable to open file"; 
  // print_network_params(*net_params);
}

void print_network_stats(network* net){
  ofstream myfile;
  myfile.open ("graph_props.txt");
  myfile<<net->nodes.size()<<"\t"<<net->edges.size()<<"\n";
  // Node id starts from 1
  for(int i=1;i<=net->nodes.size();i++){
    myfile<<net->nodes[i]->node_fee->base_fee<<"\t"<<net->nodes[i]->node_fee->proportional_fee_const<<"\n";
  }
  // all edges
  for(auto x: net->edges){
    // x is iterator in map
    myfile<<(x.second)->from_node_id<<"\t"<<(x.second)->to_node_id<<"\t"<<(x.second)->balance<<"\n";
  }
  myfile.close();
}

int main(int argc, char *argv[]) {
  
  network_params net_params;
  
  network *new_network;
  long n_nodes, n_edges;

  read_input(&net_params);

  printf("NETWORK INITIALIZATION\n");
  new_network = initialize_network(net_params);


  print_network_stats(new_network);

  n_nodes = new_network->nodes.size();
  n_edges = new_network->edges.size();

  // stores which transaction is present at each node in a step;
  // For example at a moment of time if `T1`, `T3`, `T4` are leaving the node `Ni`, 
  // then Transaction_present[i][1], [3],[4] will be true
  // and rest transaction[i][j] will be false.
  vector<vector<pair<bool,int>>> transactions_present(n_nodes+1, vector<pair<bool,int>>(net_params.num_of_txn+1, {false,0}));

  // cout<<"Beware:::::::::@@@@@@@ This is the previous implementation.\n\n\n\n\n\n\n\n\n\n\n\n";
  network* dummy_network = new network;
  *dummy_network = (*new_network);
  auto dummy_txn_pre = transactions_present;

  
  for(int i=0;i<net_params.num_of_txn_sets;i++){
//     cout<<"\n$$$$$$$$$$$$$$$-------------------------------------------------$$$$$$$$$$$$$$$$$$$\n";
    cout<<"Transaction set: "<<i+1<<"\n\n";
    vector<transaction> transactions_to_execute= get_random_transactions(net_params);
    vector<transaction> dummy_txn = transactions_to_execute;

    // Uses bellman ford algo, data can be found in transaction_store_BellmanFord.txt
//     process_payments_bellman_ford(dummy_network, net_params, dummy_txn,dummy_txn_pre);

//     // Uses DFS algo, data can be found in transaction_store_DFS.txt
//     process_payments(new_network, net_params, transactions_to_execute,transactions_present);
    
    process_payments_using_pqueue(new_network, net_params, transactions_to_execute);

    // This is the previous implementation using priority queue
    // cout<<"Beware:::::::::@@@@@@@ This is the previous implementation.\n\n";
    // process_payments_previous_imple(dummy_network, net_params, transactions_to_execute);
  }

  return 0;
}
