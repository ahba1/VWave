#!/bin/bash

main() {
    ./../build.sh
    java -agentpath:../build/libVWave.so TestedDemo
}
main