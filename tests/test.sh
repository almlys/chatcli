#!/bin/bash
for i in `seq 1 500`; do
  let port=7000+i
  python testtool.py bep$i localhost port 25 &
done
