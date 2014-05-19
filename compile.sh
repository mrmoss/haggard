#!/bin/bash

#Housework
	#Get OS
	OS='uname -s'

#Compiler
	COMPILER="g++"

#Sources
	#Falconer
	FALCONER_DIR="falconer"
	FALCONER="${FALCONER_DIR}/falconer.cpp"

	#Haggard
	HAGGARD="main.cpp"

	#MSL
	MSL_DIR="msl"
	MSL="${MSL_DIR}/2d.cpp ${MSL_DIR}/2d_util.cpp \
		${MSL_DIR}/glut_input.cpp ${MSL_DIR}/glut_ui.cpp \
		${MSL_DIR}/socket.cpp ${MSL_DIR}/socket_util.cpp \
		${MSL_DIR}/sprite.cpp ${MSL_DIR}/string_util.cpp \
		${MSL_DIR}/time_util.cpp"

	#SOIL
	SOIL_DIR="SOIL"
	SOIL="${SOIL_DIR}/stb_image_aug.c ${SOIL_DIR}/SOIL.c"

	#Full Source
	SRC="${FALCONER} ${HAGGARD} ${MSL} ${SOIL}"

#Libraries
	#GL
	if [ "${OS}" == "Darwin" ]
	then
		OS_GL="-framework OpenGL -framework Glew -framework GLUT"
	else
		OS_GL="-lGL -lglut -lGLEW"
	fi

	#OpenCV
	OPENCV="-lopencv_core -lopencv_highgui -lopencv_features2d -lopencv_imgproc -lopencv_video"

	#PThread
	PTHREAD="-lpthread"

	#Full Libraries
	LIB="-lftgl ${OS_GL} ${OPENCV} ${PTHREAD}"

#Binary Name
	BIN="-o haggard"

#Compiler Flags
	CFLAGS="-O -Wall"

#Search Directories
	DIRS="-I. -I/usr/local/include -L/usr/local/lib -I/usr/include/freetype2"

#Compile
${COMPILER} ${SRC} ${LIB} ${BIN} ${CFLAGS} ${DIRS}