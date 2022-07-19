cd ~/Downloads/ElasticFusion/
git submodule update --init
cd third-party/OpenNI2/
make -j12
cd ../Pangolin/
mkdir build
cd build
cmake .. -DEIGEN_INCLUDE_DIR=$HOME/Downloads/ElasticFusion/third-party/Eigen/ -DBUILD_PANGOLIN_PYTHON=false
make -j12
cd ../../..
mkdir build
cd build/
cmake ..
make ElasticFusion -j12
