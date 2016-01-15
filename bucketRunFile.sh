#!/bin/bash

SESS_NAME="my_session"
OUTPUT_DIR=outputs/
CONFIG_FILE=src/bucketTest.cfg

tmux new-session -d -s $SESS_NAME "./bin/dfg_master $CONFIG_FILE 1>$OUTPUT_DIR/dfg_master_output.txt 2>&1"

sleep 2

tmux set-window-option -g remain-on-exit on
tmux set-window-option -u -t 0 remain-on-exit

tmux split-window -t $SESS_NAME:0.0 -v "./bin/dfg_general_client $CONFIG_FILE a 1>$OUTPUT_DIR/dfg_gen_client_a.txt 2>&1"
tmux split-window -t $SESS_NAME:0.0 -h "./bin/dfg_general_client $CONFIG_FILE b 1>$OUTPUT_DIR/dfg_gen_client_b.txt 2>&1"
tmux split-window -t $SESS_NAME:0.1 -h "./bin/dfg_general_client $CONFIG_FILE c 1>$OUTPUT_DIR/dfg_gen_client_c.txt 2>&1"

tmux neww -d "./bin/dfg_general_client $CONFIG_FILE d 1>$OUTPUT_DIR/dfg_gen_client_d.txt 2>&1"
tmux split-window -t $SESS_NAME:1.0 -v "./bin/dfg_general_client $CONFIG_FILE e 1>$OUTPUT_DIR/dfg_gen_client_e.txt 2>&1"
tmux split-window -t $SESS_NAME:1.0 -h "./bin/dfg_general_client $CONFIG_FILE f 1>$OUTPUT_DIR/dfg_gen_client_f.txt 2>&1"
tmux split-window -t $SESS_NAME:1.1 -h "./bin/dfg_general_client $CONFIG_FILE g 1>$OUTPUT_DIR/dfg_gen_client_g.txt 2>&1"

tmux neww -d "./bin/dfg_general_client $CONFIG_FILE h 1>$OUTPUT_DIR/dfg_gen_client_h.txt 2>&1"
tmux split-window -t $SESS_NAME:2.0 -v "./bin/dfg_general_client $CONFIG_FILE new 1>$OUTPUT_DIR/dfg_gen_client_new.txt 2>&1" 
tmux split-window -t $SESS_NAME:2.0 -h "./bin/dfg_general_client $CONFIG_FILE j 1>$OUTPUT_DIR/dfg_gen_client_j.txt 2>&1"
tmux split-window -t $SESS_NAME:2.1 -h

tmux attach -t $SESS_NAME

