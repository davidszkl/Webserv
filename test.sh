#!/bin/bash

for n in {0..10000}; do
	j=$(($n % 2))
	if [ $j -eq 1 ]; then
		curl -H "count: $n" -X GET "http://127.0.0.1:4242/not_found"
	else
		curl -H "count: $n" -X GET "http://127.0.0.1:4242/index.html"
	fi
done;