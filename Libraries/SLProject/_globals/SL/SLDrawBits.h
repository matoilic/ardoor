//#############################################################################
//  File:      SLDrawBits.h
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLDRAWBITS_H
#define SLDRAWBITS_H

#include <stdafx.h>

//-----------------------------------------------------------------------------
/*!
Drawing Bits control some visual states of the scene and are applied per scene 
view or per single shape objects. Not all are used from the beginning
*/
#define SL_DB_HIDDEN      1   //Hides an object   
#define SL_DB_POLYGONLINE 2   //Draw polygons as wireframe 
#define SL_DB_CULLOFF     4   //Face culling off
#define SL_DB_NORMALS     8   //Draw the normals
#define SL_DB_TEXOFF     16   //Turn off texture mapping
#define SL_DB_SELECTED   32   //Flags an object as selected  
#define SL_DB_BBOXGROUP  64   //Draw bounding boxes of the groups
#define SL_DB_BBOX      128   //Draw bounding boxes of the shapes  
#define SL_DB_VOXELS    256   //Draw the voxels of the uniform grid 
#define SL_DB_NOANIM    512   //Stops any animation

//-----------------------------------------------------------------------------
//! Drawing states stored in the bits of an unsigned int
/*! The drawing bits can be applied to the entire scene, a group or a mesh. The
default value is 0 signifying the default state. See the #defines above for the
different drawing bit flags. 
*/
class SLDrawBits
{     
   public:           
                        SLDrawBits (){_drawBits=0;}
                       ~SLDrawBits (){;}
            
            void        allOff     () {_drawBits=0;}
            void        on     (SLuint bit){SL_SETBIT(_drawBits, bit);}
            void        set     (SLuint bit, SLbool val){if(val) SL_SETBIT(_drawBits, bit);
                                                            else SL_DELBIT(_drawBits, bit);}
            void        off     (SLuint bit){SL_DELBIT(_drawBits, bit);}
            void        toggle  (SLuint bit){SL_TOGBIT(_drawBits, bit);}
            SLbool      get     (SLuint bit){return (_drawBits&bit)?true:false;}

   private:            
            SLuint      _drawBits; //!< Drawing flags for the view
};
//-----------------------------------------------------------------------------
#endif
