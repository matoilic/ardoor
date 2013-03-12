//#############################################################################
//  File:      SL3DSMeshFile.h
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Purpose:   3DS file loader
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLMeshFile3DS_H
#define SLMeshFile3DS_H

#include <stdafx.h>
#include "SLGroup.h"
#include "SL3DSMesh.h"

//-----------------------------------------------------------------------------
//! 3DS-File loader used for 3DS format import.
/*! The SL3DSMeshFile class provides all the file loading functionality to 
parse and load a 3DS file from 3DStudio Max. 
*/
class SL3DSMeshFile
{  public:
                        SL3DSMeshFile(){;} 
                       ~SL3DSMeshFile(){;}
              
      static SLGroup*   load(SLstring  filename,
                             SLint     dumpLevel=0,
                             SLbool    fixVerticesAmongSmoothgroups=true,
                             SLbool    rotate90DegAroundX=true);

      static SLstring   defaultPath;//!< Default path for 3DS models
      
   private:
      static SLGroup*   _g;         //!< group that holds the 3DS model
      static SLScene*   _s;         //!< parent scene
      static SLuint     _iMatStart; //!< start index in the scene material array
      static FILE*      _file;      //!< The binary 3ds file
      static SLint      _dumpLevel; //!< 0=off,1=main nodes, 2=details, 3=coords
      static SLbool     _fixVerticesAmongSmoothgroups;
      static SLbool     _rotate90DegAroundX;
      static SLuint*    _iFV;       //!< Temp. array for face vertex indexes
      static SLuint     _numiFV;    //!< Number of face vertex indexes
      static SLstring   _sFilename;
      static SLstring   _sPathname;
      static clock_t    _timeStart; //!< for time at start
      static clock_t    _timeMat;   //!< time to load material
      static clock_t    _timeObj;   //!< time to load objects
      static clock_t    _timePP;    //!< time to post process

      // Every chunk in the 3ds file starts with this struct
      struct ChunkHeader 
      {  SLushort id;      //!< The chunk's id
         SLint    len;     //!< The lenght of the chunk
      };
      
      static void       Read3DSChunkHdr(ChunkHeader &ch, SLint depth);
      static void       Read3DSString  (SLstring &str, SLint len = 256);   
      
      // Low level file data readers with little endian format
      //! Read3DSushort: Read a unsigned short from file in little endian format.
      static inline SLushort Read3DSushort()
      {  SLushort us;
         fread(&us, 1, 2, _file);
         return us;
      }
      //! Read3DSshort: Read a short from file in little endian format.
      static inline SLshort  Read3DSshort()
      {  SLshort s;
         fread(&s, 1, 2, _file);
         return s;
      }
      //! Read3DSulong: Read a 4 byte unsigned int from file in little endian format.
      static inline SLuint  Read3DSuint()
      {  SLuint ul;
         fread(&ul, 1, sizeof(SLuint), _file);
         return ul;
      }
      //! Read3DSlong:  Read a 4 byte int from file in little endian format.
      static inline SLint   Read3DSint()
      {  SLint l;
         fread(&l, 1, sizeof(SLint), _file);
         return l;
      }
      //! Read3DSfloat: Read a float from file in little endian format.
      static inline SLfloat  Read3DSfloat()
      {  SLfloat f;
         fread(&f, 1, sizeof(SLfloat), _file);
         return f;
      }
      
      // Low level data type chunk
      static void COLOR_TRU_Read                     (SLint depth, SLint len, SLint iFile, SLCol4f &color);
      static void COLOR_RGB_Read                     (SLint depth, SLint len, SLint iFile, SLCol4f &color);
      static void PERCENT_SHORT_Read                 (SLint depth, SLint len, SLint iFile, SLshort &percent);
      static void PERCENT_FLOAT_Read                 (SLint depth, SLint len, SLint iFile, SLfloat &percent);
      static void UNKNOWN_Read                       (SLint depth, SLint len, SLint iFile);
      
      // Processes the Main Chunk that all the other chunks exist is
      static void MAIN3DS_Read                       (SLint depth, SLint len, SLint iFile);
         static void MAIN_VERS_Read                  ();
         static void EDIT3DS_Read                    (SLint depth, SLint len, SLint iFile);
            static void EDIT_VERS_Read               ();
            static void EDIT_MASTERSCALE_Read        ();
            static void EDIT_MATERIAL_Read           (SLint depth, SLint len, SLint iFile);
               static void MAT_NAME_Read             (SLint iMat);
               static void MAT_AMBIENT_Read          (SLint depth, SLint len, SLint iFile, SLint iMat);
               static void MAT_DIFFUSE_Read          (SLint depth, SLint len, SLint iFile, SLint iMat);
               static void MAT_SPECULAR_Read         (SLint depth, SLint len, SLint iFile, SLint iMat);
               static void MAT_SHIN1_PCT_Read        (SLint depth, SLint len, SLint iFile, SLint iMat);
               static void MAT_SHIN2_PCT_Read        (SLint depth, SLint len, SLint iFile, SLint iMat);
               static void MAT_TRANSP1_PCT_Read      (SLint depth, SLint len, SLint iFile, SLint iMat);
               static void MAT_TRANSP2_PCT_Read      (SLint depth, SLint len, SLint iFile, SLint iMat);
               static void MAT_MAPTEX1_Read          (SLint depth, SLint len, SLint iFile, SLint iMat);
                  static void MAP_NAME_Read          (SLint iMat);
                  static void MAP_PARAMS_Read        (SLint depth, SLint len, SLint iFile, SLint iMat);
            static void EDIT_OBJECT_Read             (SLint depth, SLint len, SLint iFile);
               static void OBJ_TRIMESH_Read          (SLint depth, SLint len, SLint iFile, SLstring objName);
                  static void TRI_VERTEXL_Read       (SLint depth, SLint len, SLint iFile, SL3DSMesh* mesh);
                  static void TRI_VERTEXFLAGL_Read   (SLint depth, SL3DSMesh* mesh);
                  static void TRI_LOCALMATRIX_Read   (SLint depth, SLint len, SLint iFile, SL3DSMesh* mesh);
                  static void TRI_VISIBLE_Read       (SLint depth, SLint len, SLint iFile, SL3DSMesh* mesh);
                  static void TRI_MAPCOORDS_Read     (SLint depth, SLint len, SLint iFile, SL3DSMesh* mesh);
                  static void TRI_FACEL_Read         (SLint depth, SLint len, SLint iFile, SL3DSMesh* mesh);
                     static void FACE_MATERIAL_Read  (SLint depth, SLint len, SLint iFile, SL3DSMesh* mesh);
                     static void FACE_SMOOTHGRP_Read (SLint depth, SLint len, SLint iFile, SL3DSMesh* mesh);
               static void OBJ_LIGHT_Read            (SLint depth, SLint len, SLint iFile, SLstring objName);
               static void OBJ_CAMERA_Read           (SLint depth, SLint len, SLint iFile, SLstring objName);
         static void KEYF3DS_Read                    (SLint depth, SLint len, SLint iFile);
};
//-----------------------------------------------------------------------------
#endif //SLMeshFile3DS_H
