# Update repository
apt update

# Install build essentials
apt install -y build-essential clang-13

# Install cmake
apt install -y cmake

# Install python & conan
apt install -y python3 python3-pip
pip install conan

# Install necessary packages
apt install -y wget libc6-dev libcurl4-openssl-dev libicu-dev

echo "Your ready to go!"