# Payment Channel Network Simulator for Bitcoin

This is a simulator for payment channel network in Bitcoin.
Payment channel networks are a type of off-chain scaling solution for blockchains, allowing for fast and cheap transactions without having to rely on the blockchain for every transaction. Payment channels are created between two parties, enabling them to transact multiple times without broadcasting every transaction to the blockchain. Payment channel networks are formed when multiple payment channels are connected to each other, creating a network of channels that can be used to route payments between any two parties in the network.


This contains the code of Lightning Network simulator, which simulates the network topology very similar to real life network.
It can also execute transactions as happens in PCN, Dijkstra, DFS (centralized approaches) and Bellman-Ford algorithms are implemented to find the path for each transaction.
The simulator insurses that there is a single connected graph component. If multiple componenets exist, it joins the high degree nodes of those components so that every node has a path from any other node.  
For more details, refer https://bitcoin.org/bitcoin.pdf
The PCN topology statistics can be found on https://bitcoinvisuals.com

## Requirements
This simulator requires the following software to be installed:

- c++17
- gsl library

To execute a set of transactions between nodes, run the following command:
1. g++ -std=c++17 simulator.cpp network.cpp transactions.cpp
2. ./a.out

## Input
Input is provided from input.txt file
n_nodes, degree_average,
degree_90th_percentile,
degree_50th_percentile,
degree_10th_percentile,
capacity_average,
capacity_90th_percentile,
capacity_50th_percentile,
capacity_10th_percentile,
base_fee_lower,
base_fee_upper,
proportional_fee_const_lower,
proportional_fee_const_upper,
imbalance_fee_const_lower,
imbalance_fee_const_upper,
num_of_txn_sets,
num_of_txn,
txn_amount_lower_limit,
txn_amount_upper_limit,
faulty_node_probability

## Features
The Payment Channel Networks Simulator includes the following features:

1. Simulation of payment channel networks with multiple participants
2. The ability to specify network topology and transaction volumes
3. Performance metrics for the network, such as transaction throughput and latency
4. Both centralized and distributed algorithms for path calculation.


## Example
For input:
n_nodes=5000
degree_average=10
degree_90th_percentile=15
degree_50th_percentile=8
degree_10th_percentile=2
capacity_average=5000
capacity_90th_percentile=10000
capacity_50th_percentile=7000
capacity_10th_percentile=2000
base_fee_lower=2
base_fee_upper=5
proportional_fee_const_lower=20
proportional_fee_const_upper=100
imbalance_fee_const_lower=20
imbalance_fee_const_upper=100
num_of_txn_sets=3
num_of_txn=500
txn_amount_lower_limit=1
txn_amount_upper_limit=200
faulty_node_probability=0.005

The output will be:
NETWORK INITIALIZATION

Transaction set: 1
Number of transactions completed: 486 out of 500
Average path length: 4.54321
Average fee : 1385

Transaction set: 2
Number of transactions completed: 485 out of 500
Average path length: 4.56701
Average fee : 1352

Transaction set: 3
Number of transactions completed: 483 out of 500
Average path length: 4.55694
Average fee : 1578


We can see statistics for individual transaction:
Txn fee: 195
Txn fee: 120
Txn fee: 69
Txn fee: 181
Txn fee: 923
Number of transactions completed: 5 out of 5
Average path length: 3.8
Average fee : 297
