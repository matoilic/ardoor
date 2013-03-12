//#############################################################################
//  File:      SLMeshFileDAE.h
//  Author:    Emanuel Hediger
//  Date:      16-MAR-10 (FS10)
//  Purpose:   DAE file loader
//  Copyright: Marcus Hudritsch, Kirchrain 18, 2572 Sutz
//             THIS SOFTWARE IS PROVIDED FOR EDUCATIONAL PURPOSE ONLY AND
//             WITHOUT ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED.
//#############################################################################

#ifndef SLMeshFileDAE_H
#define SLMeshFileDAE_H

#include <stdafx.h>
#include "SLGroup.h"
#include "SLMesh.h"

#include <dae.h>
#include <dom/domConstants.h>
#include <dom/domCOLLADA.h>
#include <dom/domProfile_COMMON.h>
#include <dae/daeSIDResolver.h>
#include <dom/domInstance_controller.h>
#include <dom/domInstance_geometry.h>
#include <dae/domAny.h>
#include <dae/daeErrorHandler.h>
#include <dae/daeUtils.h>
#include <dom/domFx_surface_init_from_common.h>
#include <modules/stdErrPlugin.h>
#include <dom/domEllipsoid.h>
#include <dom/domInputGlobal.h>
#include <dom/domAsset.h>

//-----------------------------------------------------------------------------
class SLMeshFileDAE
{  public:
                        SLMeshFileDAE(){;} 
                       ~SLMeshFileDAE()
					   {
						   if(dae)
						   {
							   dae->clear();
							   delete dae;
							   dae = NULL;
						   }
					   }
      // sets the path folder for the model
      static void		SetResourcePath(string path);
	  // loads a *.dae file into the scene
      static SLGroup*	Load(SLstring  filename, SLScene*  scene);

   private:
      static SLGroup*   _g;         // group that holds the DAE model
      static SLScene*   _s;         // parent scene
      static SLstring   _sPathname; // path to the collada resources

	  static DAE*		dae;
	  static bool		_yUP; // true if Y-Achses point up

	  static map<string, int> _materialIDMap;	// maps a material id to the materials index in the scenes material list
	  static map<string, int> _geometryIDMap;	// maps a gemoetry id to its mesh/geometry
	  static map<string,  domCommon_newparam_typeRef> _params; // used to temporarily store parameters of a dae

	  static void		ExtractNode(domNode* dn, SLGroup*   g);

	  static SLMesh*	ExtractGeometry(domInstance_geometry* geom);
	  static int		ExtractPolylist(domPolylist_Array polyListArr, int currentPolygon, int currentSurface, SLFace* F, SLMatFaces* M);
	  static int		ExtractTriangles(domTriangles_Array trianglesArr, int currentPolygon, int currentSurface, SLFace* F, SLMatFaces* M); 

	  static SLVec3f	ExtractVec3f(domFloat_array* floatArray, int index, int stride);
	  static SLVec2f	ExtractVec2f(domFloat_array* floatArray, int index, int stride);
	  static int		GetOrCreateIndex(domListOfUInts pArr, int polyIndex);	

	  static void		printSLVec(SLVec3f vec);
	  static void		printSLVec(SLVec2f vec);

	  static int		CountFaces(domTriangles_Array trianglesArr);
	  static int		CountFaces(domPolylist_Array polyListArr);
	  static int		CountFaces(domPolylist* polylist);

	  static void		ProcessInputs(domInputLocalOffset_Array inputArr);

	  static int		ExtractMaterial(domMaterial* domMat, string symbol);
	  static void		BindMaterial(domBind_material* bindMaterial);

	  static void		ExtractEffect(domInstance_effect* eInst, SLMaterial* mat);
	  static void		ExtractPhong(domProfile_COMMON::domTechnique::domPhong* phong, SLMaterial* mat);
	  static void		ExtractLambert(domProfile_COMMON::domTechnique::domLambert* lambert, SLMaterial* mat);
	  static void		ExtractBlinn(domProfile_COMMON::domTechnique::domBlinn* blinn, SLMaterial* mat);

	  static void		ExtractTexture(domCommon_color_or_texture_type_complexType::domTexture* tex, SLMaterial* mat);
	  static void		ExtractImage(domImage* img, SLMaterial* mat);
	  static SLCol4f	ExtractColor(domCommon_color_or_texture_type_complexType* ct);

};
//-----------------------------------------------------------------------------
#endif //SLMeshFileDAE_H
