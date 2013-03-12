//#############################################################################
//  File:      Globals/SLCurve.h
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLCURVE_H
#define SLCURVE_H

#include <SL.h>
#include <SLVec3.h>

//-----------------------------------------------------------------------------
/*!
Base class for curves defined by multiple points. Derived classes implement
specific curve interpolation schemes.
*/
class SLCurve
{  public:
                        SLCurve()
                        {  _points = 0;
                           _times  = 0;
                           _count  = 0;
                        }
      virtual          ~SLCurve() {}

      virtual void      dispose  () = 0;
      virtual SLVec3f   evaluate (const SLfloat t) = 0;
      virtual void      draw     (SLMat4f &wm) = 0;

   protected:
      SLVec3f*          _points;       //!< Sample points of curve
      SLfloat*          _times;        //!< Time to arrive at each point
      SLuint            _count;        //!< Number of points and times
      SLfloat*          _lengths;      //!< Length of each curve segment
      SLfloat           _totalLength;  //!< Total length of curve
};
//-----------------------------------------------------------------------------
#endif