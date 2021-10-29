#!/bin/bash

echo "Creating HPV Audio for $1"

sleep 1

./ffmpeg.exe -i "./video/$1/sequence_01.mp4" -map 0:1 "./video/$1/"$1"_sequence_01.wav"

sleep 2
