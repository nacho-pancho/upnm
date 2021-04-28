#!/bin/bash
cp data/camera.pgm .
cp data/peppers.ppm .
cp data/einstein.pbm .
build/test camera.pgm 
build/test peppers.ppm 
build/test einstein.pbm 
