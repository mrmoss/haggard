/*
  Select the appropriate configuration header.
  Orion Sky Lawlor, olawlor@acm.org, 6/29/2002
*/
#ifndef __OSL_CONFIG_H
#define __OSL_CONFIG_H

#if WIN32
/* Windows: use hardcoded values*/
#  include "osl/config_win32.h"
#else
/* Must be UNIX: grab automatic header.
   If this file doesn't exist, run osl/unix/config/config.sh
*/
#  include "osl/config_auto.h"
#endif

/*FIXME: this shouldn't be hardcoded, but I think it's right*/
#define OSL_HAS_UINT64 1

#endif /*def(thisHeader)*/
