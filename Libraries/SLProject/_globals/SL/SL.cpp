//#############################################################################
//  File:      Globals/SL.cpp
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h>           // precompiled headers
#ifdef SL_MEMLEAKDETECT
#include <nvwa/debug_new.h>   // memory leak detector
#endif

//-----------------------------------------------------------------------------
void SL::exitMsg(const SLchar* msg, const SLint line, const SLchar* file)
{  
   #ifdef SL_OS_ANDROID
   __android_log_print(ANDROID_LOG_INFO, "SLProject", 
                       "Exit %s at line %d in %s\n", msg, line, file);
   #else
   fprintf(stderr, "Exit %s at line %d in %s\n", msg, line, file);
   #endif
   
   #ifdef SL_MEMLEAKDETECT
   // turn off leak checks on forced exit
   new_autocheck_flag = false;
   #endif
   
   exit(-1);
}
//-----------------------------------------------------------------------------
void SL::warnMsg(const SLchar* msg, const SLint line, const SLchar* file)
{  
   #ifdef SL_OS_ANDROID
   __android_log_print(ANDROID_LOG_INFO, "SLProject", 
                       "Warning %s at line %d in %s\n", msg, line, file);
   #else
   fprintf(stderr, "Warning %s at line %d in %s\n", msg, line, file);
   #endif
}
//-----------------------------------------------------------------------------
