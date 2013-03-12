//#############################################################################
//  File:      _globals/SLFileSystem.h
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h>

#ifndef SLFILESYSTEM_H
#define SLFILESYSTEM_H

//-----------------------------------------------------------------------------
//! SLFileSystem provides basic filesystem functions
class SLFileSystem
{  public:
   
   /*! 
   fileExists returns true if a file exists.
   */
   static SLbool fileExists(SLstring& pathfilename);
};
//-----------------------------------------------------------------------------
#endif



