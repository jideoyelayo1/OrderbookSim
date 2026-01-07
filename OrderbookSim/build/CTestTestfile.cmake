# CMake generated Testfile for 
# Source directory: /Users/jideoyelayo/Documents/Programming/AITradingSim/OrderbookSim/OrderbookSim
# Build directory: /Users/jideoyelayo/Documents/Programming/AITradingSim/OrderbookSim/OrderbookSim/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[OrderbookCoreTests]=] "/Users/jideoyelayo/Documents/Programming/AITradingSim/OrderbookSim/OrderbookSim/build/OrderbookTestExec")
set_tests_properties([=[OrderbookCoreTests]=] PROPERTIES  WORKING_DIRECTORY "/Users/jideoyelayo/Documents/Programming/AITradingSim/OrderbookSim/OrderbookSim/OrderbookTest" _BACKTRACE_TRIPLES "/Users/jideoyelayo/Documents/Programming/AITradingSim/OrderbookSim/OrderbookSim/CMakeLists.txt;73;add_test;/Users/jideoyelayo/Documents/Programming/AITradingSim/OrderbookSim/OrderbookSim/CMakeLists.txt;0;")
subdirs("_deps/googletest-build")
