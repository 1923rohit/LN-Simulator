Command to execute the code:
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
#
# Avg txn fee: 5800 satoshi
# Amout sent in each transaction data is not available,
# also the base fee constants values are not available
#
#
#
number of node- x-axis, y-axis -> number of nodes , throughput