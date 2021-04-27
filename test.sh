#!/bin/bash
cp data/camera.pgm .
cp data/peppers.ppm .
build/test camera.pgm 
build/test peppers.ppm 
