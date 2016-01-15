#!/bin/bash

SESS_NAME="my_session"
CONFIG_FILE=src/bucketTest.cfg

tmux new-session -d -s $SESS_NAME "./bin/dfg_master $CONFIG_FILE"

#Need this to ensure the clients get the right master string, sloppy but not worth the effort to fix
sleep 2 

tmux set-window-option -g remain-on-exit on
tmux set-window-option -u -t 0 remain-on-exit

tmux split-window -t $SESS_NAME:0.0 -v "./bin/dfg_general_client $CONFIG_FILE a"
tmux split-window -t $SESS_NAME:0.0 -h "./bin/dfg_general_client $CONFIG_FILE b"
tmux split-window -t $SESS_NAME:0.1 -h "./bin/dfg_general_client $CONFIG_FILE c"

tmux neww -d "./bin/dfg_general_client $CONFIG_FILE d"
tmux split-window -t $SESS_NAME:1.0 -v "./bin/dfg_general_client $CONFIG_FILE e"
tmux split-window -t $SESS_NAME:1.0 -h "./bin/dfg_general_client $CONFIG_FILE f"
tmux split-window -t $SESS_NAME:1.1 -h "./bin/dfg_general_client $CONFIG_FILE g"

tmux neww -d "./bin/dfg_general_client $CONFIG_FILE h"
tmux split-window -t $SESS_NAME:2.0 -v "./bin/dfg_general_client $CONFIG_FILE new"
tmux split-window -t $SESS_NAME:2.0 -h "./bin/dfg_general_client $CONFIG_FILE j"
tmux split-window -t $SESS_NAME:2.1 -h

tmux attach -t $SESS_NAME

