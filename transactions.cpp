#include <bits/stdc++.h>
#include <cstdlib>
#include <random>
#include "variables.h"
#include "transactions.h"
#define ll long long int

using namespace std;

// For priority queue to compare.
struct compare_events{
    bool operator()(const pair<int, path_var>& p1, const pair<int, path_var>& p2){
        return p1.second.time_taken > p2.second.time_taken;
    }
};

// Comparator for vector to be sorted in ascending order of time taken.
bool compare_by_ready_time(path_var& a, path_var& b){
    return (a.time_taken < b.time_taken);
}
// uses dfs
void process_payments(network* cur_network, const network_params& net_params, vector<transaction>& transactions_to_execute, vector<vector<pair<bool,int>>>& transactions_present){
  // Stores the events to be processed i.e. edges to be processed.
  // The top element in the queue is the one with the least time(ready time).
  // {event type, event}
  vector<path_var> process_events;
  ofstream txn_file;
  txn_file.open("transaction_store_DFS.txt");
  // Payment request, finds a path for each transaction requested.
  for(int ind=0;ind<transactions_to_execute.size();ind++){
    auto& cur_txn = transactions_to_execute[ind];
    // This transaction might be completed.
    vector<path_var> path;
    // path = get_payment_path_using_bellman_ford(cur_txn, cur_network, net_params);
    path = get_payment_path_using_DFS(cur_txn, cur_network, net_params);

    cur_txn.path_length = path.size()-1;
    txn_file<<cur_network->nodes[transactions_to_execute[ind].sender_id]->path_calculation_time<<"\t"<<transactions_to_execute[ind].amount
    <<"\t"<<transactions_to_execute[ind].sender_id<<"\t"<<transactions_to_execute[ind].receiver_id<<"\n";
    // cout<<cur_network->nodes[transactions_to_execute[ind].sender_id]->path_calculation_time<<"\t"<<transactions_to_execute[ind].amount
    // <<"\t"<<transactions_to_execute[ind].sender_id<<"\t"<<transactions_to_execute[ind].receiver_id<<"\n";
    
    if(path.empty()){
    txn_file<<"-1"<<"\n";
    // cout<<"Not possible to reach the destination due to lack of balances.\n\n\n";
    continue;
    }
    for(auto path_temp:path){
        txn_file<<path_temp.node_id<<"\t";
        // cout<<path_temp.node_id<<"\t";
    }
    txn_file<<"\n";
    // cout<<"\n";
//   cout<<"Sender: "<<cur_txn.sender_id<<" Receiver: "<< cur_txn.receiver_id<<" Amt: "<<cur_txn.amount<<"\n";
//   cout<<"Fee taken "<<cur_txn.fee<<"\n";

//   cout<<"Path taken, len :"<<path.size()<<"\n";
    for(int i=0;i<path.size();i++){
    // cout<<path[i].node_id<<" --> ";
    // path[i].txn_id = ind;
    process_events.push_back(path[i]);
    }
  }
  sort(process_events.begin(),process_events.end(),compare_by_ready_time);

  vector<vector<path_var>> hops_processed(transactions_to_execute.size()+1);
  // Now we have the events to be processed and the time when they are ready to be processed.
  for(int i=0;i<process_events.size();){
    int cur_time = process_events[i].time_taken;
    int max_time = cur_time + net_params.cooldown_time;
    int j=i;
    int max_cnt=0;
    vector<int> txn_count(net_params.n_nodes+1,0);

    // store the transaction which are ready between the time cur_time to cur_time + cooldown_time
    for(j=i;j<process_events.size() && process_events[j].time_taken <= max_time;j++){
        int node_txn_passes = process_events[j].node_id;
        transactions_present[node_txn_passes][process_events[j].txn_id] = {true,j};
        txn_count[node_txn_passes]++;
        // cout<<"See the node though which txn passes "<<node_txn_passes<<" and the txn num is "<<process_events[j].txn_id<<"\n";
        max_cnt = max(max_cnt, txn_count[node_txn_passes]);
    }
    i=j;
    // cout<<"Max count "<<max_cnt<<"\n";
    // execute the transaction one at a time for each node by looking at the priority for each node in the order.
    for(int temp_num=0;temp_num<max_cnt;temp_num++){

        // for each node, execute the transaction with the highest/lowest priority
        for(int node_ind=1; node_ind <= net_params.n_nodes; node_ind++){
            if(txn_count[node_ind]>0){
                // cout<<"See the node id "<<node_ind<<" and txn count "<<txn_count[node_ind]<<"\n";
                double max_priority=0.0;
                int ind_with_max_priority=-1;
                node* temp_node = cur_network->nodes[node_ind];
                for(int txn_ind=0; txn_ind< net_params.num_of_txn; txn_ind++){
                    if(transactions_present[node_ind][txn_ind].first && temp_node->transaction_priorities[txn_ind] >= max_priority){
                        max_priority = temp_node->transaction_priorities[txn_ind];
                        ind_with_max_priority = txn_ind;
                    }
                }

                // process the txn `ind_with_max_priority`

                // execute the txn
                path_var& cur_path_var = process_events[transactions_present[node_ind][ind_with_max_priority].second];
                // update the variables
                // decrease the remaining txn count
                txn_count[node_ind]--;
                // this transaction will be executed, so next time it isn't present.
                transactions_present[node_ind][ind_with_max_priority]={false,0};

                int cur_txn_id = cur_path_var.txn_id;
                // Case 1: the transaction is cancelled. no need to process its next steps.
                if(transactions_to_execute[cur_txn_id].status==1){
                    // This transaction is cancelled. Do not process any further steps.
                    continue;
                }
                node* cur_node = cur_network->nodes[cur_path_var.node_id];
                edge* outgoing_edge = cur_path_var.outgoing_edge;
                // If this is the destination,  update the balances and give the fees to intermediate nodes.
                if(!outgoing_edge){
                    // This transaction is complete.
                    for(int k=0;k<hops_processed[cur_txn_id].size();k++){
                        path_var temp_int_node = hops_processed[cur_txn_id][k];
                        // if(cur_network->edges.find(temp_int_node.outgoing_edge->counter_edge_id) == cur_network->edges.end() ){
                        //     cout<<"Edge mapping not found, line 82. \n";
                        //     return;
                        // }
                        edge* incoming_edge = cur_network->edges[temp_int_node.outgoing_edge->counter_edge_id];
                        incoming_edge->balance += temp_int_node.amount_passed;
                        // if(cur_network->nodes.find(temp_int_node.node_id) == cur_network->nodes.end() ){
                        //     cout<<"Node mapping not found, line 88 \n";
                        //     return;
                        // }
                        node* temp_node = cur_network->nodes[temp_int_node.node_id];
                        temp_node->fee_collected += temp_int_node.fee_taken;
                    }
                    transactions_to_execute[cur_txn_id].status = 2;
                    // transaction is complete.
                    continue;
                }
                if(outgoing_edge->balance < cur_path_var.amount_passed){
                    // Inactive node or Not enough balance.
                    // Cancel txn.
                    transactions_to_execute[cur_txn_id].status = 1;
                    // Reverted txn. Here it happens instantaneously.
                    for(int k=0;k<hops_processed[cur_txn_id].size();k++){
                        edge* temp_edge = hops_processed[cur_txn_id][k].outgoing_edge;
                        temp_edge->balance += hops_processed[cur_txn_id][k].amount_passed;
                    }
                    continue;
                }
                outgoing_edge->balance -=cur_path_var.amount_passed;
                hops_processed[cur_txn_id].push_back(cur_path_var);

            }
        }
    }
  }

  // Calculate network statistics.
  int txn_completed=0;
  double average_path_length=0;
  long long int average_fee=0;
  for(int ind=0;ind< transactions_to_execute.size();ind++){
    if(transactions_to_execute[ind].status==2){
        txn_completed++;
        average_path_length+= transactions_to_execute[ind].path_length;
        average_fee += transactions_to_execute[ind].fee;
        if(transactions_to_execute[ind].fee<0){
            cout<<"Neg fee : "<<transactions_to_execute[ind].fee<<" ind "<<ind<<"\n";
        }
    }
  }
  cout<<"Using DFS algo---\n";
  // Print transaction statistics
  if(txn_completed>0){
    average_path_length/= (txn_completed);
    average_fee/= (txn_completed);
    cout<<"Number of transactions completed: "<<txn_completed<<" out of "<<transactions_to_execute.size()<<"\n";
    cout<<"Average path length: "<<average_path_length<<"\n";
    cout<<"Average fee : "<<average_fee<<"\n";
  }else{
    cout<<"No transaction completed.\n";
  }
  txn_file.close();

}

void process_payments_bellman_ford(network* cur_network, const network_params& net_params, vector<transaction>& transactions_to_execute, vector<vector<pair<bool,int>>>& transactions_present){
  // Stores the events to be processed i.e. edges to be processed.
  // The top element in the queue is the one with the least time(ready time).
  // {event type, event}
  vector<path_var> process_events;
  ofstream txn_file;
  txn_file.open("transaction_store_BellmanFord.txt");
  // Payment request, finds a path for each transaction requested.
  for(int ind=0;ind<transactions_to_execute.size();ind++){
    auto& cur_txn = transactions_to_execute[ind];
    // This transaction might be completed.
    vector<path_var> path;
    path = get_payment_path_using_bellman_ford(cur_txn, cur_network, net_params);
    // path = get_payment_path_using_DFS(cur_txn, cur_network, net_params);

    cur_txn.path_length = path.size()-1;
    txn_file<<cur_network->nodes[transactions_to_execute[ind].sender_id]->path_calculation_time<<"\t"<<transactions_to_execute[ind].amount
    <<"\t"<<transactions_to_execute[ind].sender_id<<"\t"<<transactions_to_execute[ind].receiver_id<<"\n";
    // cout<<cur_network->nodes[transactions_to_execute[ind].sender_id]->path_calculation_time<<"\t"<<transactions_to_execute[ind].amount
    // <<"\t"<<transactions_to_execute[ind].sender_id<<"\t"<<transactions_to_execute[ind].receiver_id<<"\n";
    
    if(path.empty()){
    txn_file<<"-1"<<"\n";
    // cout<<"Not possible to reach the destination due to lack of balances.\n\n\n";
    continue;
    }
    for(auto path_temp:path){
        txn_file<<path_temp.node_id<<"\t";
        // cout<<path_temp.node_id<<"\t";
    }
    txn_file<<"\n";
    // cout<<"\n";
//   cout<<"Sender: "<<cur_txn.sender_id<<" Receiver: "<< cur_txn.receiver_id<<" Amt: "<<cur_txn.amount<<"\n";
//   cout<<"Fee taken "<<cur_txn.fee<<"\n";

//   cout<<"Path taken, len :"<<path.size()<<"\n";
    for(int i=0;i<path.size();i++){
    // cout<<path[i].node_id<<" --> ";
    // path[i].txn_id = ind;
    process_events.push_back(path[i]);
    }
  }
  sort(process_events.begin(),process_events.end(),compare_by_ready_time);

  vector<vector<path_var>> hops_processed(transactions_to_execute.size()+1);
  // Now we have the events to be processed and the time when they are ready to be processed.
  for(int i=0;i<process_events.size();){
    int cur_time = process_events[i].time_taken;
    int max_time = cur_time + net_params.cooldown_time;
    int j=i;
    int max_cnt=0;
    vector<int> txn_count(net_params.n_nodes+1,0);

    // store the transaction which are ready between the time cur_time to cur_time + cooldown_time
    for(j=i;j<process_events.size() && process_events[j].time_taken <= max_time;j++){
        int node_txn_passes = process_events[j].node_id;
        transactions_present[node_txn_passes][process_events[j].txn_id] = {true,j};
        txn_count[node_txn_passes]++;
        // cout<<"See the node though which txn passes "<<node_txn_passes<<" and the txn num is "<<process_events[j].txn_id<<"\n";
        max_cnt = max(max_cnt, txn_count[node_txn_passes]);
    }
    i=j;
    // cout<<"Max count "<<max_cnt<<"\n";
    // execute the transaction one at a time for each node by looking at the priority for each node in the order.
    for(int temp_num=0;temp_num<max_cnt;temp_num++){

        // for each node, execute the transaction with the highest/lowest priority
        for(int node_ind=1; node_ind <= net_params.n_nodes; node_ind++){
            if(txn_count[node_ind]>0){
                // cout<<"See the node id "<<node_ind<<" and txn count "<<txn_count[node_ind]<<"\n";
                double max_priority=0.0;
                int ind_with_max_priority=-1;
                node* temp_node = cur_network->nodes[node_ind];
                for(int txn_ind=0; txn_ind< net_params.num_of_txn; txn_ind++){
                    if(transactions_present[node_ind][txn_ind].first && temp_node->transaction_priorities[txn_ind] >= max_priority){
                        max_priority = temp_node->transaction_priorities[txn_ind];
                        ind_with_max_priority = txn_ind;
                    }
                }

                // process the txn `ind_with_max_priority`

                // execute the txn
                path_var& cur_path_var = process_events[transactions_present[node_ind][ind_with_max_priority].second];
                // update the variables
                // decrease the remaining txn count
                txn_count[node_ind]--;
                // this transaction will be executed, so next time it isn't present.
                transactions_present[node_ind][ind_with_max_priority]={false,0};

                int cur_txn_id = cur_path_var.txn_id;
                // Case 1: the transaction is cancelled. no need to process its next steps.
                if(transactions_to_execute[cur_txn_id].status==1){
                    // This transaction is cancelled. Do not process any further steps.
                    continue;
                }
                node* cur_node = cur_network->nodes[cur_path_var.node_id];
                edge* outgoing_edge = cur_path_var.outgoing_edge;
                // If this is the destination,  update the balances and give the fees to intermediate nodes.
                if(!outgoing_edge){
                    // This transaction is complete.
                    for(int k=0;k<hops_processed[cur_txn_id].size();k++){
                        path_var temp_int_node = hops_processed[cur_txn_id][k];
                        // if(cur_network->edges.find(temp_int_node.outgoing_edge->counter_edge_id) == cur_network->edges.end() ){
                        //     cout<<"Edge mapping not found, line 82. \n";
                        //     return;
                        // }
                        edge* incoming_edge = cur_network->edges[temp_int_node.outgoing_edge->counter_edge_id];
                        incoming_edge->balance += temp_int_node.amount_passed;
                        // if(cur_network->nodes.find(temp_int_node.node_id) == cur_network->nodes.end() ){
                        //     cout<<"Node mapping not found, line 88 \n";
                        //     return;
                        // }
                        node* temp_node = cur_network->nodes[temp_int_node.node_id];
                        temp_node->fee_collected += temp_int_node.fee_taken;
                    }
                    transactions_to_execute[cur_txn_id].status = 2;
                    // transaction is complete.
                    continue;
                }
                if(outgoing_edge->balance < cur_path_var.amount_passed){
                    // Inactive node or Not enough balance.
                    // Cancel txn.
                    transactions_to_execute[cur_txn_id].status = 1;
                    // Reverted txn. Here it happens instantaneously.
                    for(int k=0;k<hops_processed[cur_txn_id].size();k++){
                        edge* temp_edge = hops_processed[cur_txn_id][k].outgoing_edge;
                        temp_edge->balance += hops_processed[cur_txn_id][k].amount_passed;
                    }
                    continue;
                }
                outgoing_edge->balance -=cur_path_var.amount_passed;
                hops_processed[cur_txn_id].push_back(cur_path_var);

            }
        }
    }
  }

  // Calculate network statistics.
  int txn_completed=0;
  double average_path_length=0;
  long long int average_fee=0;
  for(int ind=0;ind< transactions_to_execute.size();ind++){
    if(transactions_to_execute[ind].status==2){
        txn_completed++;
        average_path_length+= transactions_to_execute[ind].path_length;
        average_fee += transactions_to_execute[ind].fee;
        if(transactions_to_execute[ind].fee<0){
            cout<<"Neg fee : "<<transactions_to_execute[ind].fee<<" ind "<<ind<<"\n";
        }
    }
  }
  cout<<"Using Bellman ford algo---\n";
  // Print transaction statistics
  if(txn_completed>0){
    average_path_length/= (txn_completed);
    average_fee/= (txn_completed);
    cout<<"Number of transactions completed: "<<txn_completed<<" out of "<<transactions_to_execute.size()<<"\n";
    cout<<"Average path length: "<<average_path_length<<"\n";
    cout<<"Average fee : "<<average_fee<<"\n";
  }else{
    cout<<"No transaction completed.\n";
  }
  txn_file.close();
}

void process_payments_using_pqueue(network* cur_network, const network_params& net_param, vector<transaction>& transactions_to_execute){
  // Code for finding if the node has become inactive randmoly with given probability.

  // If the probability is 0.05, then we generate a random number b/w 0 to 100, if it is less 
  // than 5 then it is faulty, else not.
  std::default_random_engine generator;
  double faulty_prob = net_param.faulty_node_probability;
  ll num=1;
  while(faulty_prob < 1.0 && net_param.faulty_node_probability > 0.0){
    faulty_prob*=10;
    num*=10;
  }
  std::uniform_int_distribution<int> distribution(1,num);

  srand(time(NULL));
  // Stores the events to be processed i.e. edges to be processed.
  // The top element in the queue is the one with the least time(ready time).
  // {event type, event}
  priority_queue<pair<int, path_var>, vector<pair<int, path_var>>, compare_events> process_events;
  
  // Payment request, finds a path for each transaction requested.
  for(int ind=0;ind<transactions_to_execute.size();ind++){
    auto& cur_txn = transactions_to_execute[ind];
    // cur_txn.sender_id=1;
    // cur_txn.receiver_id=2;
    if(payment_request(cur_txn, cur_network)){
      // This transaction might be completed.
      vector<path_var> path;
      path = get_payment_path_using_bellman_ford(cur_txn, cur_network, net_param);

      cur_txn.path_length = path.size()-1;
      if(path.empty()){
        // cout<<"Not possible to reach the destination due to lack of balances.\n\n\n";
        continue;
      }
    //   cout<<"Sender: "<<cur_txn.sender_id<<" Receiver: "<<cur_txn.receiver_id<<" Amt: "<<cur_txn.amount<<"\n";
    //   cout<<"Fee taken "<<cur_txn.fee<<"\n";

    //   cout<<"Path taken, len :"<<path.size()<<"\n";
      for(int i=0;i<path.size();i++){
        // cout<<path[i].node_id<<" --> ";
        path[i].txn_id = ind;
        process_events.push(make_pair(1,path[i]));
        // Event type is 1, i.e. it represents normal hop, not reverted one
      }
    //   cout<<"\n";
    }else{
      // This transaction can't be executed.
    }
  }

    vector<vector<path_var>> hops_processed(transactions_to_execute.size());
  // Now we have the events to be processed and the time when they are ready to be processed.

  while(!process_events.empty()){
    auto cur_event = process_events.top();
    process_events.pop();
    int cur_txn_id = cur_event.second.txn_id;
    if(transactions_to_execute[cur_txn_id].status==1){
        // This transaction is cancelled. Do not process any further steps.
        continue;
    }
    path_var& cur_path_var = cur_event.second;
    node* cur_node = cur_network->nodes[cur_path_var.node_id];
    edge* outgoing_edge = cur_path_var.outgoing_edge;
    if(cur_event.first == 2){
        // Reverted txn
        outgoing_edge->balance += cur_path_var.amount_passed;
    }
    else{
        if(cur_network->nodes.find(cur_path_var.node_id) ==cur_network->nodes.end()){
            cout<<"Node not found\n";return;
        }

        // If this is the destination,  update the balances and give the fees to intermediate nodes.
        if(!outgoing_edge){
            // This transaction is complete.
            for(int i=0;i<hops_processed[cur_txn_id].size();i++){
                path_var temp_int_node = hops_processed[cur_txn_id][i];
                if(cur_network->edges.find(temp_int_node.outgoing_edge->counter_edge_id) == cur_network->edges.end() ){
                    cout<<"Edge mapping not found, line 104. \n";
                    return;
                }
                edge* incoming_edge = cur_network->edges[temp_int_node.outgoing_edge->counter_edge_id];
                incoming_edge->balance += temp_int_node.amount_passed;
                if(cur_network->nodes.find(temp_int_node.node_id) == cur_network->nodes.end() ){
                    cout<<"Node mapping not found, line 110 \n";
                    return;
                }
                node* temp_node = cur_network->nodes[temp_int_node.node_id];
                temp_node->fee_collected += temp_int_node.fee_taken;
            }
            transactions_to_execute[cur_txn_id].status = 2;
            continue;
        }

        int random_number = distribution(generator);
        if(random_number <= faulty_prob){
            //cout<<"Inactive node............\n";
        }
        if(outgoing_edge->balance < cur_path_var.amount_passed){
            // Inactive node or Not enough balance.
            // Cancel txn.
            transactions_to_execute[cur_txn_id].status = 1;
            // revert txn.
            ll time_var = cur_event.second.time_taken;
            for(int i=0;i<hops_processed[cur_txn_id].size();i++){
                time_var += get_hop_processing_time(10,20);
                process_events.push({2, hops_processed[cur_txn_id][i]});
            }
            continue;
        }
        
        outgoing_edge->balance -=cur_path_var.amount_passed;
        hops_processed[cur_txn_id].push_back(cur_path_var);
    }
  }

  // Calculate network statistics.
  int txn_completed=0;
  double average_path_length=0;
  long long int average_fee=0;
  for(int ind=0;ind<transactions_to_execute.size();ind++){
    if(transactions_to_execute[ind].status==2){
        txn_completed++;
        average_path_length+= transactions_to_execute[ind].path_length;
        average_fee += transactions_to_execute[ind].fee;
        cout<<"Txn fee: "<<transactions_to_execute[ind].fee<<"\n";
    }
  }

  // Print transaction statistics
  if(txn_completed>0){
    average_path_length/= (txn_completed);
    average_fee/= (txn_completed);
    cout<<"Number of transactions completed: "<<txn_completed<<" out of "<<transactions_to_execute.size()<<"\n";
    cout<<"Average path length: "<<average_path_length<<"\n";
    cout<<"Average fee : "<<average_fee<<"\n";
  }else{
    cout<<"No transaction completed.\n";
  }

}



vector<transaction> get_random_transactions(const network_params& net_params){
    vector<transaction> res;

    for(int i=0;i<net_params.num_of_txn;i++){
        transaction new_txn;
        new_txn.sender_id = rand()%(net_params.n_nodes) + 1;
        new_txn.receiver_id = rand()%(net_params.n_nodes) + 1;
        new_txn.amount = net_params.txn_amount_lower_limit;
        if(net_params.txn_amount_upper_limit - net_params.txn_amount_lower_limit > 0){
            new_txn.amount = net_params.txn_amount_lower_limit+ rand()%(net_params.txn_amount_upper_limit - net_params.txn_amount_lower_limit);
        }
        new_txn.fee=0;
        new_txn.status=0;
        new_txn.txn_number = i;

        // Corner case if sender id and receiver id are same.
        // Increase the sender id if so unless it is the maximum id limit.
        if(new_txn.sender_id == new_txn.receiver_id){
            if(new_txn.sender_id == net_params.n_nodes)new_txn.sender_id=1;
            else new_txn.sender_id+=1;
        }

        res.push_back(new_txn);
    }
    return res;
}

// Status of Txn:
// 0 if not started
// 1 if cancelled
// 2 if complete
bool payment_request(transaction& req, network* cur_network){
    // Check if the receiver exists in the network.
    if(cur_network->nodes.find(req.receiver_id) == cur_network->nodes.end()){
        req.status = 1;
        return false;
    }
    // Check if the sender exists in the network.
    if(cur_network->nodes.find(req.sender_id) == cur_network->nodes.end()){
        req.status = 1;
        return false;
    }

    // Check if the capacities of the sender is greater than the amount.
    const auto& sender_node = cur_network->nodes[req.sender_id];
    const auto& sender_edges = sender_node->open_edges;
    int edge_exists = 0;
    for(const auto& edge: sender_edges){
        if(edge->balance >= req.amount){
            edge_exists=1;break;
        }
    }
    if(edge_exists)return true;
    else{
        req.status=1;
        return false;
    }

    return true;
}

//The i-th intermediate node will charge an imbalance fee
// of f im
// i = IP(δi,i+1 + 2α) − IP(δi,i+1), where δi,j + 2α is
// the imbalance degree after payment transfer.
// δi,j = |bi,j −bj,i|
long long int imbalance_penalty(int imbalance_fee_const, long long int balance_diff){
    return imbalance_fee_const*(balance_diff*balance_diff);
}
// Calculates fee taken by a node
double calc_fee(node* cur_node, long long int amount, int balance_diff,int num_of_txn){
    double fee=0.0;
    fee+= cur_node->node_fee->base_fee;
    double th=1000.0;
    fee+= double((double(cur_node->node_fee->proportional_fee_const*amount))/th);
    // Add imbalance fee.
    fee+= double(abs(imbalance_penalty(cur_node->node_fee->imbalance_fee_const, balance_diff + 2*amount) 
    - imbalance_penalty(cur_node->node_fee->imbalance_fee_const, balance_diff)))/1e5;
    // Add time value fee.
    double one=1.0;
    double num_tx= 100;
    double amt=amount;
    // // fee+= ((one/num_tx)*amt);
    // cout<<"B: "<<cur_node->node_fee->base_fee<<" P: "<<double((double(cur_node->node_fee->proportional_fee_const*amount))/th)
    // <<" Im: "<<double(abs(imbalance_penalty(cur_node->node_fee->imbalance_fee_const, balance_diff + 2*amount) 
    // - imbalance_penalty(cur_node->node_fee->imbalance_fee_const, balance_diff)))/1e5<<" Prp: "<<((one/num_tx)*amt)<<"\n";
    // cout<<"Fee bf : "<<fee<<"\n";
    fee=ceil(fee);
    // cout<<"Fee af"<<fee<<"\n";
    return fee;
}

// Returns random time valuee between given bounds.
int get_hop_processing_time(int lower_limit, int upper_limit){
    return (rand()%(upper_limit - lower_limit) + lower_limit);
}

// Returns path calculation time (random values)
// Values vary between 100-200
int get_path_calculation_time(){
    return rand()%100 + 100;
}

// Uses dijkstra algorithm which is centralised.
vector<path_var> get_payment_path_using_Dijkstra(transaction& req, network* cur_network,const network_params& net_params){
    int start_node = req.receiver_id; // Vs
    int dest_node = req.sender_id; // Vr
    int total_nodes = net_params.n_nodes;
    vector<ll> dis(total_nodes+1,1e18);
    vector<int> hops(total_nodes+1,0);
    vector<pair<int,edge*>> prev_node(total_nodes+1,{-1,NULL});
    vector<bool> vis(total_nodes+1,false);
    priority_queue<pair<ll,int>,vector<pair<ll,int>>, greater<pair<ll,int>>> q;
    q.push({req.amount,start_node});
    dis[start_node]=req.amount;
    // `q` stores {distance, node}

    while(!q.empty()){
        auto cur_node = q.top();q.pop();
        if(vis[cur_node.second])continue;
        vis[cur_node.second]=true;
        // printing details
        // cout<<"-------See the cur node "<<cur_node.second<<" with dis "<<cur_node.first<<"\n";
        const auto itr = (cur_network->nodes).find(cur_node.second);
        if(itr == (cur_network->nodes).end()){
            cout<<"In `get_payment_path()` function, mapping of "<<
            cur_node.second<<" node doesn't exist\n";
            break;
        }
        ll amount_to_be_transferred = cur_node.first;
        // We are traversing the graph in reverse order, i.e.
        // we are going to source from destinantion.
        // cout<<"Number of edges "<<((cur_network->nodes[cur_node.second])->open_edges.size())<<"\n";
        for(auto& cur_edge: (cur_network->nodes[cur_node.second])->open_edges){
            // This edge is the counter edge, we need to get the balance of 
            // the counter edge.
            
            if(cur_network->edges.find(cur_edge->counter_edge_id)==cur_network->edges.end()){
                cout<<"Edge id mapping not found in `get_payment_path()` function, edge id: "
                << cur_edge->counter_edge_id<<"\n";
            }
            edge* counter_edge = cur_network->edges[cur_edge->counter_edge_id];
            // cout<<"Counter edge "<<counter_edge->id<<" and the node "<<cur_edge->to_node_id<<"\n";
            // If the balance isn't sufficient, we cannot go through this edge.
            if(counter_edge->balance < amount_to_be_transferred){
                // cout<<"Lack of balance\n";
                continue;
            }

            // cout<<"The node: "<<cur_edge->to_node_id<<" has capacity\n";
            ll balance_diff = abs(cur_edge->balance - counter_edge->balance);
            ll fee_taken = calc_fee(cur_network->nodes[cur_edge->to_node_id],req.amount, balance_diff, net_params.num_of_txn);
            // cout<<"Fee taken is "<<fee_taken<<"\n";
            if(dis[cur_edge->to_node_id] > dis[cur_node.second] + fee_taken){
                dis[cur_edge->to_node_id] = dis[cur_node.second] + fee_taken;
                prev_node[cur_edge->to_node_id] = {cur_node.second, counter_edge};
                hops[cur_edge->to_node_id]++;
                if(!vis[cur_edge->to_node_id])q.push({dis[cur_edge->to_node_id],cur_edge->to_node_id});
            }
        }
    }

    if(dis[dest_node]>1e17){
        // We cannot reach via any possible path. i.e. no path can fullfil the required capacity.
        req.status = 1; // req can't be completed due to limited channel capacity.
        vector<path_var> empty_path_vec;
        return empty_path_vec;
        // Empty vector indicates that no path exists
    }
    
    req.fee = dis[dest_node] -req.amount;
    // Restore path.
    vector<path_var> path;
    int time_taken = get_path_calculation_time();
    for(int v = dest_node; v!=-1 && v!= start_node; v = prev_node[v].first){
        path_var intermediate_node;
        intermediate_node.node_id = v;
        intermediate_node.time_taken = time_taken;
        intermediate_node.outgoing_edge = prev_node[v].second;
        intermediate_node.fee_taken=0;
        intermediate_node.amount_passed = 0;
        path.push_back(intermediate_node);
        // Note that the lower bound and upper bound of time
        // for hop processing is arbitrarily chosen. ( 10 and 20)
        time_taken += net_params.hop_processing_time;
    }
    path_var receiver_node;
    receiver_node.node_id = start_node;
    receiver_node.time_taken = time_taken;
    receiver_node.outgoing_edge = NULL;
    receiver_node.fee_taken=0;
    path.push_back(receiver_node);

    for(int i=1;i<path.size()-1;i++){
        path[i].fee_taken  = dis[path[i].node_id] - dis[path[i+1].node_id];
        path[i].amount_passed = dis[path[i+1].node_id];
    }
    if(!path.empty())path[0].amount_passed = req.amount;
    req.path_length = path.size();

    return path;

}

// Uses Bellman ford algorithm, which is distributed.
vector<path_var> get_payment_path_using_bellman_ford(transaction& req, network* cur_network,const network_params& net_params){
    if(cur_network->nodes.find(req.sender_id) == cur_network->nodes.end() || cur_network->nodes.find(req.receiver_id) == cur_network->nodes.end()){
        cout<<"Wrong transaction values....\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
        vector<path_var> empty_path_vec;
        return empty_path_vec;
    }
    // cout<<"Called bellman ford algo\n";
    int n = cur_network->nodes.size() +1;
    vector<long long int> dis (n, 1e17);
    dis[req.receiver_id] = req.amount;
    vector<pair<int,edge*>> p (n, {-1,NULL});
    int total_edges = cur_network->edges.size();
    // cout<<" Total nodes: "<<n<<" total edges: "<<total_edges<<"\n";

    for (int ind=0;ind<n-1;ind++)
    {
        bool any = false;
        for (int i = 0; i < total_edges; ++i){
            if(cur_network->edges.find(i)==cur_network->edges.end()){
                cout<<"Edge id mapping not found in `get_payment_path_bellman_ford()` function, edge id: "
                << i<<"\n";
                continue;
            }
            auto cur_edge = cur_network->edges[i];

            if(cur_network->edges.find(cur_edge->counter_edge_id) == cur_network->edges.end()){
                cout<<"Edge id mapping not found in `get_payment_path_bellman_ford()` function, edge id: "
                << cur_edge->counter_edge_id<<"\n";
            }
            
            auto counter_edge = cur_network->edges[cur_edge->counter_edge_id];
            if (dis[cur_edge->from_node_id] < 1e17 && counter_edge->balance >= dis[cur_edge->from_node_id]){
                ll balance_diff = abs(cur_edge->balance - counter_edge->balance);
                ll fee_taken = calc_fee(cur_network->nodes[cur_edge->to_node_id],dis[cur_edge->from_node_id], balance_diff, net_params.num_of_txn);
                if (dis[cur_edge->to_node_id] > dis[cur_edge->from_node_id] + fee_taken)
                {
                    dis[cur_edge->to_node_id] = dis[cur_edge->from_node_id] + fee_taken;
                    p[cur_edge->to_node_id] = {cur_edge->from_node_id, counter_edge};
                    any = true;
                }
            }
        }
        if (!any)  break;
    }

    if (dis[req.sender_id] >=1e17){
        // We cannot reach via any possible path. i.e. no path can fullfil the required capacity.
        req.status = 1; // req can't be completed due to limited channel capacity.
        vector<path_var> empty_path_vec;
        return empty_path_vec;
        // Empty vector indicates that no path exists
    }
    vector<path_var> path;
    int time_taken = cur_network->nodes[req.sender_id]->path_calculation_time;
    for(int v = req.sender_id; v!=-1 && v!= req.receiver_id; v = p[v].first){
        path_var intermediate_node;
        intermediate_node.txn_id = req.txn_number;
        intermediate_node.node_id = v;
        intermediate_node.time_taken = time_taken;
        intermediate_node.outgoing_edge = p[v].second;
        intermediate_node.fee_taken=0;
        intermediate_node.amount_passed = 0;
        path.push_back(intermediate_node);
        time_taken += net_params.hop_processing_time;
    }
    path_var receiver_node;
    receiver_node.txn_id = req.txn_number;
    receiver_node.node_id = req.receiver_id;
    receiver_node.time_taken = time_taken;
    receiver_node.outgoing_edge = NULL;
    receiver_node.fee_taken=0;
    path.push_back(receiver_node);

    for(int i=1;i<path.size()-1;i++){
        path[i].fee_taken  = dis[path[i].node_id] - dis[path[i+1].node_id];
        path[i].amount_passed = dis[path[i+1].node_id];
    }
    if(!path.empty())path[0].amount_passed = req.amount;
    req.path_length = path.size();
    req.fee = dis[req.sender_id] - dis[req.receiver_id];

    return path;
}

// Uses DFS algorithm.
vector<path_var> get_payment_path_using_DFS(transaction& req, network* cur_network,const network_params& net_params){
    if(cur_network->nodes.find(req.sender_id) == cur_network->nodes.end() || cur_network->nodes.find(req.receiver_id) == cur_network->nodes.end()){
        cout<<"Wrong transaction values....\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
        vector<path_var> empty_path_vec;
        return empty_path_vec;
    }
    int n = cur_network->nodes.size() +1;
    vector<vector<pair<int,edge*>>> adj(n+1);
    for(auto x: cur_network->edges){
        edge* cur_edge = x.second;
        adj[cur_edge->from_node_id].push_back({cur_edge->to_node_id,cur_edge});
    }
    vector<long long int> dis (n+1, 1e17);
    vector<bool> vis(n+1,false);
    dis[req.receiver_id] = req.amount;
    vis[req.receiver_id] = true;
    vector<pair<int,edge*>> p (n+1, {-1,NULL});
    int total_edges = cur_network->edges.size();
    // cout<<" Total nodes: "<<n<<" total edges: "<<total_edges<<"\n";

    stack<int> s;
    s.push(req.receiver_id);

    while(!s.empty()){
        int cur_node_id = s.top();
        s.pop();
        if(!vis[cur_node_id]){
            vis[cur_node_id]=true;
        }
        for(auto to_node_var: adj[cur_node_id]){
            int to_node_id = to_node_var.first;
            auto cur_edge = to_node_var.second;
            auto counter_edge = cur_network->edges[cur_edge->counter_edge_id];
            if (dis[cur_edge->from_node_id] < 1e17 && counter_edge->balance >= dis[cur_edge->from_node_id]){
                ll balance_diff = abs(cur_edge->balance - counter_edge->balance);
                ll fee_taken = calc_fee(cur_network->nodes[cur_edge->to_node_id],dis[cur_edge->from_node_id], balance_diff,net_params.num_of_txn);
                if (dis[cur_edge->to_node_id] > dis[cur_edge->from_node_id] + fee_taken)
                {
                    dis[cur_edge->to_node_id] = dis[cur_edge->from_node_id] + fee_taken;
                    p[cur_edge->to_node_id] = {cur_edge->from_node_id, counter_edge};
                }
                if(!vis[to_node_id]){
                    s.push(to_node_id);
                }
            }
        }
    }

    if(dis[req.sender_id] >=1e17){
        req.status = 1; // req can't be completed due to limited channel capacity.
        vector<path_var> empty_path_vec;
        return empty_path_vec;
        // Empty vector indicates that no path exists
    }


    vector<path_var> path;
    int time_taken = cur_network->nodes[req.sender_id]->path_calculation_time;
    for(int v = req.sender_id; v!=-1 && v!= req.receiver_id; v = p[v].first){
        path_var intermediate_node;
        intermediate_node.txn_id = req.txn_number;
        intermediate_node.node_id = v;
        intermediate_node.time_taken = time_taken;
        intermediate_node.outgoing_edge = p[v].second;
        intermediate_node.fee_taken=0;
        intermediate_node.amount_passed = 0;
        path.push_back(intermediate_node);
        time_taken += net_params.hop_processing_time;
    }
    path_var receiver_node;
    receiver_node.txn_id = req.txn_number;
    receiver_node.node_id = req.receiver_id;
    receiver_node.time_taken = time_taken;
    receiver_node.outgoing_edge = NULL;
    receiver_node.fee_taken=0;
    path.push_back(receiver_node);

    for(int i=1;i<path.size()-1;i++){
        path[i].fee_taken  = dis[path[i].node_id] - dis[path[i+1].node_id];
        path[i].amount_passed = dis[path[i+1].node_id];
    }
    if(!path.empty())path[0].amount_passed = req.amount;
    req.path_length = path.size();
    req.fee = dis[req.sender_id] - dis[req.receiver_id];
    // cout<<"Fee for txn with amt: "<<req.amount<<" is " <<req.fee<<" with path len "<<req.path_length<<"\n";

    return path;
}
