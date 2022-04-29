#!/bin/bash
# Esse script foi baseado no site: https://www.baeldung.com/linux/process-periodic-cpu-usage

port=$1
proc="./bin/broker"
N=$2
cpulogfile="./analise/dados/$N-clients-cpu.log"
netlogfile="./analise/net-captures/$N-clients-net.log"

> $cpulogfile
> $netlogfile
./bin/broker $port & 1>/dev/null
sudo nethogs -d 1 -t enp2s0 &>> $netlogfile &
while true;
do
    echo "+++++++++++++++++++++++++++" >> $cpulogfile;
    ps ux | grep "./bin/broker" | grep -v grep >> $cpulogfile;
    sleep 1
done
