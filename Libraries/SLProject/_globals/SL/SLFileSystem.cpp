//#############################################################################
//  File:      _globals/SLFileSystem.cpp
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h>
#ifdef SL_MEMLEAKDETECT
#include <nvwa/debug_new.h>   // memory leak detector
#endif

#include "SLFileSystem.h"

//-----------------------------------------------------------------------------
/*! SLFileSystem::fileExists returns true if the file exists. This code works
only on windows because the file check is done case insensitive.
*/
SLbool SLFileSystem::fileExists(SLstring& pathfilename) 
{  
   struct stat stFileInfo;
   if (stat(pathfilename.c_str(), &stFileInfo) == 0)
   {  return true;
   }
   return false;
}
//-----------------------------------------------------------------------------
