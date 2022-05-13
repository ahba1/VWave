#!/bin/bash

if test $1 = "-l"
then
	echo "MethodService";
	echo "ThreadService";
else
	if test $1 = "-test"
	then
		cd ..;
		./build.sh;
		cd sample;
		echo "begin to test";
		java -agentpath:./../build/libVWave.so=test $2;
	else
		cd ..;
		cmd = "-skip";
		if [$cmd != $4]
		then 
			echo "begin to build";
			./build.sh;
		fi
		cd sample;
		rm -f *.txt;
		rm -f *.log;
		echo "Attach agent to $3";
		java -agentpath:./../build/libVWave.so=service=$1-task=$2 $3;
	fi
fi
