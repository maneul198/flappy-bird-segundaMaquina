#! /bin/bash

while [ 1 = 1 ]; do
	sleep 60
	corriendo=$(ps -alx | grep smart-vending-flappy-bird \
		| grep -v 'grep' | wc -l)
	if [ $corriendo != 1 ]; then
		smart-vending-flappy-bird &
	fi
done
