# Download and extract boost
mkdir /temp/boost/
wget "https://boostorg.jfrog.io/native/main/release/1.75.0/source/boost_1_75_0.tar.bz2" -o "/temp/boost/boost.tar.bz2"
tar -x -f "/temp/boost/boost.tar.bz2" --overwrite -C "/temp/boost/boost_1_75_0"

# Configure boost
/temp/boost/boost_1_75_0/bootstrap.sh

# Install boost
/temp/boost/boost_1_75_0/b2 install