#!/bin/bash

 > TimeResult

for i in {1..50000}
do
    ./a.out $i >> TimeResult
done