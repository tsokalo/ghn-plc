# ghn-plc
This repository contains the ns-3 based simulation model of G.hn protocol with PLC physical layer. In also includes the modified plc ns-3 module and also plc-fd module, which enable full-duplex for CSMA/CD in PLC. In addition there are evaluation programs and scripts as described below.

Copy ghn-plc, plc and plc-fd folders to the src folder of your ns3 folder.

Copy the file from the scratch folder to the scratch folder of your ns3 folder.

Run run-test-csma-ca.py and run-test-csma-cd.py to obtain the simulation results.

The folder EvalSimGhn contains the C++ program to evaluate the simulation results.

The folder isplc2017 contains the C++ program to obtain the analytic results.

The folder sim contains the scripts to build the plots basing on the evaluated simulation and analytic results.

The folder calc contains the scripts to build the plots of analytical results.

The README files in calc/ and in sim/ contain the full list of instructions.


