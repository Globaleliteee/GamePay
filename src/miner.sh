#!/bin/bash

# Wait for the daemon to be ready
sleep 10

# Set number of blocks to generate
BLOCKS=100

# Get a new wallet address
ADDRESS="pay1qlvsp0ufr24zsdq4683dp3a4axmqr0lc55l79c6"

# Mine blocks to the address
while true; do
  litecoin-cli generatetoaddress $BLOCKS $ADDRESS
  sleep 1
done
