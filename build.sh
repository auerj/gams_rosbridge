g++ main.cpp -std=c++11 -orosbridge \
-I$MADARA_ROOT/include -I$GAMS_ROOT/src -I$CAPNP_ROOT/c++/src -I$EIGEN_ROOT -I$ROS_ROOT/../../include -I/usr/include/pcl-1.7/  \
-L$GAMS_ROOT/lib/ -L$MADARA_ROOT/lib/ -L$ROS_ROOT/../../lib -lboost_system -lrostime -lroscpp -lcpp_common -lMADARA -lGAMS