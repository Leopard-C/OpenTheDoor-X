#!/bin/bash

echo Here

config_path=/home/pi/.door/client/config
exec_path=/home/pi/Documents/cpp/OpenTheDoor-X/OpenTheDoor-CS/client

for i in {1..10}
do
  # detect network
  ping -c 1 114.114.114.114 > /dev/null 2>&1
  if [ $? -eq 0 ]; then
    echo OK

    # get ip address
    ip_new=$(ifconfig |grep inet|grep -v 127.0.0.1|grep -v inet6|awk '{print $2}'|tr -d "addr:")
    ip_old=$(cat $config_path/client.json|grep server_addr |sed 's/.*\"\([0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\)",/\1/g')

    echo New ip address is $ip_new
    echo Old ip address is $ip_old

    # replace by new ip address
    if [[ $ip_new != $ip_old ]]; then
      sed "s/[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}/$ip_new/g" $config_path/client.json -i
    fi

    $exec_path/door_client
    exit 0
  else
    echo Network error
    sleep 5s
  fi
done

