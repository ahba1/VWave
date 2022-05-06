#!/bin/bash

main() {
    cd ..
    ./build.sh
    cd sample
    #java -agentpath:./../build/libVWave.so=test TestedDemo
    rm -f *.txt
    rm -f *.log
    java -agentpath:./../build/libVWave.so=service=method-record=./ TestedDemo
}
main