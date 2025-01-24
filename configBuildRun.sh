#! /bin/bash

if [ $# = 2 ]; then
  # the second argument is the file to processing
  var="$2"
else
  # if only get one command-line argument, code.txt is default
  var="code.txt"
fi

if [ $1 = "1" ]; then
  # config, build, run
  cmake --fresh --preset default && cmake --build build && ./main2 $var
elif [ $1 = "1o" ]; then
  # config, build, run
  cmake --fresh --preset default -DOp=ON && cmake --build build && ./main2 $var
elif [ $1 = "2" ]; then
  # build, run
  cmake --build build && ./main2 $var
elif [ $1 = "3" ]; then
  # run
  ./main2 $var
fi

# visualize the cfg of llvm IR
if [ $1 = "viscfg" ]; then
  opt -passes=dot-cfg t.ll && dot .baz.dot -Tpng -o t.png && feh t.png
fi

if [ $1 = "4" ]; then
  # config, build, run
  cmake --fresh --preset default && cmake --build build --target=testl && ./build/testl
fi
