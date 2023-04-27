# Payment Channel Network Simulator for Bitcoin

This is a simulator for payment channel network in Bitcoin.
Payment channel networks are a type of off-chain scaling solution for blockchains, allowing for fast and cheap transactions without having to rely on the blockchain for every transaction. Payment channels are created between two parties, enabling them to transact multiple times without broadcasting every transaction to the blockchain. Payment channel networks are formed when multiple payment channels are connected to each other, creating a network of channels that can be used to route payments between any two parties in the network.

## Requirements

This simulator requires the following software to be installed:

- c++17
- gsl library

## Example

To execute a set of transactions between nodes, run the following command:
1. g++ -std=c++17 simulator.cpp network.cpp transactions.cpp
2. ./a.out

## Input
Input is provided from input.txt file
n_nodes=200 (number of nodes in the graph)
degree_average=7 (average node degree)
degree_90th_percentile=10
degree_50th_percentile=5
degree_10th_percentile=1
capacity_average=5000 (average edge capacity)
capacity_90th_percentile=10000
capacity_50th_percentile=7000
capacity_10th_percentile=2000
base_fee_lower=2 (base fee lower bound)
base_fee_upper=5 (base fee upeer bound)
proportional_fee_const_lower=20
proportional_fee_const_upper=100
imbalance_fee_const_lower=20
imbalance_fee_const_upper=100
num_of_txn_sets=10
num_of_txn=5000 (number of transaction to be generated in each set)
txn_amount_lower_limit=1
txn_amount_upper_limit=100
faulty_node_probability=0.001

## Features
The Payment Channel Networks Simulator includes the following features:

1. Simulation of payment channel networks with multiple participants
2. The ability to specify network topology and transaction volumes
3. Performance metrics for the network, such as transaction throughput and latency
4. Both centralized and distributed algorithms for path calculation.
