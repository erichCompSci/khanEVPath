#!/bin/bash

SESS_NAME="my_session"

tmux new-session -d -s $SESS_NAME "./bin/dfg_master src/mytest2.cfg"

#Need this to ensure the clients get the right master string, sloppy but not worth the effort to fix
sleep 2 

tmux set-window-option -g remain-on-exit on
tmux set-window-option -u -t 0 remain-on-exit

tmux split-window -t $SESS_NAME:0.0 -v "./bin/dfg_general_client src/mytest2.cfg a"
tmux split-window -t $SESS_NAME:0.0 -h "./bin/dfg_general_client src/mytest2.cfg b"
tmux split-window -t $SESS_NAME:0.1 -h "./bin/dfg_general_client src/mytest2.cfg c"

tmux neww -d "./bin/dfg_general_client src/mytest2.cfg d"
tmux split-window -t $SESS_NAME:1.0 -v "./bin/dfg_general_client src/mytest2.cfg e"
tmux split-window -t $SESS_NAME:1.0 -h "./bin/dfg_general_client src/mytest2.cfg f"
tmux split-window -t $SESS_NAME:1.1 -h "./bin/dfg_general_client src/mytest2.cfg g"

tmux neww -d "./bin/dfg_general_client src/mytest2.cfg h"
tmux split-window -t $SESS_NAME:2.0 -v

tmux attach -t $SESS_NAME

