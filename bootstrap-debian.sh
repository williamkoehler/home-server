# Update repository
apt update

# Install build essentials
apt install -y build-essential

# Install cmake
apt install -y cmake

# Install necessary packages
./booststrap-boost.sh
apt install -y libc6-dev libssl-dev libsqlite3-dev libcurl4-openssl-dev

echo "Your ready to go!"