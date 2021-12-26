# Download and extract openssl
mkdir openssl
wget "https://www.openssl.org/source/openssl-3.0.1.tar.gz"
tar -x -f "openssl-3.0.1.tar.gz" --overwrite -C "openssl"

# Configure openssl
cd openssl/openssl-3.0.1
./config --openssldir=/usr/local/ssl

# Compile openssl
make

# Install openssl
make install

echo "Your ready to go!"