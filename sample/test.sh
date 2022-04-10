#!/bin/bash

main() {
    cd ..
    ./build.sh
    cd sample
    java -agentpath:./../build/libVWave.so=test TestedDemo
}
main