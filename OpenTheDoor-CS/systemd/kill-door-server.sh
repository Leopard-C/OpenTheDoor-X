#!/bin/sh
 
NAME=door_server
PID=`ps -ef | grep "$NAME" | grep -v "grep" | awk '{print $2}'`

for pid in $PID
do
  kill -INT $pid
  echo "killed $pid"
done
