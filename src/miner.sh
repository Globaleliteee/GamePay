#!/bin/bash

# Wait for the daemon to be ready
sleep 10

# Set number of blocks to generate
BLOCKS=100

# Get a new wallet address
ADDRESS="pay1qadapyc76haq8l3v5zy9hcz8e5ydv2zspxyfwpf"

# Mine blocks to the address
while true; do
  ./gamepay-cli  generatetoaddress $BLOCKS $ADDRESS
  sleep 1
done
