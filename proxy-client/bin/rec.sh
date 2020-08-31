#!/bin/bash


sysctl -w net.core.optmem_max=20480
sysctl -w net.core.rmem_default=22937
sysctl -w net.core.rmem_max=22937
sysctl -w net.core.wmem_default=22937
sysctl -w net.core.wmem_max=22937
sysctl -w net.ipv4.tcp_mem='3542 472336 70850'
sysctl -w net.ipv4.tcp_rmem='2296 8738 629145'
sysctl -w net.ipv4.tcp_wmem='2096 1638 419430'
clear
