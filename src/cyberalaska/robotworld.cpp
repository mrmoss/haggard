/**
Parse command line arguments to create all robots.

Dr. Orion Sky Lawlor, lawlor@alaska.edu, 2013-11-06 (Public Domain)
*/
#include "../cyberalaska/robotworld.h"
#include "../cyberalaska/bullseye_camera.h"

#include "../cyberalaska/rover5.cpp"
#include "../cyberalaska/hindenburg_robot.cpp"
#include "../cyberalaska/bullseye_camera.cpp"
#include "../rasterCV/bullseye.cpp"

#include "../cyberalaska/porthread.cpp"
#include "../msl/serial.cpp"
#include "../msl/time_util.cpp"

void show_help() {
std::cout<<"Usage: controller --help\n"
	 <<"       controller --rover5 DEVICE --cam CAMERA\n\n"
	 <<"Options:\n"
	 <<"  --help		Print this message and exit.\n"
	 <<"  --rover5 DEVICE	Defines which comm device to use.\n"
	  <<"  --hindenburg DEVICE	Defines which comm device to use.\n"
	 <<"  --cam CAMERA		Defines which camera to use.\n\n";
}

/// Set up the robot world.  Check the length of the lists to see if
///  it's acceptable for your purposes.
cyberalaska::robotworld::robotworld(int argc,char *argv[])
{
	int argi=1; // command line argument index
	if (argc<=argi) {
		show_help();
		exit(0);
	}
	else {
		while (argc>argi) { /* keyword-value pairs */
			char *key=argv[argi++];
			if (0==strcmp(key,"--rover5")) {
				ground.push(make_rover5(argv[argi++]));
			}
			else if (0==strcmp(key,"--hindenburg")) {
				air.push(make_hindenburg(argv[argi++]));
			}
			else if (0==strcmp(key,"--cam")) {
				camera.push(make_bullseye_camera(atoi(argv[argi++])));
			}
			else if (0==strcmp(key,"--help")) {
				show_help();
				exit(0);
			}
			else {
				std::cerr<<"Unrecognized command line argument '"<<key<<"'!\n";
				exit(1);
			}
		}
	}

	std::cout<<"Created world with "<<ground.length<<" ground and "<<air.length<<" air vehicles\n";
}

