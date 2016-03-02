#!/bin/bash
./run_criu.sh clean
cgexec -g cpu:mygroup ./run_criu.sh init &&
 ./run_criu.sh dump && 
 cgexec -g cpu:mygroup ./run_criu.sh restore

