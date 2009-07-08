#!/bin/sh

echo "UP"
ip r a 192.168.8.0/24 via 192.168.5.1
ip r a 192.168.100.16 via 192.168.5.1
