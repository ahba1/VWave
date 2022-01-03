#!/bin/bash
create_and_enter_build_folder() {
  if [ -d "build/" ];then
  echo 'build already exist, delete..'
  rm -rf build
  fi
  echo 'create folder'
  mkdir build
  cd build
}

build_phase_cmake() {
  echo 'begin to build project'
  echo 'cmake begin'
  cmake ..
  if [$? -ne 0];then
    echo 'cmake failed'
    exit 1
  fi
}

build_phase_make() {
  echo 'make begin'
  make
  if [$? -ne 0];then
    echo 'make failed'
    exit $?
  fi
}

main() {
  create_and_enter_build_folder
  build_phase_cmake
  build_phase_make
  echo 'build successfully'
}
main
