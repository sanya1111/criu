#!/bin/bash
log=log
pid_log=pid_log

function run_workers {
	for i in `seq 1 $1`; do
		setsid ./shadow_worker.sh $i < /dev/null &> $log$i &
	done

	pids=$(ps -C shadow_worker.sh -o pid | grep -Eo "[0-9]*")
	echo $pids >> $pid_log 
}

function kill_workers {
	for i in `cat $pid_log`; do
		 kill -9 $i; 
	done
}

function add_workers_to_cgroup {
	for i in `cat $pid_log`; do
		 echo $i >> /sys/fs/cgroup/$cgroup/tasks
	done
}

function kill_cgroup {
	for i in `cat /sys/fs/cgroup/$cgroup/tasks`; do
		kill -9 $i;
	done
}

function clean {
	kill_workers
	kill_cgroup
	rm -f $log* $pid_log
}

case $1 in 
	"kill" )
	kill_workers 
	;;
	"clean" )
	clean
	;;
	"cgroup" )
	add_workers_to_cgroup
	;;
	* ) 
	run_workers $1 
	;;
esac