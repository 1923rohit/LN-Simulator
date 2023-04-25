This contains the code of Lightning Network simulator, which simulates the network topology very similar to real life network.
It can also execute transactions as happens in PCN, Dijkstra, DFS (centralized approaches) and Bellman-Ford algorithms are implemented to find the path for each transaction.

The PCN topology statistics can be found on bitcoinvisuals.com


Commands to execute the code:
1. g++ -std=c++17 simulator.cpp network.cpp transactions.cpp
2. ./a.out
# LN-simulator-ML-implementation

# Environment init
source LNenv/bin/activate

# Real network statistics as per bitcoinvisuals.com
# Nodes: 18,000
# Channels: ~76k
# Cap:
#   Avg.: 47,00,000
#   90th: 10,000,000
#   50th: 1,000,000
#   10th: 90,000
#
# Channels per node:
#   Avg. : 8.8
#   90th : 15
#   50th : 2
#   10th : 1
