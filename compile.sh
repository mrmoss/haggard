#!/bin/sh
MSL_DIR=msl
MSL="${MSL_DIR}/2d.cpp ${MSL_DIR}/2d_util.cpp \
	${MSL_DIR}/glut_input.cpp ${MSL_DIR}/glut_ui.cpp \
	${MSL_DIR}/socket.cpp ${MSL_DIR}/socket_util.cpp \
	${MSL_DIR}/string_util.cpp ${MSL_DIR}/time_util.cpp"

FALCONER_DIR="falconer"
FALCONER="${FALCONER_DIR}/falconer.cpp"

LIB="-lGL -lglut -lGLEW -lSOIL -lftgl"

DIRS="-I. -I/usr/include/freetype2"

g++ main.cpp ${MSL} ${FALCONER} ${DIRS} -o haggard -O -Wall ${LIB}