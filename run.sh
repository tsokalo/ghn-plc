#!/bin/bash

./waf build

LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/tsokalo/workspace/coin-Clp/lib/:/home/tsokalo/workspace/SimpleNetSim/build/utils/utils/:/home/tsokalo/workspace/SimpleNetSim/build/galois-field/galois-field/:/home/tsokalo/workspace/SimpleNetSim/build/ccack/ccack/:/home/tsokalo/workspace/SimpleNetSim/build/lp-solver/lp-solver/:/home/tsokalo/workspace/SimpleNetSim/build/network/network/:/home/tsokalo/workspace/SimpleNetSim/build/traffic/traffic/:/home/tsokalo/workspace/SimpleNetSim/build/routing-rules/routing-rules/:/home/tsokalo/workspace/new-kodo-rlnc/kodo-rlnc/kodo_build:./build /home/tsokalo/workspace/ns-allinone-3.25/ns-3.25/build/scratch/ghn-nc-plc-example
