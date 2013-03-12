//#############################################################################
//  File:      SLUniformGrid.h
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLUNIFORMGRID_H
#define SLUNIFORMGRID_H

#include <stdafx.h>

#include "SLAccelStruct.h"

//-----------------------------------------------------------------------------
//! Special vector type similar to std::vector w. smaller size type
typedef SLVector<SLushort, SLushort> SLV16ushort;
//-----------------------------------------------------------------------------
//! SLUniformGrid is an acceleration structure the uniformly subdivides space.
/*! A uniform grid is an axis aligned, regularly subdivided grid for 
accelerating the ray-triangle intersection.
*/
class SLUniformGrid : public SLAccelStruct    
{  public:
                              SLUniformGrid  (SLMesh* m);
                             ~SLUniformGrid  ();

               void           build          (SLVec3f minV, SLVec3f maxV);
               void           updateStats    (SLGroup* parent);
               void           draw           (SLSceneView* sv);
               SLbool         intersect      (SLRay* ray);
               
               // Delete the vertex buffer object if not rendered anymore
               void           disposeBuffers (){ if (_bufP.id()) _bufP.dispose();}
               
   private:
               SLV16ushort*   _vox;          //!< 1D voxel array for tria. indexes
               SLint          _voxResX;      //!< Voxel resolution in x-dir.
               SLint          _voxResY;      //!< Voxel resolution in y-dir.
               SLint          _voxResZ;      //!< Voxel resolution in z-dir.
               SLint          _voxResXY;     //!< = _voxResX * _voxResY
               SLfloat        _voxExtX;      //!< Voxel extent in x-dir
               SLfloat        _voxExtY;      //!< Voxel extent in y-dir
               SLfloat        _voxExtZ;      //!< Voxel extent in z-dir
               
               SLGLBuffer     _bufP;   //!< Buffer object for vertex positions
};
//-----------------------------------------------------------------------------
#endif //SLUNIFORMGRID_H

