# Download and extract openssl
mkdir openssl
wget "https://www.openssl.org/source/openssl-1.1.1m.tar.gz"
tar -x -f "openssl-1.1.1m.tar.gz" --overwrite -C "openssl"

# Configure openssl
cd "openssl/openssl-1.1.1m"
./config --openssldir=/usr/local/ssl

# Compile openssl
make

# Install openssl
make install

echo "Your ready to go!"
