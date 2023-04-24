#!/bin/bash


for i in {111..120}; do

    ./a.out
    
    cp transaction_store.txt final_train/transaction_${i}.txt
    cp graph_props.txt final_train/graph_data_${i}.txt

done