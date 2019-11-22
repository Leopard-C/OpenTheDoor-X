#!/bin/bash

# run as user pi. And root is not allowed
if [ `id -u` -eq 0 ]; then
  echo "Do not run as root"
  echo "Exit"
  exit 1
fi


# make directory
#

if [ ! -d "$HOME/.door/systemd" ]; then
  mkdir ~/.door/systemd -p
fi

if [ ! -d "$HOME/.door/client/logs" ]; then
  mkdir ~/.door/client/logs -p
fi

if [ ! -d "$HOME/.door/client/config" ]; then
  mkdir ~/.door/client/config -p
fi

if [ ! -d "$HOME/.door/server/logs" ]; then
  mkdir ~/.door/server/logs -p
fi

if [ ! -d "$HOME/.door/server/config" ]; then
  mkdir ~/.door/server/config -p
fi

if [ ! -d "$HOME/.door/server/cert" ]; then
  mkdir ~/.door/server/cert -p
fi


# copy config file
#
cp ./client/config/* ~/.door/client/config
cp ./server/config/* ~/.door/server/config
cp ./server/cert/* ~/.door/server/cert

cp ./systemd/*.sh ~/.door/systemd

echo "OK!"
