#!/bin/bash

echo -e "GET /jansen/ HTTP/1.1" | nc -v $1 $2
