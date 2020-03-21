#!/bin/bash

./amalgam.py --prefix=MG $(cat mongoose.h.manifest) > mongoose.h
./amalgam.py --prefix=MG --public-header=mongoose.h $(cat mongoose.c.manifest) > mongoose.c
