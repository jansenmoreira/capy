#!/bin/bash

echo -e "GET /test/ HTTP/1.1" | nc -v $1 $2
# echo -e "GET https:///test HTTP/1.1\r\nHost: 127.0.0.1:80\r\nConnection: close\r\n\r\n" | nc -v $1 $2
