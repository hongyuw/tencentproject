#!/bin/bash


sysctl -w net.core.rmem_max=167772160
sysctl -w net.core.wmem_max=167772160
sysctl -w net.core.rmem_default=167772160
sysctl -w net.core.wmem_default=167772160
sysctl -w net.ipv4.tcp_rmem='40960 873800 16777216'
sysctl -w net.ipv4.tcp_wmem='40960 873800 16777216'
clear
echo start
./DEMOPROXY 
