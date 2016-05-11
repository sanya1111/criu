#! /bin/bash

#chmod +x ./test.sh

DIR="$1"

#setsid nohup ./test.sh &
setsid nohup gen_tree/gen_tree &
PID=$!

sudo ../criu dump -t $PID -D "$DIR" && echo OK
../main.py "$DIR"
