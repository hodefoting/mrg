#!/bin/sh

echo "source: $1"
echo "command: " ../mrg browser $1 -o  output/`echo $1|sed s/html/png/`
../mrg browser $1 -o  output/`echo $1|sed s/html/png/` || true

