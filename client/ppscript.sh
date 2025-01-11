#!/bin/bash
mkdir -p ../cloud_enc/encrypted_data
mkdir -p ../cloud_enc/tapes
filename="preAux.txt"
wordsize="8"
./preprocessor $filename $wordsize
