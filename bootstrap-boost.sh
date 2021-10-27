# Download and extract boost
mkdir boost
wget "https://boostorg.jfrog.io/artifactory/main/release/1.75.0/source/boost_1_75_0.tar.bz2"
tar --bzip2 -x -f "boost_1_75_0.tar.bz2" --overwrite -C "boost"

# Configure boost
cd boost/boost_1_75_0
./bootstrap.sh

# Install boost
./b2 install

echo "Your ready to go!"