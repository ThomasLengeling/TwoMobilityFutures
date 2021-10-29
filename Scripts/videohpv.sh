#!/bin/bash

echo "Creating HPV Videos for $1"

sleep 1

mkdir "./video/$1/temp1" ;
mkdir "./video/$1/temp2" ;
mkdir "./video/$1/temp3" ;
mkdir "./video/$1/temp4" ;
mkdir "./video/$1/temp5" ;
mkdir "./video/$1/temp6" ;
mkdir "./video/$1/temp7" ;
mkdir "./video/$1/temp8" ;
mkdir "./video/$1/temp9" ;

./ffmpeg.exe -i "./video/$1/sequence_01.mp4"  -map 0:1 -ac 2 "./video/$1/"$1"_sequence_01.wav" ;

./ffmpeg.exe -i "./video/$1/sequence_01.mp4" "./video/$1/temp1/sequence_01_%05d.png" ;
./ffmpeg.exe -i "./video/$1/sequence_02.mp4" "./video/$1/temp2/sequence_02_%05d.png" ;
./ffmpeg.exe -i "./video/$1/sequence_03.mp4" "./video/$1/temp3/sequence_03_%05d.png" ;
./ffmpeg.exe -i "./video/$1/sequence_04.mp4" "./video/$1/temp4/sequence_04_%05d.png" ;
./ffmpeg.exe -i "./video/$1/sequence_05.mp4" "./video/$1/temp5/sequence_05_%05d.png" ;
./ffmpeg.exe -i "./video/$1/sequence_06.mp4" "./video/$1/temp6/sequence_06_%05d.png" ;
./ffmpeg.exe -i "./video/$1/sequence_07.mp4" "./video/$1/temp7/sequence_07_%05d.png" ;
./ffmpeg.exe -i "./video/$1/sequence_08.mp4" "./video/$1/temp8/sequence_08_%05d.png" ;
./ffmpeg.exe -i "./video/$1/sequence_09.mp4" "./video/$1/temp9/sequence_09_%05d.png" ;

./HPVCreatorConsole.exe -i "./video/$1/temp1" -f 24 -t 0 -n 8 -o "./video/$1/"$1"_sequence_01.hpv" ;
./HPVCreatorConsole.exe -i "./video/$1/temp2" -f 24 -t 0 -n 8 -o "./video/$1/"$1"_sequence_02.hpv" ;
./HPVCreatorConsole.exe -i "./video/$1/temp3" -f 24 -t 0 -n 8 -o "./video/$1/"$1"_sequence_03.hpv" ;
./HPVCreatorConsole.exe -i "./video/$1/temp4" -f 24 -t 0 -n 8 -o "./video/$1/"$1"_sequence_04.hpv" ;
./HPVCreatorConsole.exe -i "./video/$1/temp5" -f 24 -t 0 -n 8 -o "./video/$1/"$1"_sequence_05.hpv" ;
./HPVCreatorConsole.exe -i "./video/$1/temp6" -f 24 -t 0 -n 8 -o "./video/$1/"$1"_sequence_06.hpv" ;
./HPVCreatorConsole.exe -i "./video/$1/temp7" -f 24 -t 0 -n 8 -o "./video/$1/"$1"_sequence_07.hpv" ;
./HPVCreatorConsole.exe -i "./video/$1/temp8" -f 24 -t 0 -n 8 -o "./video/$1/"$1"_sequence_08.hpv" ;
./HPVCreatorConsole.exe -i "./video/$1/temp9" -f 24 -t 0 -n 8 -o "./video/$1/"$1"_sequence_09.hpv" ;
