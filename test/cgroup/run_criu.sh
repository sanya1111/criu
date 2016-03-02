#!/bin/bash
criu=../../criu/criu
workers=10
pid_log=pid_log
img_dir=img
export cgroup=cpu/mygroup

function init(){
	mkdir -p $img_dir && echo "creating img dir\n"
	./run_shadow_worker.sh $workers && echo "running shadow workers\n"
	#cgexec instead of it
	# ./run_shadow_worker.sh cgroup && echo "adding workers to cgroup\n"
}

function dump(){
	$criu dump  --images-dir=$img_dir -vvvv --dump-cgroup=/sys/fs/cgroup/$cgroup
}

function restore(){
	$criu restore -d --images-dir=$img_dir -vvv
}

case $1 in
	"init" )
	init
	;;
	"dump" )
	dump
	;;
	"restore" )
	restore
	;;
	"clean" )
	./run_shadow_worker.sh clean
	rm -rf $img_dir
	;;
esac