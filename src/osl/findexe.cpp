/**
 Locate the running executable.
 
 INCOMPLETE example program.
 
 Orion Sky Lawlor, olawlor@acm.org, 2006/03/21 (Public Domain)
*/

#define PATH_LEN 2048

/************* Windows **************
http://www.thescarms.com/vbasic/PathFinder.asp
suggests GetModuleHandle & GetModuleFileName.
*/
#include <windows.h>

std::string osl::findExe(void)
{
	char buf[PATH_LEN];
	HANDLE hExe=GetModuleHandle(NULL);
	assert(hExe!=NULL);
	assert(GetModuleFileName(hExe,buf,PATH_LEN)));
	return buf;
}

/************** Linux *****************/

/* From "getexename", by Nicolai Haehnle <prefect_@gmx.net>
  http://www.flipcode.com/cgi-bin/fcarticles.cgi?show=64160
*/
#include <sys/types.h>
#include <unistd.h>

std::string osl::findExe(void)
{
	char buf[PATH_LEN];
	pid_t pid;
	int ret;
	
	/* Read the symbolic link */
	ret=readlink("/proc/self/exe", buf, PATH_LEN);
	assert(ret!=-1);
	assert(ret<size);
	
	/* Ensure proper NUL termination */
	buf[ret] = 0;
	
	return buf;
}

/*************** generic ****************
  Walk the PATH, looking for argv[0].
*/
????
