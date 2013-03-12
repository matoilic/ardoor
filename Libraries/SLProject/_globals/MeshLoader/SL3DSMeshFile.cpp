//#############################################################################
//  File:      SL3DSMeshFile.cpp
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

#include "SLScene.h"
#include "SL3DSMeshFile.h"
#include "SLMesh.h"
#include "SLMaterial.h"
#include "SLGLTexture.h"

//-----------------------------------------------------------------------------
//! Default path for 3DS models used when only filename is passed in load.
SLstring SL3DSMeshFile::defaultPath = "../_data/models/3DS/";
//-----------------------------------------------------------------------------
// The chunk's id numbers
#define MAIN3DS                    0x4D4D
  #define MAIN_VERS                0x0002
  #define EDIT3DS                  0x3D3D
    #define EDIT_VERS              0x3D3E
                                  
    #define EDIT_MATERIAL          0xAFFF
      #define MAT_NAME             0xA000
      #define MAT_AMBIENT          0xA010
      #define MAT_DIFFUSE          0xA020
      #define MAT_SPECULAR         0xA030
      #define MAT_SHIN1_PCT        0xA040
      #define MAT_SHIN2_PCT        0xA041 // not used
      #define MAT_TRANSP1_PCT      0xA050
      #define MAT_TRANSP2_PCT      0xA052 //skipped
      #define MAT_REF_BLUR_PCT     0xA053 //skipped
      #define MAT_RENDER_TYPE      0xA100 //skipped
      #define MAT_SELF_ILLUM       0xA084 //skipped
      #define MAT_SELF_ILPCT       0xA08A //skipped
      #define MAT_WIRE_THICKNESS   0xA087 //skipped
      
      #define MAT_MAPTEX1          0xA200
      #define MAT_MAPTEX2          0xA33A //skipped
      #define MAT_MAPREFL          0xA220 //skipped
      #define MAT_MAPBUMP          0xA230 //skipped
      #define MAT_MAPOPACITY       0xA210 //skipped
      #define MAT_MAPSHININESS     0xA33C //skipped
      #define MAT_MAPSPECULAR      0xA204 //skipped
      #define MAT_MAPSELFILLUM     0xA33D //skipped
        #define MAP_NAME           0xA300
        #define MAP_TILING         0xA351 //skipped
        #define MAP_BLUR_PERC      0xA353 //skipped
        #define MAP_SCALEU         0xA354
        #define MAP_SCALEV         0xA356
        #define MAP_OFFSETU        0xA358
        #define MAP_OFFSETV        0xA35A
                                  
    #define EDIT_MASTERSCALE       0x0100 //skipped
    #define EDIT_CONFIG2           0x3E3D //skipped
    #define EDIT_VIEW_P1           0x7012 //skipped
    #define EDIT_VIEW_P2           0x7011 //skipped
    #define EDIT_VIEW_P3           0x7020 //skipped
    #define EDIT_VIEW1             0x7001 //skipped
    #define EDIT_BACKGR            0x1200 //skipped
    #define EDIT_AMBIENT           0x2100 //skipped
                                                      
    #define EDIT_OBJECT            0x4000
      #define OBJ_TRIMESH          0x4100
        #define TRI_VERTEXL        0x4110
        #define TRI_VERTEXFLAGL    0x4111 //Read but ignored
        #define TRI_MAPCOORDS      0x4140
        #define TRI_MAPSTANDARD    0x4170 //skipped
        #define TRI_LOCALMATRIX    0x4160 //skipped                  
        #define TRI_VISIBLE        0x4165 //skipped
        #define TRI_FACEL          0x4120
          #define FACE_MATERIAL    0x4130
          #define FACE_SMOOTHGRP   0x4150 
      #define OBJ_LIGHT            0x4600 //skipped
        #define LIT_OFF            0x4620 //skipped
        #define LIT_SPOT           0x4610 //skipped
        #define LIT_UNKNWN01       0x465A //skipped
      #define OBJ_CAMERA           0x4700 //skipped
        #define CAM_UNKNWN01       0x4710 //skipped
        #define CAM_UNKNWN02       0x4720 //skipped
                                  
  #define KEYF3DS                  0xB000 //skipped
    #define KEYF_FRAMES            0xB008 //skipped
    #define KEYF_UNKNOWN01         0xB009 //skipped
    #define KEYF_UNKNOWN02         0xB00A //skipped
    #define KEYF_OBJDES            0xB002 //skipped
      #define KEYF_DUMMY_NAME      0xB011 //skipped
      #define KEYF_HIERARCHY       0xB010 //skipped
      #define KEYF_PIVOT           0xB020 //skipped
      #define KEYF_UNKNOWN_03      0xB013 //skipped
      #define KEYF_UNKNOWN_04      0xB014 //skipped
      #define KEYF_UNKNOWN_05      0xB015 //skipped
      #define KEYF_UNKNOWN_06      0xB021 //skipped
      #define KEYF_UNKNOWN_07      0xB022 //skipped
      #define KEYF_UNKNOWN_08      0xB030 //skipped
                                  
#define COLOR_RGB                  0x0010
#define COLOR_TRU                  0x0011
#define COLOR_TRUG                 0x0012
#define COLOR_RGBG                 0x0013
#define PERCENT_SHORT              0x0030
#define PERCENT_FLOAT              0x0031
                
//-----------------------------------------------------------------------------
SLGroup* SL3DSMeshFile::_g = 0;        
FILE*    SL3DSMeshFile::_file = 0;         
SLint    SL3DSMeshFile::_dumpLevel = 0;    
SLuint*  SL3DSMeshFile::_iFV = 0; 
SLuint   SL3DSMeshFile::_numiFV = 0; 
SLbool   SL3DSMeshFile::_fixVerticesAmongSmoothgroups = true; 
SLbool   SL3DSMeshFile::_rotate90DegAroundX = true;
clock_t  SL3DSMeshFile::_timeStart = 0;
clock_t  SL3DSMeshFile::_timeMat = 0;
clock_t  SL3DSMeshFile::_timeObj = 0;
clock_t  SL3DSMeshFile::_timePP = 0;
SLstring SL3DSMeshFile::_sPathname = "";
SLstring SL3DSMeshFile::_sFilename = "";

//-----------------------------------------------------------------------------
//! Load a 3DS-file and returns a SLGroup.
/*! Load a 3DS-file and returns a SLGroup node with one SLMesh per object.
\param pathfilename Full path w. filename or filename only (default path is used)
\param dumpLevel Dump Level (0=nothing, 3=detailed)
\param fixVerticesAmongSmoothgroups Flag for fixeing the mesh vertex structure.
\param rotate90DegAroundX Flag for swapping y- with z-axis.
*/
SLGroup* SL3DSMeshFile::load(SLstring pathfilename,
                             SLint    dumpLevel,
                             SLbool   fixVerticesAmongSmoothgroups,
                             SLbool   rotate90DegAroundX)
{  
   // Check existance
   if (!SLFileSystem::fileExists(pathfilename))
   {  pathfilename = defaultPath + pathfilename;
      if (!SLFileSystem::fileExists(pathfilename))
      {  SLstring msg = "SL3DSMeshFile: File not found: " + pathfilename;
         SL_EXIT_MSG(msg.c_str());
      }
   }
   
   // take start time
   _timeMat = 0;
   _timeObj = 0;
   _timePP = 0;
   _timeStart = clock();
   
   // holds the main chunk header
   ChunkHeader main;
   
   _dumpLevel = dumpLevel;
   _fixVerticesAmongSmoothgroups = fixVerticesAmongSmoothgroups;
   _rotate90DegAroundX = rotate90DegAroundX;
   
   // create holding group
   _g = new SLGroup();
   
   // extract path & filename
   SLstring sPFN  = pathfilename;
   _sFilename = sPFN.substr(sPFN.find_last_of('/')+1);
   _sPathname = sPFN.substr(0, sPFN.find_last_of('/')+1);
   
   // Load the file
   _file = fopen((SLchar*)pathfilename.c_str(),"rb");
   if (_file==0) SL_EXIT_MSG("SL3DSMeshFile::load: Loading file failed.");

   // Make sure we are at the beginning
   fseek(_file, 0, SEEK_SET);

   // Load the Main Chunk's header
   Read3DSChunkHdr(main, 0);

   ////////////////////////////////////////////
   // Start Processing
   MAIN3DS_Read(1, main.len, ftell(_file));
   ////////////////////////////////////////////

   // Don't need the file anymore so close it
   fclose(_file); 
   SL_LOG("Time to postproc. : %5.2f sec.\n", (SLfloat)(_timePP)/CLOCKS_PER_SEC);  
   SL_LOG("Time to load 3DS  : %5.2f sec.\n", (SLfloat)(clock()-_timeStart)/CLOCKS_PER_SEC);
   return _g;
}

//-----------------------------------------------------------------------------




//-----------------------------------------------------------------------------
void SL3DSMeshFile::Read3DSChunkHdr(ChunkHeader &ch, SLint depth)
{  
   ch.id  = Read3DSushort();
   ch.len = Read3DSint();
   
   if (_dumpLevel)
   {  for (SLint i=0; i<depth; ++i) SL_LOG("  ");
      switch (ch.id)
      {  case MAIN3DS                       : SL_LOG("MAIN3DS --------------------------------------\n"); break;        
            case MAIN_VERS                  : SL_LOG("MAIN_VERS"); break;      
            case EDIT3DS                    : SL_LOG("EDIT3DS --------------------------------------\n"); break;        
               case EDIT_VERS               : SL_LOG("EDIT_VERS"); break;
               case EDIT_MATERIAL           : SL_LOG("EDIT_MATERIAL -------------------------------------\n"); break;
                  case MAT_NAME             : SL_LOG("MAT_NAME"); break;
                  case MAT_AMBIENT          : SL_LOG("MAT_AMBIENT\n"); break;
                  case MAT_DIFFUSE          : SL_LOG("MAT_DIFFUSE\n"); break;
                  case MAT_SPECULAR         : SL_LOG("MAT_SPECULAR\n"); break;
                  case MAT_SHIN1_PCT        : SL_LOG("MAT_SHIN1_PCT\n"); break;
                  case MAT_SHIN2_PCT        : SL_LOG("MAT_SHIN2_PCT\n"); break;
                  case MAT_TRANSP1_PCT      : SL_LOG("MAT_TRANSP1_PCT\n"); break;
                  case MAT_TRANSP2_PCT      : SL_LOG("MAT_TRANSP2_PCT\n"); break;
                  case MAT_REF_BLUR_PCT     : SL_LOG("MAT_REF_BLUR_PCT\n"); break;
                  case MAT_RENDER_TYPE      : SL_LOG("MAT_RENDER_TYPE\n"); break;
                  case MAT_SELF_ILLUM       : SL_LOG("MAT_SELF_ILLUM\n"); break;
                  case MAT_SELF_ILPCT       : SL_LOG("MAT_SELF_ILPCT\n"); break;
                  case MAT_WIRE_THICKNESS   : SL_LOG("MAT_WIRE_THICKNESS\n"); break;
                  case MAT_MAPTEX1          : SL_LOG("MAT_MAPTEX1\n"); break;
                     case MAP_NAME          : SL_LOG("MAP_NAME"); break;
                     case MAP_TILING        : SL_LOG("MAP_TILING\n"); break;
                     case MAP_BLUR_PERC     : SL_LOG("MAP_BLUR_PERC"); break;
                     case MAP_SCALEU        : SL_LOG("MAP_SCALEU"); break;
                     case MAP_SCALEV        : SL_LOG("MAP_SCALEV"); break;
                     case MAP_OFFSETU       : SL_LOG("MAP_OFFSETU"); break;
                     case MAP_OFFSETV       : SL_LOG("MAP_OFFSETV"); break;
                  case MAT_MAPTEX2          : SL_LOG("MAT_MAPTEX2\n"); break;
                  case MAT_MAPREFL          : SL_LOG("MAT_MAPREFL\n"); break;
                  case MAT_MAPBUMP          : SL_LOG("MAT_MAPBUMP\n"); break;
                  case MAT_MAPOPACITY       : SL_LOG("MAT_MAPOPACITY\n"); break;
                  case MAT_MAPSHININESS     : SL_LOG("MAT_MAPSHININESS\n"); break;
                  case MAT_MAPSPECULAR      : SL_LOG("MAT_MAPSPECULAR\n"); break;
                  case MAT_MAPSELFILLUM     : SL_LOG("MAT_MAPSELFILLUM\n"); break;

               case EDIT_MASTERSCALE        : SL_LOG("EDIT_MASTERSCALE"); break;
               case EDIT_CONFIG2            : SL_LOG("EDIT_CONFIG2 --------------------------------------\n"); break; 
               case EDIT_VIEW_P1            : SL_LOG("EDIT_VIEW_P1 --------------------------------------\n"); break;
               case EDIT_VIEW_P2            : SL_LOG("EDIT_VIEW_P2 --------------------------------------\n"); break;
               case EDIT_VIEW_P3            : SL_LOG("EDIT_VIEW_P3 --------------------------------------\n"); break;
               case EDIT_VIEW1              : SL_LOG("EDIT_VIEW1 ----------------------------------------\n"); break;
               case EDIT_BACKGR             : SL_LOG("EDIT_BACKGR ---------------------------------------\n"); break;
               case EDIT_AMBIENT            : SL_LOG("EDIT_AMBIENT --------------------------------------\n"); break;
                                          
               case EDIT_OBJECT             : SL_LOG("EDIT_OBJECT ---------------------------------------"); break; 
                  case OBJ_TRIMESH          : SL_LOG("OBJ_TRIMESH\n"); break;
                     case TRI_VERTEXL       : SL_LOG("TRI_VERTEXL"); break;
                     case TRI_VERTEXFLAGL   : SL_LOG("TRI_VERTEXFLAGL"); break;
                     case TRI_MAPCOORDS     : SL_LOG("TRI_MAPCOORDS"); break;
                     case TRI_LOCALMATRIX   : SL_LOG("TRI_LOCALMATRIX"); break;
                     case TRI_VISIBLE       : SL_LOG("TRI_VISIBLE\n"); break;
                     case TRI_FACEL         : SL_LOG("TRI_FACEL"); break;
                        case FACE_MATERIAL  : SL_LOG("FACE_MATERIAL"); break;
                        case FACE_SMOOTHGRP : SL_LOG("FACE_SMOOTHGRP:\n"); break;
                  case OBJ_LIGHT            : SL_LOG("OBJ_LIGHT\n"); break;
                     case LIT_OFF           : SL_LOG("LIT_OFF\n"); break;
                     case LIT_SPOT          : SL_LOG("LIT_SPOT\n"); break;
                     case LIT_UNKNWN01      : SL_LOG("LIT_UNKNWN01\n"); break;
                  case OBJ_CAMERA           : SL_LOG("OBJ_CAMERA\n"); break;
                     case CAM_UNKNWN01      : SL_LOG("CAM_UNKNWN01\n"); break;
                     case CAM_UNKNWN02      : SL_LOG("CAM_UNKNWN02\n"); break;  
        
               case KEYF3DS                 : SL_LOG("KEYF3DS --------------------------------------\n"); break;
                  case KEYF_FRAMES          : SL_LOG("KEYF_FRAMES \n");     break; 
                  case KEYF_UNKNOWN01       : SL_LOG("KEYF_UNKNOWN01 \n");  break;
                  case KEYF_UNKNOWN02       : SL_LOG("KEYF_UNKNOWN02 \n");  break; 
                  case KEYF_OBJDES          : SL_LOG("KEYF_OBJDES \n");     break; 
                     case KEYF_DUMMY_NAME   : SL_LOG("KEYF_DUMMY_NAME \n"); break; 
                     case KEYF_HIERARCHY    : SL_LOG("KEYF_HIERARCHY \n");  break; 
                     case KEYF_PIVOT        : SL_LOG("KEYF_PIVOT \n");      break;
                     case KEYF_UNKNOWN_03   : SL_LOG("KEYF_UNKNOWN_03 \n"); break;
                     case KEYF_UNKNOWN_04   : SL_LOG("KEYF_UNKNOWN_04 \n"); break;
                     case KEYF_UNKNOWN_05   : SL_LOG("KEYF_UNKNOWN_05 \n"); break;
                     case KEYF_UNKNOWN_06   : SL_LOG("KEYF_UNKNOWN_06 \n"); break;
                     case KEYF_UNKNOWN_07   : SL_LOG("KEYF_UNKNOWN_07 \n"); break;
                     case KEYF_UNKNOWN_08   : SL_LOG("KEYF_UNKNOWN_08 \n"); break;
         
         case COLOR_RGB       : SL_LOG("COLOR_RGB    "); break;
         case COLOR_TRU       : SL_LOG("COLOR_TRU    "); break;
         case COLOR_TRUG      : SL_LOG("COLOR_TRUG   "); break;
         case COLOR_RGBG      : SL_LOG("COLOR_RGBG   "); break;
         case PERCENT_SHORT   : SL_LOG("PERCENT_SHORT"); break;
         case PERCENT_FLOAT   : SL_LOG("PERCENT_FLOAT"); break;
         default              : SL_LOG("??? : %X\n", ch.id); break;
      }
   }
}

//-----------------------------------------------------------------------------
void SL3DSMeshFile::Read3DSString(SLstring &str, SLint len)
{  SLint c, i;
   SLchar* buf = new SLchar[len+1];
   
   for (i=0; (c=fgetc(_file)) != EOF && c!='\0'; i++) 
   {  if (i < len) buf[i] = c;
   }
   if (i < len) buf[i] = '\0';
   else buf[len-1] = '\0';
   
   str = buf;
   delete[] buf;
}
//-----------------------------------------------------------------------------




//-----------------------------------------------------------------------------
void SL3DSMeshFile::MAIN3DS_Read(SLint depth, SLint len, SLint iFile)
{  ChunkHeader h;
   while (ftell(_file) < (iFile + len - 6))
   {  Read3DSChunkHdr(h, depth);
      SLuint chunkStart = ftell(_file);
      switch (h.id)
      {  case MAIN_VERS: MAIN_VERS_Read(); break;
         case EDIT3DS:   EDIT3DS_Read(depth+1, h.len, chunkStart); break;
         case KEYF3DS:   KEYF3DS_Read(depth+1, h.len, chunkStart); break;
         default: if (_dumpLevel) UNKNOWN_Read(depth+1, h.len, chunkStart); break;
      }
      fseek(_file, chunkStart + h.len - 6, SEEK_SET);
   }
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void SL3DSMeshFile::MAIN_VERS_Read()
{  SLint version = Read3DSint();
   
   if (_dumpLevel==1) SL_LOG("\n");
   if (_dumpLevel>=2) SL_LOG(": (%i)\n", version);
}
//-----------------------------------------------------------------------------
void SL3DSMeshFile::EDIT3DS_Read(SLint depth, SLint len, SLint iFile)
{  ChunkHeader h;
   
   while (ftell(_file) < (iFile + len - 6))
   {  Read3DSChunkHdr(h, depth); 
      SLuint chunkStart = ftell(_file);  
      switch (h.id)
      {  case EDIT_VERS       : EDIT_VERS_Read       (); break;
         case EDIT_MASTERSCALE: EDIT_MASTERSCALE_Read(); break;
         case EDIT_MATERIAL   : EDIT_MATERIAL_Read   (depth+1, h.len, chunkStart); break;
         case EDIT_OBJECT     : EDIT_OBJECT_Read     (depth+1, h.len, chunkStart); break;
         default              : if (_dumpLevel) UNKNOWN_Read(depth+1, h.len, chunkStart); break;
      }
      fseek(_file, chunkStart + h.len - 6, SEEK_SET);
   }
}
//-----------------------------------------------------------------------------
void SL3DSMeshFile::KEYF3DS_Read(SLint depth, SLint len, SLint iFile)
{  ChunkHeader h;

   while (ftell(_file) < (iFile + len - 6))
   {  Read3DSChunkHdr(h, depth);
      SLuint chunkStart = ftell(_file); 
      if (_dumpLevel) UNKNOWN_Read (depth+1, h.len, chunkStart);
      fseek(_file, chunkStart + h.len - 6, SEEK_SET);
   }
}
//-----------------------------------------------------------------------------




//-----------------------------------------------------------------------------
void SL3DSMeshFile::EDIT_VERS_Read()
{  SLint version = Read3DSint();
   
   if (_dumpLevel==1) SL_LOG("\n");
   if (_dumpLevel>=2) SL_LOG(": (%i)\n", version);
}

//-----------------------------------------------------------------------------
void SL3DSMeshFile::EDIT_MASTERSCALE_Read()
{
   SLfloat masterScale = Read3DSfloat();
   
   if (_dumpLevel==1) SL_LOG("\n");
   if (_dumpLevel>=2) SL_LOG(": (%f)\n", masterScale);
}
//-----------------------------------------------------------------------------
void SL3DSMeshFile::EDIT_MATERIAL_Read(SLint depth, SLint len, SLint iFile)
{
   SLScene* s = SLScene::current;
   ChunkHeader h;
   
   if (_timeMat==0) _timeMat = clock();
   
   SLMaterial* material = new SLMaterial();
   s->materials().push_back(material);

   while (ftell(_file) < (iFile + len - 6))
   {  Read3DSChunkHdr(h, depth); 
      SLuint chunkStart = ftell(_file);  
      switch (h.id)
      {  case MAT_NAME        : MAT_NAME_Read        (s->materials().size()-1); break;
         case MAT_AMBIENT     : MAT_AMBIENT_Read     (depth+1, h.len, chunkStart, s->materials().size()-1); break;
         case MAT_DIFFUSE     : MAT_DIFFUSE_Read     (depth+1, h.len, chunkStart, s->materials().size()-1); break;
         case MAT_SPECULAR    : MAT_SPECULAR_Read    (depth+1, h.len, chunkStart, s->materials().size()-1); break;
         case MAT_SHIN1_PCT   : MAT_SHIN1_PCT_Read   (depth+1, h.len, chunkStart, s->materials().size()-1); break;
         case MAT_SHIN2_PCT   : MAT_SHIN2_PCT_Read   (depth+1, h.len, chunkStart, s->materials().size()-1); break;
         case MAT_TRANSP1_PCT : MAT_TRANSP1_PCT_Read (depth+1, h.len, chunkStart, s->materials().size()-1); break;
         case MAT_TRANSP2_PCT : MAT_TRANSP2_PCT_Read (depth+1, h.len, chunkStart, s->materials().size()-1); break;
         case MAT_MAPTEX1     : MAT_MAPTEX1_Read     (depth+1, h.len, chunkStart, s->materials().size()-1); break;
      }
      fseek(_file, chunkStart + h.len - 6, SEEK_SET);
   }
}
//-----------------------------------------------------------------------------
void SL3DSMeshFile::MAT_NAME_Read(SLint iMat)
{  
   SLScene* s = SLScene::current;
   
   // read the material name (max. 15 chars)
   SLstring sName;
   Read3DSString(sName, 80);
   
   s->materials()[iMat]->name(sName);
   
   if (_dumpLevel>=1) SL_LOG(": \"%s\"\n", s->materials()[iMat]->name().c_str());
}
//-----------------------------------------------------------------------------
void SL3DSMeshFile::MAT_AMBIENT_Read(SLint depth, SLint len, SLint iFile, SLint iMat)
{  
   SLScene* s = SLScene::current;
   ChunkHeader h;
   SLCol4f ambi;

   while (ftell(_file) < (iFile + len - 6))
   {  Read3DSChunkHdr(h, depth);
      SLuint chunkStart = ftell(_file);  
      switch (h.id)
      {  case COLOR_RGB : COLOR_RGB_Read(depth+1, h.len, chunkStart, ambi); s->materials()[iMat]->ambient(ambi); break;
         case COLOR_TRU : COLOR_TRU_Read(depth+1, h.len, chunkStart, ambi); s->materials()[iMat]->ambient(ambi); break;
         case COLOR_RGBG: COLOR_RGB_Read(depth+1, h.len, chunkStart, ambi); s->materials()[iMat]->ambient(ambi); break;
         case COLOR_TRUG: COLOR_TRU_Read(depth+1, h.len, chunkStart, ambi); s->materials()[iMat]->ambient(ambi); break;
         default: if (_dumpLevel) UNKNOWN_Read(depth+1, h.len, chunkStart);
      }
      fseek(_file, chunkStart + h.len - 6, SEEK_SET);
   }
}
//-----------------------------------------------------------------------------
void SL3DSMeshFile::MAT_DIFFUSE_Read(SLint depth, SLint len, SLint iFile, SLint iMat)
{  
   SLScene* s = SLScene::current;
   ChunkHeader h;
   SLCol4f diff;

   while (ftell(_file) < (iFile + len - 6))
   {  Read3DSChunkHdr(h, depth);
      SLuint chunkStart = ftell(_file); 
      switch (h.id)
      {  case COLOR_RGB : COLOR_RGB_Read(depth+1, h.len, chunkStart, diff); s->materials()[iMat]->diffuse(diff); break;
         case COLOR_TRU : COLOR_TRU_Read(depth+1, h.len, chunkStart, diff); s->materials()[iMat]->diffuse(diff); break;
         case COLOR_RGBG: COLOR_RGB_Read(depth+1, h.len, chunkStart, diff); s->materials()[iMat]->diffuse(diff); break;
         case COLOR_TRUG: COLOR_TRU_Read(depth+1, h.len, chunkStart, diff); s->materials()[iMat]->diffuse(diff); break;
         default: if (_dumpLevel) UNKNOWN_Read(depth+1, h.len, chunkStart);
      }
      fseek(_file, chunkStart + h.len - 6, SEEK_SET);
   }
}
//-----------------------------------------------------------------------------
void SL3DSMeshFile::MAT_SPECULAR_Read(SLint depth, SLint len, SLint iFile, SLint iMat)
{  
   SLScene* s = SLScene::current;
   ChunkHeader h;
   SLCol4f spec;

   while (ftell(_file) < (iFile + len - 6))
   {  Read3DSChunkHdr(h, depth);
      SLuint chunkStart = ftell(_file); 
      switch (h.id)
      {  case COLOR_RGB : COLOR_RGB_Read(depth+1, h.len, chunkStart, spec); s->materials()[iMat]->specular(spec); break;
         case COLOR_TRU : COLOR_TRU_Read(depth+1, h.len, chunkStart, spec); s->materials()[iMat]->specular(spec); break;
         case COLOR_RGBG: COLOR_RGB_Read(depth+1, h.len, chunkStart, spec); s->materials()[iMat]->specular(spec); break;
         case COLOR_TRUG: COLOR_TRU_Read(depth+1, h.len, chunkStart, spec); s->materials()[iMat]->specular(spec); break;
         default: if (_dumpLevel) UNKNOWN_Read(depth+1, h.len, chunkStart);
      }
      fseek(_file, chunkStart + h.len - 6, SEEK_SET);
   }
}
//-----------------------------------------------------------------------------
void SL3DSMeshFile::MAT_MAPTEX1_Read(SLint depth, SLint len, SLint iFile, SLint iMat)
{  
   SLScene* s = SLScene::current;
   ChunkHeader h;

   while (ftell(_file) < (iFile + len - 6))
   {  Read3DSChunkHdr(h, depth); 
      SLuint chunkStart = ftell(_file); 
      SLfloat f;
      SLVGLTexture& textures = s->materials()[iMat]->textures();
      
      switch (h.id)
      {  case PERCENT_SHORT:  SLshort percent;
                              PERCENT_SHORT_Read(depth+1, h.len, chunkStart, percent); 
                              break;
         case MAP_NAME     :  MAP_NAME_Read(iMat);
                              break;
         case MAP_TILING   :  MAP_PARAMS_Read   (depth+1, h.len, chunkStart, iMat);
                              break;
         case MAP_BLUR_PERC:  PERCENT_FLOAT_Read(depth+1, h.len, chunkStart, f); 
                              break;
         case MAP_SCALEU   :  PERCENT_FLOAT_Read(depth+1, h.len, chunkStart, f); 
                              if (textures.size()) textures[0]->scaleS(f);
                              break;
         case MAP_SCALEV   :  PERCENT_FLOAT_Read(depth+1, h.len, chunkStart, f);
                              if (textures.size()) textures[0]->scaleT(f);
                              break;
         case MAP_OFFSETU  :  PERCENT_FLOAT_Read(depth+1, h.len, chunkStart, f);
                              if (textures.size()) textures[0]->translateS(f);
                              break;
         case MAP_OFFSETV  :  PERCENT_FLOAT_Read(depth+1, h.len, chunkStart, f);
                              if (textures.size()) textures[0]->translateT(f);
                              break;
         default: if (_dumpLevel) UNKNOWN_Read(depth+1, h.len, chunkStart); 
      }
      fseek(_file, chunkStart + h.len - 6, SEEK_SET);
   }
}
//-----------------------------------------------------------------------------
void SL3DSMeshFile::MAP_NAME_Read(SLint iMat)
{  
   SLScene* s = SLScene::current;
   SLstring sFilename;
   Read3DSString(sFilename, 80);

   // Load the name and indicate that the material has a texture   
   SLstring sPathFilename = _sPathname + sFilename;
   
   if (SLFileSystem::fileExists(sPathFilename))
   {  
      if (_dumpLevel>=1) SL_LOG(": \"%s\"\n", sFilename.c_str());
      
      SLGLTexture* tex = new SLGLTexture(sPathFilename);
      
      // add it to the texture vector of the scene
      s->textures().push_back(tex);
      
      // add it to the material
      s->materials()[iMat]->textures().push_back(tex);
   } else
   {  SL_LOG("Error: Texture file not found or access denied: %s\n", sPathFilename.c_str());
   }
     
}
//-----------------------------------------------------------------------------
void SL3DSMeshFile::MAP_PARAMS_Read(SLint depth, SLint len, SLint iFile, SLint iMat)
{  (void)depth;
   (void)len;
   (void)iFile;
   (void)iMat;
}
//-----------------------------------------------------------------------------
void SL3DSMeshFile::MAT_SHIN1_PCT_Read(SLint depth, SLint len, SLint iFile, SLint iMat)
{  
   SLScene* s = SLScene::current;
   ChunkHeader h;

   while (ftell(_file) < (iFile + len - 6))
   {  Read3DSChunkHdr(h, depth);
      SLuint chunkStart = ftell(_file); 
      switch (h.id)
      {  case PERCENT_SHORT: 
            SLshort sh;
            PERCENT_SHORT_Read(depth+1, h.len, chunkStart, sh); 
            s->materials()[iMat]->shininess((SLfloat)sh*1.28f);
            break;
         case PERCENT_FLOAT: 
            SLfloat fl;
            PERCENT_FLOAT_Read(depth+1, h.len, chunkStart, fl);
            s->materials()[iMat]->shininess(fl*128.0f);
            break;
         default: if (_dumpLevel) UNKNOWN_Read(depth+1, h.len, chunkStart); 
      }
      fseek(_file, chunkStart + h.len - 6, SEEK_SET);
   }
}
//-----------------------------------------------------------------------------
void SL3DSMeshFile::MAT_SHIN2_PCT_Read(SLint depth, SLint len, SLint iFile, SLint iMat)
{
   (void)iMat;
   ChunkHeader h;

   while (ftell(_file) < (iFile + len - 6))
   {  Read3DSChunkHdr(h, depth);
      SLuint chunkStart = ftell(_file); 
      switch (h.id)
      {  case PERCENT_SHORT: 
            SLshort sh;
            PERCENT_SHORT_Read(depth+1, h.len, chunkStart, sh);  //Not used
            break;
         case PERCENT_FLOAT: 
            SLfloat fl;
            PERCENT_FLOAT_Read(depth+1, h.len, chunkStart, fl);  //Not used
            break;
         default: if (_dumpLevel) UNKNOWN_Read(depth+1, h.len, chunkStart); 
      }
      fseek(_file, chunkStart + h.len - 6, SEEK_SET);
   }
}
//-----------------------------------------------------------------------------
void SL3DSMeshFile::MAT_TRANSP1_PCT_Read(SLint depth, SLint len, SLint iFile, SLint iMat)
{
   SLScene* s = SLScene::current;
   (void)iMat;
   ChunkHeader h;
   SLshort     sh;
   SLfloat     iFV;

   while (ftell(_file) < (iFile + len - 6))
   {  Read3DSChunkHdr(h, depth);
      SLuint chunkStart = ftell(_file); 
      switch (h.id)
      {  case PERCENT_SHORT: 
            PERCENT_SHORT_Read(depth+1, h.len, chunkStart, sh);
            s->materials()[iMat]->kt((SLfloat)sh/100.0f);
            break;
         case PERCENT_FLOAT: 
            PERCENT_FLOAT_Read(depth+1, h.len, chunkStart, iFV);
            s->materials()[iMat]->kt(iFV);
            break;
         default: if (_dumpLevel) UNKNOWN_Read (depth+1, h.len, chunkStart);
      }
      fseek(_file, chunkStart + h.len - 6, SEEK_SET);
   }
}
//-----------------------------------------------------------------------------
void SL3DSMeshFile::MAT_TRANSP2_PCT_Read(SLint depth, SLint len, SLint iFile, SLint iMat)
{
   (void)iMat;
   ChunkHeader h;

   while (ftell(_file) < (iFile + len - 6))
   {  Read3DSChunkHdr(h, depth);
      SLuint chunkStart = ftell(_file); 
      switch (h.id)
      {  case PERCENT_SHORT: 
            SLshort sh;
            PERCENT_SHORT_Read(depth+1, h.len, chunkStart, sh);  //Not used
            break;
         case PERCENT_FLOAT: 
            SLfloat fl;
            PERCENT_FLOAT_Read(depth+1, h.len, chunkStart, fl);  //Not used
            break;
         default: if (_dumpLevel) UNKNOWN_Read(depth+1, h.len, chunkStart); 
      }
      fseek(_file, chunkStart + h.len - 6, SEEK_SET);
   }
}
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
void SL3DSMeshFile::EDIT_OBJECT_Read(SLint depth, SLint len, SLint iFile)
{  
   ChunkHeader h;

   if (_timeObj==0) 
   {  SL_LOG("Time to load mat. : %5.2f sec.\n", 
             (SLfloat)(clock()-_timeMat)/CLOCKS_PER_SEC);
      _timeObj = clock();
   }
   
   // Load the object's name
   SLstring sObjName;
   Read3DSString(sObjName, 80);
   if (_dumpLevel>=1) SL_LOG(": \"%s\"\n", sObjName.c_str());
   
   //if (sObjName!="Lastil10" && 
   //    sObjName!="Lastil05" && 
   //    sObjName!="tampon" && 
   //    sObjName!="Plane47" && 
   //    sObjName!="Plane50") return;
   
   while (ftell(_file) < (iFile + len - 6))
   {  Read3DSChunkHdr(h, depth);
      SLuint chunkStart = ftell(_file);
      switch (h.id)
      {  case OBJ_TRIMESH: OBJ_TRIMESH_Read(depth+1, h.len, chunkStart, sObjName); break;
         case OBJ_LIGHT  : OBJ_LIGHT_Read  (depth+1, h.len, chunkStart, sObjName); break;
         case OBJ_CAMERA : OBJ_CAMERA_Read (depth+1, h.len, chunkStart, sObjName); break;
         default: if (_dumpLevel) UNKNOWN_Read(depth+1, h.len, chunkStart); 
      }
      fseek(_file, chunkStart + h.len - 6, SEEK_SET);
   }
}
//-----------------------------------------------------------------------------
void SL3DSMeshFile::OBJ_TRIMESH_Read(SLint depth, SLint len, SLint iFile, SLstring objName)
{
   SLScene* s = SLScene::current;
   (void) objName;
   ChunkHeader h;
      
   // create mesh object and append it to the mesh group
   SL3DSMesh* mesh = new SL3DSMesh(_g, objName);
   
   while (ftell(_file) < (iFile + len - 6))
   {  Read3DSChunkHdr(h, depth); 
      SLuint chunkStart = ftell(_file);
      switch (h.id)
      {  case TRI_VERTEXL    : TRI_VERTEXL_Read    (depth+1, h.len, chunkStart, mesh); break;
         case TRI_VERTEXFLAGL: TRI_VERTEXFLAGL_Read(depth+1, mesh); break;
         case TRI_LOCALMATRIX: TRI_LOCALMATRIX_Read(depth+1, h.len, chunkStart, mesh); break;
         case TRI_VISIBLE    : TRI_VISIBLE_Read    (depth+1, h.len, chunkStart, mesh); break;
         case TRI_MAPCOORDS  : TRI_MAPCOORDS_Read  (depth+1, h.len, chunkStart, mesh); break;
         case TRI_FACEL      : TRI_FACEL_Read      (depth+1, h.len, chunkStart, mesh); break;
         default: if (_dumpLevel) UNKNOWN_Read(depth+1, h.len, chunkStart); break;
      }
      fseek(_file, chunkStart + h.len - 6, SEEK_SET);
   }
   
   ////////////////////////////////////////////////////////////////////////////
   // Post Processing
   // Reading the 3DS triangle mesh is finished here.
   ////////////////////////////////////////////////////////////////////////////
   
   clock_t startPostProcess = clock();
   
   if (mesh->numV > 0)
   {
      // if no smoothgroup has been defined define default smoothgroup 
      if (mesh->smoF.size() == 0)
      {  
         // add the default smoothgroup 0
         mesh->smoF.push_back(new SL3DSSmoothFaces(0));
         
         // set the smoothgroup per face
         for (SLuint iF=0; iF<mesh->numSMF; ++iF) mesh->SMF[iF].iSF = 0;
         mesh->smoF[0]->numF = mesh->numSMF;
         
         // build up smoothgroup data
         mesh->smoF[0]->F = new SLushort[mesh->smoF[0]->numF];
         mesh->smoF[0]->I = new SLushort[mesh->smoF[0]->numF*3];
         
         // get them from the loaded face vertex indexes
         SLuint cur_iFV = 0;
         for (SLuint iF=0; iF < mesh->numSMF; ++iF)
         {  mesh->smoF[0]->F[iF]  = iF;
            mesh->smoF[0]->I[cur_iFV] = _iFV[cur_iFV]; cur_iFV++;
            mesh->smoF[0]->I[cur_iFV] = _iFV[cur_iFV]; cur_iFV++;
            mesh->smoF[0]->I[cur_iFV] = _iFV[cur_iFV]; cur_iFV++;
         }
      }
   
      // if no material and matFaces has been defined define default material 
      if (mesh->matF.size() == 0)
      {  
         // create default gray appearance in the meshgroups array
         //s->materials().push_back(new SLMaterial((SLchar*)"Default gray", SLCol4f(0.5f,0.5f,0.5f), SLCol4f(1,1,1), 80));
         
         // add the default matFaces group
         //mesh->matF.push_back(new SL3DSMatFaces(s->materials().size()-1));
         mesh->matF.push_back(new SL3DSMatFaces());
         
         // set the matFace index 0 for all faces
         for (SLuint iF=0; iF<mesh->numSMF; ++iF) mesh->SMF[iF].iMF = 0;
         mesh->matF[0]->numF = mesh->numSMF;
         
         // build up smoothgroup data
         mesh->matF[0]->F = new SLushort[mesh->matF[0]->numF];
         mesh->matF[0]->I = new SLushort[mesh->matF[0]->numF*3];
         
         // get them from the loaded face vertex indexes
         SLuint cur_iFV = 0;
         for (SLuint iF=0; iF < mesh->numSMF; ++iF)
         {  mesh->matF[0]->F[iF]  = iF;
            mesh->matF[0]->I[cur_iFV] = _iFV[cur_iFV]; cur_iFV++;
            mesh->matF[0]->I[cur_iFV] = _iFV[cur_iFV]; cur_iFV++;
            mesh->matF[0]->I[cur_iFV] = _iFV[cur_iFV]; cur_iFV++;
         }
      }

      // release temp FV array
      delete [] _iFV;
      _iFV = 0;
   
      if (_fixVerticesAmongSmoothgroups) 
         mesh->fixVertexRedundancy(depth, _dumpLevel);
      //mesh->applyInverseLocalMatrix();
      mesh->calcNormals();
   
      ////////////////////////////////////////////////////////////////////////////
      // Finally create scenegraph mesh instance
      SLMesh* mesh2 = new SLMesh(mesh->name);
      
      //copy arrays
      mesh2->numV = mesh->numV;
      
      mesh2->P = new SLVec3f[mesh2->numV]; 
      memcpy(mesh2->P, mesh->P, mesh->numV*sizeof(SLVec3f));
      
      mesh2->N = new SLVec3f[mesh2->numV]; 
      memcpy(mesh2->N, mesh->N, mesh->numV*sizeof(SLVec3f));
      
      if (mesh->Tc)
      {  mesh2->Tc = new SLVec2f[mesh2->numV]; 
         memcpy(mesh2->Tc, mesh->Tc, mesh->numV*sizeof(SLVec2f));
      }
      
      mesh2->numF = mesh->numSMF;
      mesh2->F = new SLFace[mesh2->numF];
      
      // count MatFaces that contain faces
      SLuint numM2 = 0;
      for (SLuint i=0; i<mesh->matF.size(); ++i)
         if (mesh->matF[i]->numF > 0) numM2++;
      
      // copy only matFaces that contain faces
      mesh2->numM = numM2;
      mesh2->M = new SLMatFaces[numM2];
         
      SLuint startF = 0, i2 = 0;
      for (SLuint i=0; i<mesh->matF.size(); ++i)
      {  
         if (mesh->matF[i]->numF > 0)
         {  if (mesh->matF[i]->iMat >= 0)
                 mesh2->M[i2].mat = s->materials()[mesh->matF[i]->iMat];
            else mesh2->M[i2].mat = 0;
            mesh2->M[i2].numF = mesh->matF[i]->numF;
            mesh2->M[i2].startF = startF;
            memcpy(mesh2->F+startF, 
                   mesh->matF[i]->I, 
                   mesh2->M[i2].numF*sizeof(SLFace));
            startF += mesh2->M[i2].numF;
            i2++;
         }
      }
      _g->addNode(mesh2);
      
      _timePP += clock()-startPostProcess; 
      ////////////////////////////////////////////////////////////////////////////
   }  // mesh->numV > 0
   
   // finally delete loader mesh
   delete mesh;
}
//-----------------------------------------------------------------------------
void SL3DSMeshFile::OBJ_LIGHT_Read(SLint depth, SLint len, SLint iFile, SLstring objName)
{
   (void)objName;
   ChunkHeader h;

   while (ftell(_file) < (iFile + len - 6))
   {  Read3DSChunkHdr(h, depth);
      SLuint chunkStart = ftell(_file);
      if (_dumpLevel) UNKNOWN_Read(depth+1, h.len, chunkStart); break;
      fseek(_file, chunkStart + h.len - 6, SEEK_SET);
   }
}
//-----------------------------------------------------------------------------
void SL3DSMeshFile::OBJ_CAMERA_Read(SLint depth, SLint len, SLint iFile, SLstring objName)
{
   (void)len;
   (void)iFile;
   (void)objName;
   ChunkHeader h;

   while (ftell(_file) < (iFile + len - 6))
   {  Read3DSChunkHdr(h, depth);
      SLuint chunkStart = ftell(_file); 
      if (_dumpLevel) UNKNOWN_Read(depth+1, h.len, chunkStart); break;
      fseek(_file, chunkStart + h.len - 6, SEEK_SET);
   }
}
//-----------------------------------------------------------------------------
void SL3DSMeshFile::TRI_VERTEXL_Read(SLint depth, SLint len, SLint iFile, SL3DSMesh* mesh)
{
   (void)len;
   (void)iFile;
   // Read the number of vertices of the object
   mesh->numV = Read3DSushort();
   
   if (_dumpLevel==1) SL_LOG("\n");
   if (_dumpLevel>=2) SL_LOG(": (%d vertices)\n", mesh->numV);

   // Allocate arrays for the vertices and normals
   mesh->P = new SLVec3f[mesh->numV];

   // Loop through vertices and load single floats
   // (Loading all floats with fread at once is not remarkable faster!)
   for (SLuint i = 0; i < mesh->numV; ++i)
   {  mesh->P[i].x   =  Read3DSfloat();
      if (_rotate90DegAroundX) 
      {  mesh->P[i].z = -Read3DSfloat();
         mesh->P[i].y =  Read3DSfloat();
         
      } else
      {  mesh->P[i].y =  Read3DSfloat();
         mesh->P[i].z =  Read3DSfloat();
      }
      
      if (_dumpLevel>=3) 
      {  for (SLint tab=0; tab<depth; ++tab) SL_LOG("  ");
         SL_LOG("P[%d].xyz=(%7.3f,%7.3f,%7.3f)\n", i, mesh->P[i].x,
                                                      mesh->P[i].y,
                                                      mesh->P[i].z);
      }
   }
}
//-----------------------------------------------------------------------------
void SL3DSMeshFile::TRI_VERTEXFLAGL_Read(SLint depth, SL3DSMesh* mesh)
{
   (void)mesh;
   // Read the number of vertex flags
   SLushort numFlags = Read3DSushort();
   
   if (_dumpLevel==1) SL_LOG("\n");
   if (_dumpLevel>=2) SL_LOG(": (%d vertex flags)\n", numFlags);

   // Read the vertexflags
   for (SLuint i = 0; i < numFlags; ++i)
   {  SLuint flags   =  Read3DSushort();
      
      if (_dumpLevel>=3) 
      {  for (SLint tab=0; tab<depth; ++tab) SL_LOG("  ");
         SL_LOG("flags=%u\n", flags);
      }
   }
}
//-----------------------------------------------------------------------------
void SL3DSMeshFile::TRI_MAPCOORDS_Read(SLint depth, SLint len, SLint iFile, SL3DSMesh* mesh)
{
   (void)len;
   (void)iFile;

   // Read the number of coordinates
   SLuint numT = Read3DSushort();
   
   if (_dumpLevel==1) SL_LOG("\n");
   if (_dumpLevel>=2) SL_LOG(": (%d tex coords)\n", numT);
   
   if (numT>0 && mesh->numV>0 && numT!=mesh->numV) 
      SL_LOG("The no. of vertices and the no. of tex. coords are unqual!\n");

   // Allocate an array to hold the texture coordinates
   mesh->Tc = new SLVec2f[numT];

   // Read teh texture coordiantes into the array
   for (SLuint i = 0; i < numT; ++i)
   {  mesh->Tc[i].x = Read3DSfloat();
      mesh->Tc[i].y = Read3DSfloat();
      
      if (_dumpLevel>=3) 
      {  for (SLint tab=0; tab<depth; ++tab) SL_LOG("  ");
         SL_LOG("Tc[%d].uv=(%6.3f,%6.3f)\n", i, mesh->Tc[i].x, mesh->Tc[i].y);
      }
   }
}
//-----------------------------------------------------------------------------
void SL3DSMeshFile::TRI_LOCALMATRIX_Read(SLint depth, SLint len, SLint iFile, SL3DSMesh* mesh)
{
   (void)len;
   (void)iFile;

   SLfloat m[16];
   m[0] = Read3DSfloat(); m[1] = Read3DSfloat(); m[2] = Read3DSfloat(); m[3] = 0.0f;
   m[4] = Read3DSfloat(); m[5] = Read3DSfloat(); m[6] = Read3DSfloat(); m[7] = 0.0f;
   m[8] = Read3DSfloat(); m[9] = Read3DSfloat(); m[10]= Read3DSfloat(); m[11]= 0.0f;
   m[12]= Read3DSfloat(); m[13]= Read3DSfloat(); m[14]= Read3DSfloat(); m[15]= 1.0f;
   SLMat4f mat(m);
   
   //Ignore the matrix from 3DS
   mesh->om.setMatrix(mat); 
   
   //if (_dumpLevel==1) SL_LOG("\n");
   if (_dumpLevel>=1) 
   {  SL_LOG("\n");
      for (SLint tab=0; tab<depth; ++tab) SL_LOG("  ");
      SL_LOG("% 6.3f % 6.3f % 6.3f % 6.3f\n", m[0],m[4],m[8] ,m[12]);
      for (SLint tab=0; tab<depth; ++tab) SL_LOG("  ");
      SL_LOG("% 6.3f % 6.3f % 6.3f % 6.3f\n", m[1],m[5],m[9] ,m[13]);
      for (SLint tab=0; tab<depth; ++tab) SL_LOG("  ");
      SL_LOG("% 6.3f % 6.3f % 6.3f % 6.3f\n", m[2],m[6],m[10],m[14]);
      for (SLint tab=0; tab<depth; ++tab) SL_LOG("  ");
      SL_LOG("% 6.3f % 6.3f % 6.3f % 6.3f\n", m[3],m[7],m[11],m[15]);
   }
}
//-----------------------------------------------------------------------------
void SL3DSMeshFile::TRI_VISIBLE_Read(SLint depth, SLint len, SLint iFile, SL3DSMesh* mesh)
{
   (void)depth;
   (void)len;
   (void)iFile;
   (void)mesh;
}
//-----------------------------------------------------------------------------
void SL3DSMeshFile::TRI_FACEL_Read(SLint depth, SLint len, SLint iFile, SL3DSMesh* mesh)
{  ChunkHeader h;
   SLushort    flags;
   SLlong      subs;

   // Read the number of faces
   mesh->numSMF = Read3DSushort();
   _numiFV = mesh->numSMF * 3;
   
   if (_dumpLevel==1) SL_LOG("\n");
   if (_dumpLevel>=2) SL_LOG(": (%d faces)\n", mesh->numSMF);

   // Allocate arrays to hold the faces & face vertex indices
   mesh->SMF = new SL3DSFace[mesh->numSMF];
   _iFV = new SLuint[_numiFV];

   // Read the faces into the array
   for (SLuint i = 0; i < _numiFV; i+=3)
   {  _iFV[i]   = Read3DSushort();
      _iFV[i+1] = Read3DSushort();
      _iFV[i+2] = Read3DSushort();
      flags     = Read3DSushort();
      
      if (_dumpLevel>=3) 
      {  for (SLint tab=0; tab<depth; ++tab) SL_LOG("  ");
         SL_LOG("I[%d].abc=(%d,%d,%d)\n", i/3, _iFV[i],_iFV[i+1],_iFV[i+2]);
      }
   }

   // Store our current file position
   subs = ftell(_file);

   // Check to see how many materials the faces are split into
   while (ftell(_file) < (iFile + len - 6))
   {  Read3DSChunkHdr(h, depth); 
      SLuint chunkStart = ftell(_file);
      switch (h.id)
      {  case FACE_MATERIAL : FACE_MATERIAL_Read (depth+1, h.len, chunkStart, mesh); break;
         case FACE_SMOOTHGRP: FACE_SMOOTHGRP_Read(depth+1, h.len, chunkStart, mesh); break;
         default: if (_dumpLevel) UNKNOWN_Read(depth+1, h.len, chunkStart); break;
      }
      fseek(_file, chunkStart + h.len - 6, SEEK_SET);
   }
}

//-----------------------------------------------------------------------------
void SL3DSMeshFile::FACE_MATERIAL_Read(SLint depth, SLint len, SLint iFile, SL3DSMesh* mesh)
{
   (void)len;
   (void)iFile;

   SLScene* s = SLScene::current;
   SLstring sMatName;      // The material's name
   SLushort iFace;         // Holds the faces as they are read
   SLuint   iMat;          // An index to the _materials array for this material
   SLuint   iMatF;         // index of the last matF
   SLuint   numFaces;      // No. of faces with this material
   
   // Read the material's name
   Read3DSString(sMatName,80);
   numFaces = Read3DSushort();
   if (_dumpLevel>=1) SL_LOG(": \"%s\" (numFaces: %d)\n", sMatName.c_str(), numFaces);
   
   // add matF
   mesh->matF.push_back(new SL3DSMatFaces);
   iMatF = mesh->matF.size()-1;

   // Find the material's index in the _materials array
   for (iMat = 0; iMat < s->materials().size(); iMat++)
   {  if (sMatName == s->materials()[iMat]->name()) break;
   }
   
   // Store this value for later so that we can find the material
   mesh->matF[iMatF]->iMat = iMat;

   // Read the number of faces associated with this material
   mesh->matF[iMatF]->numF = numFaces;
 
   if (numFaces > 0) 
   {  // Allocate an array to hold the list of face vertex indexes associated with this material
      mesh->matF[iMatF]->F = new SLushort[mesh->matF[iMatF]->numF];
      mesh->matF[iMatF]->I = new SLushort[mesh->matF[iMatF]->numF * 3];

      // Read the faces into the array      
      if (_dumpLevel>=3) 
      {  for (SLint tab=0; tab<depth; ++tab) SL_LOG("  ");
         SL_LOG("F=");
      }
      for (SLuint i = 0; i < mesh->matF[iMatF]->numF; ++i)
      {  iFace = Read3DSushort();
         mesh->SMF[iFace].iMF = iMatF;
         
         if (_dumpLevel>=3) SL_LOG("%d,", iFace);
         
         mesh->matF[iMatF]->F[i] = iFace;
         
         // Add the face's vertex indexes to the I array
         SLuint iiFV = i*3, iFaceV = iFace*3;
         mesh->matF[iMatF]->I[iiFV  ] = _iFV[iFaceV  ];
         mesh->matF[iMatF]->I[iiFV+1] = _iFV[iFaceV+1];
         mesh->matF[iMatF]->I[iiFV+2] = _iFV[iFaceV+2];
      }  
      if (_dumpLevel>=3) SL_LOG("\n");
   }
}
//-----------------------------------------------------------------------------
void SL3DSMeshFile::FACE_SMOOTHGRP_Read(SLint depth, SLint len, SLint iFile, SL3DSMesh* mesh)
{
   (void)len;
   (void)iFile;

   // allocate temp. array smoothGroup of faces
   SLuint* faceSmoothGroup = new SLuint[_numiFV/3];
   
   // read the smoothgroup per face
   for (SLuint iF=0; iF<mesh->numSMF; ++iF) 
   {  faceSmoothGroup[iF] = Read3DSuint();
      
      // dump
      if (_dumpLevel>=3) 
      {  for (SLint tab=0; tab<depth; ++tab) SL_LOG("  ");
         SL_LOG("Face[%d]=\t",iF);
         for (SLint m=0; m<32; ++m)
         {  SL_LOG("%d", faceSmoothGroup[iF]>>m & 1);
         }
         SL_LOG("\n");
      }
      
      // add the smoothgroup to the smoothgroup vector if it doesn't exit yet
      SLuint iSF = 0;
      for (iSF=0; iSF<mesh->smoF.size(); ++iSF)
      {  if (mesh->smoF[iSF]->ID == faceSmoothGroup[iF]) break;
      }
      if (iSF == mesh->smoF.size())
      {  mesh->smoF.push_back(new SL3DSSmoothFaces(faceSmoothGroup[iF]));
      }
      mesh->SMF[iF].iSF = iSF;
     
      // increment the counter of the face vertex indexes
      mesh->smoF[iSF]->numF++;
   }
   
   // build up smoothgroup data
   for (SLuint iSF=0; iSF<mesh->smoF.size(); ++iSF)
   {  // allocate the smoothgroups face vertex indexes
      mesh->smoF[iSF]->F = new SLushort[mesh->smoF[iSF]->numF];
      mesh->smoF[iSF]->I = new SLushort[mesh->smoF[iSF]->numF*3];
      
      // get them from the loaded face vertex indexes
      SLuint cur_iFV = 0;
      for (SLuint iF=0; iF < _numiFV/3; ++iF)
      {  if (faceSmoothGroup[iF] == mesh->smoF[iSF]->ID)
         {  SLuint iiFV = iF * 3;
            mesh->smoF[iSF]->F[cur_iFV/3]  = iF;
            mesh->smoF[iSF]->I[cur_iFV++] = _iFV[iiFV++];
            mesh->smoF[iSF]->I[cur_iFV++] = _iFV[iiFV++]; 
            mesh->smoF[iSF]->I[cur_iFV++] = _iFV[iiFV]; 
         }
      }
   }
   
   // dump smoothgroup summary
   if (_dumpLevel >= 2)
   {  for (SLuint iSF=0; iSF<mesh->smoF.size(); ++iSF)
      {  for (SLint tab=0; tab<depth; ++tab) SL_LOG("  ");
         SL_LOG("smoF[%u]:I=", iSF);
         for (SLuint iFV=0; iFV< (SLuint)(mesh->smoF[iSF]->numF*3); ++iFV)
         {  SL_LOG("%u,", mesh->smoF[iSF]->I[iFV]);
         }
         SL_LOG("  iF=%u", iSF);
         for (SLuint iF=0; iF<mesh->smoF[iSF]->numF; ++iF)
         {  SL_LOG("%u,", mesh->smoF[iSF]->F[iF]);
         }
         SL_LOG("\n");
      }
   }
      
   delete[] faceSmoothGroup;
}
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
void SL3DSMeshFile::PERCENT_SHORT_Read(SLint depth, SLint len, SLint iFile, SLshort &percent)
{
   (void)depth;
   (void)len;
   (void)iFile;

   percent = Read3DSshort();
   
   if (_dumpLevel==1) SL_LOG("\n");
   if (_dumpLevel>=2) SL_LOG(": (%d)\n", percent);
}
//-----------------------------------------------------------------------------
void SL3DSMeshFile::PERCENT_FLOAT_Read(SLint depth, SLint len, SLint iFile, SLfloat &percent)
{
   (void)depth;
   (void)len;
   (void)iFile;

   percent = Read3DSfloat();
   
   if (_dumpLevel==1) SL_LOG("\n");
   if (_dumpLevel>=2) SL_LOG(": (%5.3f)\n", percent);
}
//-----------------------------------------------------------------------------
void SL3DSMeshFile::COLOR_RGB_Read(SLint depth, SLint len, SLint iFile, SLCol4f &color)
{
   (void)depth;
   (void)len;
   (void)iFile;

   color.x = Read3DSfloat();
   color.y = Read3DSfloat();
   color.z = Read3DSfloat();
   color.w = 1.0f;
   
   if (_dumpLevel==1) SL_LOG("\n");
   if (_dumpLevel>=2) SL_LOG(": rgba=(%3.1f,%3.1f,%3.1f,%3.1f)\n", 
                             color.x, color.y, color.z, color.w);
}
//-----------------------------------------------------------------------------
void SL3DSMeshFile::COLOR_TRU_Read(SLint depth, SLint len, SLint iFile, SLCol4f &color)
{
   (void)depth;
   (void)len;
   (void)iFile;

   SLuchar r ,g, b;
   
   fread(&r,sizeof(r),1,_file);
   fread(&g,sizeof(g),1,_file);
   fread(&b,sizeof(b),1,_file);

   color.x = (SLfloat)r/255.0f;
   color.y = (SLfloat)g/255.0f;
   color.z = (SLfloat)b/255.0f;
   color.w = 1.0f;
   
   if (_dumpLevel==1) SL_LOG("\n");
   if (_dumpLevel>=2) SL_LOG(": rgba=(%d,%d,%d,%d)\n", r, g, b, 255);
}
//-----------------------------------------------------------------------------
void SL3DSMeshFile::UNKNOWN_Read(SLint depth, SLint len, SLint iFile)
{  ChunkHeader h;

   while (ftell(_file) < (iFile + len - 6))
   {  Read3DSChunkHdr(h, depth); 
      SLuint chunkStart = ftell(_file);
      UNKNOWN_Read(depth+1, h.len, chunkStart); break;
      fseek(_file, chunkStart + h.len - 6, SEEK_SET);
   }
}
//-----------------------------------------------------------------------------
