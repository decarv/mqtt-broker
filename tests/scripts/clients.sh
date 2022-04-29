#!/bin/bash

SLEEPTIME=1


address=$1
port=$2
num=$3
echo "Criando $num clientes para endereço $address e porta $port"

topics=("a" "b" "c" "d" "e")
len=${#topics[@]}

echo "tamanho de topic $len"

for i in $(seq 0 $num)
do
  index=`expr $i % $len`
  topic=${topics[$index]}
  echo "Inscrevendo sub no topico $topic"
  mosquitto_sub -h $address -p $port -t $topic &
done

for i in $(seq 0 $num)
do
  index=$(($i % $len))
  topic=${topics[$index]}
  message=`date +%s | md5sum | head -c 32`
  echo "Publicando mensagem $message no topico $topic"
  mosquitto_pub -h $address -p $port -t $topic -m $message &
  sleep $SLEEPTIME
done
