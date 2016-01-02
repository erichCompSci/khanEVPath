#!/bin/bash

SESS_NAME="my_session"
OUTPUT_DIR=outputs/

tmux new-session -d -s $SESS_NAME "./bin/dfg_master src/mytest2.cfg 1>$OUTPUT_DIR/dfg_master_output.txt 2>&1"

sleep 2

tmux set-window-option -g remain-on-exit on
tmux set-window-option -u -t 0 remain-on-exit

tmux split-window -t $SESS_NAME:0.0 -v "./bin/dfg_general_client src/mytest2.cfg a 1>$OUTPUT_DIR/dfg_gen_client_a.txt 2>&1"
tmux split-window -t $SESS_NAME:0.0 -h "./bin/dfg_general_client src/mytest2.cfg b 1>$OUTPUT_DIR/dfg_gen_client_b.txt 2>&1"
tmux split-window -t $SESS_NAME:0.1 -h "./bin/dfg_general_client src/mytest2.cfg c 1>$OUTPUT_DIR/dfg_gen_client_c.txt 2>&1"

tmux neww -d "./bin/dfg_general_client src/mytest2.cfg d 1>$OUTPUT_DIR/dfg_gen_client_d.txt 2>&1"
tmux split-window -t $SESS_NAME:1.0 -v "./bin/dfg_general_client src/mytest2.cfg e 1>$OUTPUT_DIR/dfg_gen_client_e.txt 2>&1"
tmux split-window -t $SESS_NAME:1.0 -h "./bin/dfg_general_client src/mytest2.cfg f 1>$OUTPUT_DIR/dfg_gen_client_f.txt 2>&1"
tmux split-window -t $SESS_NAME:1.1 -h "./bin/dfg_general_client src/mytest2.cfg g 1>$OUTPUT_DIR/dfg_gen_client_g.txt 2>&1"

tmux neww -d "./bin/dfg_general_client src/mytest2.cfg h 1>$OUTPUT_DIR/dfg_gen_client_h.txt 2>&1"
tmux split-window -t $SESS_NAME:2.0 -v

tmux attach -t $SESS_NAME

