#!/bin/bash
for i in `seq 1 100`; do
  let port=7000+i
  python testtool.py bep$i localhost $port 10 &
done
