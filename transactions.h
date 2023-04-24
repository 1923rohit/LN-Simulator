#include <bits/stdc++.h>
#include "variables.h"
#define ll long long int
using namespace std;

// Processes payments
void process_payments(network* cur_network, const network_params& net_params, vector<transaction>& transactions_to_execute, vector<vector<pair<bool,int>>>& transactions_present);

void process_payments_bellman_ford(network* cur_network, const network_params& net_params, vector<transaction>& transactions_to_execute, vector<vector<pair<bool,int>>>& transactions_present);

// Process payments(previous implementation) which uses priority queue.
void process_payments_using_pqueue(network* cur_network, const network_params& net_param, vector<transaction>& transactions_to_execute);

// Returns a set of transactions
vector<transaction> get_random_transactions(const network_params& net_params);

// Return false if transaction is not viable, either due to invalid send/receiver id or 
// amount to send exceeds all the edge balances of sender.
bool payment_request(transaction& req, network* cur_network);

// Calculates fee taken by a node
double calc_fee(node* cur_node, long long int amount, int balance_diff, int num_of_txn);

// Returns the payment path for a given request.
// Returns empty vector if doesn't exist.
// Uses Dijkstra algo to find path
vector<path_var> get_payment_path_using_Dijkstra(transaction& req, network* cur_network,const network_params& net_params);

// Execute a transaction.
vector<pair<edge*, long long int>> execute_transaction(transaction& req, const vector<int>& dis, const vector<pair<int,edge*>>& path, network* cur_network);

// Returns random time valuee between given bounds.
int get_hop_processing_time(int lower_limit, int upper_limit);

// Returns path calculation time (random values)
// Values vary between 100-200
int get_path_calculation_time();

// Uses Bellman ford algorithm to find the path, which is distributed.
vector<path_var> get_payment_path_using_bellman_ford(transaction& req, network* cur_network,const network_params& net_params);

// Uses DFS algorithm.
vector<path_var> get_payment_path_using_DFS(transaction& req, network* cur_network,const network_params& net_params);