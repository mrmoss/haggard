/**
  Portably delete a file.
  
Orion Sky Lawlor, olawlor@acm.org, 2006/3/15 (Public Domain)
*/
#ifndef __OSL_UNLINK_H
#define __OSL_UNLINK_H

#ifdef WIN32
#include <windows.h>
namespace osl {
  inline bool unlink(const char *file) { 
	return 0==DeleteFile(file);
  }
};
#else /* UNIX-like system */
#include <unistd.h>
namespace osl {
  inline bool unlink(const char *file) { 
	return 0==::unlink(file);
  }
};
#endif


#endif
