//#############################################################################
//  File:      SLMeshFileDAE.cpp
//  Author:    Emanuel Hediger
//  Date:      16-MAR-10 (FS10)
//  Copyright: Marcus Hudritsch, Kirchrain 18, 2572 Sutz
//             THIS SOFTWARE IS PROVIDED FOR EDUCATIONAL PURPOSE ONLY AND
//             WITHOUT ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED.
//#############################################################################

#include <stdafx.h>
#ifdef _DEBUG
//#include <nvwa/debug_new.h>
#endif
#include "SLScene.h"
#include "SLMeshFileDAE.h"
#include "SLMesh.h"
#include "SLMaterial.h"
#include "SLTexture.h"
#include "SLRefShape.h"

#include "SLSceneView.h"
#include "SLGroup.h"
#include "SLCamera.h"
#include "SLLightSphere.h"
#include "SLPolygon.h"
#include "SLRectangle.h"

#include <vector>
#include <iostream>
#include <string>
#include <map>

#include <dae.h>
#include <dom/domConstants.h>
#include <dom/domCOLLADA.h>
#include <dom/domProfile_COMMON.h>
#include <dae/daeSIDResolver.h>
#include <dom/domInstance_controller.h>
#include <dae/domAny.h>
#include <dae/daeErrorHandler.h>
#include <dae/daeUtils.h>
#include <dom/domFx_surface_init_from_common.h>
#include <modules/stdErrPlugin.h>
#include <dom/domEllipsoid.h>
#include <dom/domInputGlobal.h>
#include <dom/domAsset.h>

//-----------------------------------------------------------------------------

// some constants for reading the collada
daeString COLLADA_TYPE_GEOMETRY = "geometry";
daeString COMMON_PROFILE_INPUT_POSITION = "VERTEX";
daeString COMMON_PROFILE_INPUT_NORMAL = "NORMAL";
daeString COMMON_PROFILE_INPUT_TEXCOORD = "TEXCOORD";

SLGroup* SLMeshFileDAE::_g = 0;           
SLScene* SLMeshFileDAE::_s = 0;    
SLstring SLMeshFileDAE::_sPathname = "";	// dae resource path / location of dae
bool	 SLMeshFileDAE::_yUP = true;	// stores if Y-Axis or Z-Axis points up
DAE*	 SLMeshFileDAE::dae;
map<string, int> SLMeshFileDAE::_materialIDMap;	// maps a material id to a slot in the scenes material vector
map<string, int> SLMeshFileDAE::_geometryIDMap;	// maps a geometry id to a slot int the scenes geometry vector
map<string,  domCommon_newparam_typeRef> SLMeshFileDAE::_params;	// used to temporarily store parameters of effects ect.

int vertexIndexStride;	// current stride / dimension of vector (XYZ is standard)
int normalIndexStride;	// current stride / dimension of normal (XYZ is standard)
int tcIndexStride;	// current stride / dimension of a texture coordyante (UV or STP coordynates)

bool hasTextures = false;	// stores if the current mesh has textures
bool hasNormals = false;	// stores if the current mesh has normals
bool initVNMap = true;

domFloat_array* domVertexArr;	// pointer to current vertex index domFloat_array
domFloat_array* domNormalArr;	// pointer to current normal index domFloat_array
domFloat_array* domUVArr;	// pointer to current texcoord index domFloat_array

SLVVec3f tempVertices;	// growable list of vertices of current mesh
SLVVec3f tempNormals;	// growable list of normals of current mesh
SLVVec2f tempTextureCoordynates;	// growable list of texture coordynates of current mesh
map<int, map<int, int>> vnIndexMap;	// lookup map to check if a vertex normal combination already exists
std::vector<int> inputMap;	// current input map (maps input to vertex normal or texturecoordynate ect.)
SLMaterial* matGray = new SLMaterial("Gray",  SLCol4f(0.3f,0.3f,0.3f), SLCol4f(0.0,0.0,0.0), 0, 0,  0); // default material

//-----------------------------------------------------------------------------
//! extracts and returns a SLVec3f from a given domFloat_array
SLVec3f	SLMeshFileDAE::ExtractVec3f(domFloat_array* floatArray, int index, int stride)
{
	int realIndex = index * stride;
	return SLVec3f(floatArray->getValue()[realIndex],
			       floatArray->getValue()[realIndex + 1],
				   floatArray->getValue()[realIndex + 2]);
}

//-----------------------------------------------------------------------------
//! extracts and returns a SLVec2f from a given domFloat_array
SLVec2f	SLMeshFileDAE::ExtractVec2f(domFloat_array* floatArray, int index, int stride)
{
	int realIndex = index * stride;
	return SLVec2f(floatArray->getValue()[realIndex],
			       floatArray->getValue()[realIndex + 1]);
}

//-----------------------------------------------------------------------------
//! extracts an image file and loads it into an SLTexture and adds it to the scenes textures and the material
void SLMeshFileDAE::ExtractImage(domImage* img, SLMaterial* mat) {

	cout << "Extract Image id: " << img->getId() << "\n"; 
	//cout << "Image DocURI: " << img->getDocumentURI()->getURI() << "\n";
	//cout << "Image FileName: " << img->getInit_from()->getValue().str() << "\n";

	domImage::domInit_from* initFrom = img->getInit_from();
    if (initFrom != NULL) {
		string sPathFilename = _sPathname; 
		sPathFilename.append(initFrom->getValue().getOriginalURI());
		cout << "sPathFilename: \"" << sPathFilename << "\"\n";		
		SLTexture* tex = new SLTexture(sPathFilename, 
                                     false, 
                                     GL_LINEAR, 
                                     GL_LINEAR, 
                                     GL_MODULATE, 
                                     false,
                                     false);
		_s->texture().push_back(tex);   
		mat->texture().push_back(tex);
    }
}

//-----------------------------------------------------------------------------
//! extracts a texture
void SLMeshFileDAE::ExtractTexture(domCommon_color_or_texture_type_complexType::domTexture* tex, SLMaterial* mat)
{	
    // try do evaluate the sid
    daeIDRef id(tex->getTexture());
	cout << "Extract Texture: " << id.getID() << "\n"; 
    id.setContainer(tex);
    daeElement* elm = id.getElement();
    // if this doesn't work then look in our newparam map
    if (elm == NULL) {
		 cout << "Invalid texture reference try params " << "\n";
		elm = _params[tex->getTexture()];
        if (elm == NULL) {
            cout << "Invalid texture reference: " << 
                tex->getTexture() << ". No texture Loaded." << "\n";
            return;
        }
    }  
	// ff the texture reference points to an image node
    if (elm->getElementType() == COLLADA_TYPE::IMAGE) {
		 cout << "Texture reference is IMAGE " << "\n";
        ExtractImage(dynamic_cast<domImage*>(elm), mat);
        return;
    }
    
    // ff the texture points to a common new param node
    if (elm->getElementType() == COLLADA_TYPE::COMMON_NEWPARAM_TYPE)
	{
		cout << "Texture points to common newparam type " << "\n";
		// get the sampler
        domFx_sampler2D_common* sampler = dynamic_cast<domCommon_newparam_type*>(elm)->getSampler2D();
		if (sampler == NULL)
		{        
			cout << "Sampler is NULL " << "\n";
			return;
		}
		// get the source of the sampler
        daeElement* src = _params[sampler->getSource()->getValue()];
        if (src == NULL) 
		{
			cout << "Sampler source is NULL " << "\n";
            return;
		}
		// get the surface out of the source
        domFx_surface_common* surface = dynamic_cast<domCommon_newparam_type*>(src)->getSurface();
        if (surface == NULL || surface->getType() != FX_SURFACE_TYPE_ENUM_2D)
		{
			cout << "SURFACE IS NULL " << "\n";
            return;
		}
		// get the image out of the source element
        if (surface->getFx_surface_init_common()->getInit_from_array().getCount() != 0) {
			//cout << "Surface init" << "\n";
            daeElement* init = surface->getFx_surface_init_common()->getInit_from_array()[0]->getValue().getElement();
            if (init->getElementType() == COLLADA_TYPE::IMAGE) {
				//cout << "init getElementType == Image " << "\n";
				ExtractImage(dynamic_cast<domImage*>(init), mat);
                return;
            }
        }
    }
}

//-----------------------------------------------------------------------------
//! extracts a material and stores it if necessary, returns iMat as index
int SLMeshFileDAE::ExtractMaterial(domMaterial* domMat, string symbol)
{
	if (domMat == NULL) {
        cout << "No material found. Fall back to default material. \n";
        return 0;
    } 
	SLVMaterial& sceneMat = _s->material();
	if(_materialIDMap[symbol] == 0)
	{
		cout << "Creating new material: " << symbol << "\n";
		SLMaterial* mat = new SLMaterial("Default: White",  SLCol4f(1.0f,1.0f,1.0f), SLCol4f(1,1,1), 100, 0,  0);
		ExtractEffect(domMat->getInstance_effect(), mat);
		sceneMat.push_back(mat);
		_materialIDMap[symbol] = sceneMat.size();
		return _materialIDMap[symbol] - 1;
	} 
	else
	{
		cout << "MATERIAL EXISTS: " << symbol << "\n";
		return _materialIDMap[symbol] - 1;
	}
	return 0;
}

//-----------------------------------------------------------------------------
//! extracts an effect (shader) and sets the materials properties
void SLMeshFileDAE::ExtractEffect(domInstance_effect* eInst, SLMaterial* mat)
{
	cout << "Extracting Effect\n";
	domEffect* e = dynamic_cast<domEffect*>(eInst->getUrl().getElement().cast());
	domFx_profile_abstract_Array profileArr = e->getFx_profile_abstract_array();	
	for (unsigned int i = 0; i < profileArr.getCount(); i++)
	{
		//cout << "PROFILE: " << i << "\n";
        // only process the profile_COMMON technique
        if (profileArr[i]->getElementType() == COLLADA_TYPE::PROFILE_COMMON)
		{
			//cout << "IS COMMON PROFILE\n";
			domProfile_COMMON* profile_common = 
                dynamic_cast<domProfile_COMMON*>(profileArr[i].cast());
            domProfile_COMMON::domTechnique* technique = 
                profile_common->getTechnique();	
			// read the params for this effect
			_params.clear();
            domCommon_newparam_type_Array newparam_array = profile_common->getNewparam_array();			
            for (unsigned int j = 0; j < newparam_array.getCount(); j++) {
                _params[newparam_array[j]->getSid()] = newparam_array[j];
            }

			domProfile_COMMON::domTechnique::domPhong* phong = technique->getPhong();
            domProfile_COMMON::domTechnique::domLambert* lambert = technique->getLambert();
            domProfile_COMMON::domTechnique::domBlinn* blinn = technique->getBlinn();
			if (phong != NULL)
			{
				cout << "Extract Phong" << "\n";
                ExtractPhong(phong, mat);
            }
            if (lambert != NULL)
			{
				cout << "Extract Lambert" << "\n";
                ExtractLambert(lambert, mat);
            }
			if (blinn != NULL)
			{
				cout << "Extract Blinn" << "\n";
                ExtractBlinn(blinn, mat);
            }
		}
		else
		{
			//cout << "NOT COMMON PROFILE\n";
		}
	}
}

//-----------------------------------------------------------------------------
//! extracts phong shader effect and sets material properties
void SLMeshFileDAE::ExtractPhong(domProfile_COMMON::domTechnique::domPhong* phong, SLMaterial* mat)
{
	if (phong->getDiffuse() != NULL)
	{
		//cout << "Phong getDiffuse() != NULL" << "\n";
		if (phong->getDiffuse()->getTexture() != NULL)
		{
			//cout << "Phong getDiffuse()->getTexture() != NULL : " << phong->getDiffuse()->getTexture() << "\n";
			mat->diffuse(SLCol4f(1.0f, 1.0f, 1.0f, 1.0f));
			ExtractTexture(phong->getDiffuse()->getTexture(), mat);
		} else {
			//cout << "Phong getDiffuse()->getTexture() == NULL" << "\n";
			mat->diffuse(ExtractColor(phong->getDiffuse()));
		}
	} else {
		//cout << "Phong getDiffuse() == NULL" << "\n";
	}
	mat->ambient(ExtractColor(phong->getAmbient()));
	mat->specular(ExtractColor(phong->getSpecular()));
	mat->emission(ExtractColor(phong->getEmission()));	
	if (phong->getShininess() != NULL)
	{
		if (phong->getShininess()->getFloat() != NULL)
		{
			mat->shininess(phong->getShininess()->getFloat()->getValue());
		}
	}
}

//-----------------------------------------------------------------------------
//! extracts lambert shader effect and sets material properties
void SLMeshFileDAE::ExtractLambert(domProfile_COMMON::domTechnique::domLambert* lambert, SLMaterial* mat)
{
	if (lambert->getDiffuse() != NULL)
	{
       if (lambert->getDiffuse()->getTexture() != NULL)
		{
			mat->diffuse(SLCol4f(1.0f, 1.0f, 1.0f, 1.0f));
			ExtractTexture(lambert->getDiffuse()->getTexture(), mat);
		} else {
			mat->diffuse(ExtractColor(lambert->getDiffuse()));
		}
    }
	mat->ambient(ExtractColor(lambert->getAmbient()));
	mat->specular(SLCol4f(0, 0, 0, 0));
	mat->emission(ExtractColor(lambert->getEmission()));	
}

//-----------------------------------------------------------------------------
//! extracts blinn shader effect and sets material properties
void SLMeshFileDAE::ExtractBlinn(domProfile_COMMON::domTechnique::domBlinn* blinn, SLMaterial* mat)
{
	if (blinn->getDiffuse() != NULL) {
       if (blinn->getDiffuse()->getTexture() != NULL)
		{
			mat->diffuse(SLCol4f(1.0f, 1.0f, 1.0f, 1.0f));
			ExtractTexture(blinn->getDiffuse()->getTexture(), mat);
		} else {
			mat->diffuse(ExtractColor(blinn->getDiffuse()));
		}
    }
	mat->ambient(ExtractColor(blinn->getAmbient()));
	mat->specular(ExtractColor(blinn->getSpecular()));
	mat->emission(ExtractColor(blinn->getEmission()));	
	if (blinn->getShininess() != NULL)
	{
		if (blinn->getShininess()->getFloat() != NULL)
		{
			mat->shininess(blinn->getShininess()->getFloat()->getValue());
		}
	}
}

//-----------------------------------------------------------------------------
//! extracts color and returns it as SLCol4f
SLCol4f SLMeshFileDAE::ExtractColor(domCommon_color_or_texture_type_complexType* ct)
{
    if (ct != NULL && ct->getColor() != NULL)
	{
        domFx_color_common col = ct->getColor()->getValue();
        return SLCol4f(col[0],col[1],col[2],col[3]);
    }
	return SLCol4f(1.0, 1.0, 1.0, 1.0);
}

//-----------------------------------------------------------------------------
//! prints a SLVec3f to the console
void SLMeshFileDAE::printSLVec(SLVec3f a)
{
	cout << "[" << a.x << ", " << a.y << ", " << a.z << "]\n";
}

//-----------------------------------------------------------------------------
//! prints a SLVec3f to the console
void SLMeshFileDAE::printSLVec(SLVec2f a)
{
	cout << "[" << a.x << ", " << a.y << "]\n";
}

//-----------------------------------------------------------------------------
//! returns index of a vertex and grows the set of vertices if necessary and returns newly created index
int SLMeshFileDAE::GetOrCreateIndex(domListOfUInts pArr, int polyIndex)
{
	int index = -1;
	int v = -1;
	int n = -1;
	int tc = -1;	
	// read vertex, normal and texturecoordynate index using the input map
	int inputCount = inputMap.size();
	for(int i = 0; i < inputCount; i++)
	{
		int in = inputMap[i];
		if(in == 0)
		{
			v = pArr[polyIndex + i];
		}
		else if(in == 1)
		{
			n = pArr[polyIndex + i];
		} 
		else if(in == 2)
		{
			tc = pArr[polyIndex + i];
		}
	}
	// if the model has vertex and normals
	if(v > -1 && n > -1)
	{
		// if vertex normal combination does not exist grow vertex list
		if(vnIndexMap[v][n] == 0)
		{
			tempVertices.push_back(ExtractVec3f(domVertexArr, v, vertexIndexStride));
			tempNormals.push_back(ExtractVec3f(domNormalArr, n, normalIndexStride));
			// if texture coordynates exist store them back too
			if(tc > -1)
			{
				tempTextureCoordynates.push_back(ExtractVec2f(domUVArr, tc, tcIndexStride));
			}
			vnIndexMap[v][n] = tempVertices.size();
		}
		// return vertex index
		return vnIndexMap[v][n] - 1;
	}
	else
	{
		cout << "Models without Normals not supported!" << "\n";
		return v;
	}
}

//-----------------------------------------------------------------------------
//! counts and returns the number faces of a domTriangles_Array
int SLMeshFileDAE::CountFaces(domTriangles_Array trianglesArr)
{
	int numF = 0;
	domTriangles* triangle;
	for (unsigned int t = 0; t < trianglesArr.getCount(); t++)
	{
		triangle = trianglesArr[t];
		numF += triangle->getCount();
	}
	return numF;
}

//-----------------------------------------------------------------------------
//! counts and returns the number of faces of a domPolylist_Array
int SLMeshFileDAE::CountFaces(domPolylist_Array polyListArr)
{
	int numTriangles = 0;
	for(unsigned int i = 0; i < polyListArr.getCount(); i++)
	{
		numTriangles += CountFaces(polyListArr[i]);
	}
	return numTriangles;
}

//-----------------------------------------------------------------------------
//! counts and returns the number of faces of a domPolylist
int SLMeshFileDAE::CountFaces(domPolylist* polylist)
{
	int numTriangles = 0;
	int vertexCount = (int) polylist->getVcount()->getValue().getCount();
	for(int v = 0; v < vertexCount; v++)
	{
		numTriangles += polylist->getVcount()->getValue()[v] - 2;
	}	
	return numTriangles;
}

//-----------------------------------------------------------------------------
//! recursively extracts and loads domNodes
void SLMeshFileDAE::ExtractNode(domNode* dn, SLGroup*   g)
{
	SLGroup* node = new SLGroup();
	g->addNode(node);

	// load all contained geometries
	domInstance_geometry_Array geomArr = dn->getInstance_geometry_array();
    for (unsigned int j = 0; j < geomArr.getCount(); j++)
	{				
		SLMesh* mesh = ExtractGeometry(geomArr[j]);
		if(!mesh)
		{
			cout << "Warning mesh could not be extracted!\n";
			return;
		}
		else
		{
			if(mesh->prev() != 0)
			{
				// geometry already loaded, create reference node
				SLRefShape* refMesh = new SLRefShape(mesh, "refmesh");
				node->addNode(refMesh);
			}
			else
			{
				// new geometry loaded add mesh directly
			    node->addNode(mesh);		
	        }
		}
	}
	// loop through all elementy contained in node
	daeTArray<daeSmartRef<daeElement> > elms;
    dn->getChildren(elms);
	for (unsigned int i = 0; i < elms.getCount(); i++)
	{	
		if(elms[i]->getElementType() == COLLADA_TYPE::LIGHT)
		{
			domLight* light = dynamic_cast<domLight*>(elms[i].cast());
			// currently not supported
			continue;
		}

		if (elms[i]->getElementType() == COLLADA_TYPE::MATRIX) {
            domFloat4x4 m = dynamic_cast<domMatrix*>(elms[i].cast())->getValue();
			node->setMatrix(m[0],m[1],m[2],m[4],
							m[5],m[6],m[8],m[9],
							m[10],m[11],m[12],m[13],
							m[14],m[15],m[16],m[17]);
            continue;
        }

		if (elms[i]->getElementType() == COLLADA_TYPE::ROTATE)
		{			
			domFloat4 rot = dynamic_cast<domRotate*>(elms[i].cast())->getValue();	
			node->rotate(rot[3], rot[0], rot[1], rot[2]);
            continue;
        }
        if (elms[i]->getElementType() == COLLADA_TYPE::SCALE)
		{
            domFloat3 scale = dynamic_cast<domScale*>(elms[i].cast())->getValue();
			node->scale(scale[0], scale[1], scale[2]);
            continue;
        }          
        if (elms[i]->getElementType() == COLLADA_TYPE::TRANSLATE)
		{		
            domFloat3 trans = dynamic_cast<domTranslate*>(elms[i].cast())->getValue();
			node->translate(trans[0], trans[1],trans[2]);
        }
	}        
	// loop trough all childnodes of the node
    domInstance_node_Array nodeArr = dn->getInstance_node_array();
    for (unsigned int n = 0; n < nodeArr.getCount(); n++)
	{
        ExtractNode(dynamic_cast<domNode*>(nodeArr[n]->getUrl().getElement().cast()), node);
    }
}

//-----------------------------------------------------------------------------
//! processes and creates inputmap
void SLMeshFileDAE::ProcessInputs(domInputLocalOffset_Array inputArr)
{
	cout << "process inputs\n";
	int inputCount = inputArr.getCount();
	domInputLocalOffset* input;
	daeString semantic;
	domSource* src;
	int offset;	
	inputMap.clear();
	// loop through all inputs and create entry in input map
	for(int i = 0; i < inputCount; i++)
	{
		//
			
		int in = -1;
		input = dynamic_cast<domInputLocalOffset*>(inputArr[i].cast());
		if(input == NULL) cout << "WARNING INPUT IS NULL\n";
		offset = input->getOffset();
		semantic = input->getSemantic();		

		if (strcmp(semantic, COMMON_PROFILE_INPUT_POSITION) == 0)
		{
			if(domVertexArr == NULL)	
			{	
				// input is VERTEX/POSITION, store semantic, offset, stride and domFloat_array
				cout << "process vertex array\n";
				domVertices* v = dynamic_cast<domVertices*>(input->getSource().getElement().cast());
				domInputLocal_Array vinputArr = v->getInput_array();
				src = dynamic_cast<domSource*>(vinputArr[0]->getSource().getElement().cast());
				vertexIndexStride = src->getTechnique_common()->getAccessor()->getStride();
				domVertexArr = src->getFloat_array();
			}
			in = 0;
			//...
		}
		else if (strcmp(semantic,COMMON_PROFILE_INPUT_NORMAL) == 0)
		{
			if(domNormalArr == NULL)
			{
				// input is NORMAL, store semantic, offset, stride and domFloat_array
				cout << "process normal array\n";
				src = dynamic_cast<domSource*>(input->getSource().getElement().cast());
				normalIndexStride = src->getTechnique_common()->getAccessor()->getStride();
				domNormalArr = src->getFloat_array();
				if(domNormalArr != NULL) hasNormals = true;
			}
			in = 1;
		}
		else if (strcmp(semantic,COMMON_PROFILE_INPUT_TEXCOORD) == 0)
		{
			if(domUVArr == NULL)
			{
				// input is TEXCOORD, store semantic, offset, stride and domFloat_array
				cout << "process uv array\n";
				src = dynamic_cast<domSource*>(input->getSource().getElement().cast());
				tcIndexStride = src->getTechnique_common()->getAccessor()->getStride();
				domUVArr = src->getFloat_array();
			}
			hasTextures = true;
			in = 2;

		}
		inputMap.push_back(in);
	}
}

//-----------------------------------------------------------------------------
//! loads and binds all materials of an instance geometry/mesh
void SLMeshFileDAE::BindMaterial(domBind_material* bindMat)
{
	if(bindMat)
	{
		//cout << "GOT BINDMATERIAL!" << "\n";
		// Get the <technique_common>
		domBind_material::domTechnique_common *techniqueCommon = bindMat->getTechnique_common();
		if(techniqueCommon)
		{
			//cout << "GOT TECHNIQUE COMMON!" << "\n";
			// Get the <instance_material>s
			domInstance_material_Array &instanceMaterialArray = techniqueCommon->getInstance_material_array();
			for(unsigned int j = 0; j < instanceMaterialArray.getCount(); j++)
			{
				//cout << "Found material count: " << j << "\n";
				string symbol = instanceMaterialArray[j]->getSymbol();
				string target = instanceMaterialArray[j]->getTarget().getID();
				//cout << "Store symbol: " << symbol << "\n";
				//cout << "target: " << target << "\n";				
				domElement * elm = instanceMaterialArray[j]->getTarget().getElement();
				if(elm == NULL)
				{
					cout << "DOM ELEMENT IS NULL!!" << "\n";
					return;
				}
				domMaterial* domMat = dynamic_cast<domMaterial*>(elm);
				if(domMat == NULL)
				{
					cout << "MATERIAL IS NULL!!" << "\n";
					return;
				}
				else
				{
					//cout << "MATERIAL IS NOT NULL!!" << "\n";
					ExtractMaterial(domMat, symbol);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
//! loads all polygonlists in polyListArr
int SLMeshFileDAE::ExtractPolylist(domPolylist_Array polyListArr, int currentPolygon, int currentSurface, SLFace* F, SLMatFaces* M)
{
	int numVertices = 0;
	for (int t = 0; t < (int) polyListArr.getCount(); t++)
	{
		domPolylist* polylist = polyListArr[t];
		domInputLocalOffset_Array inputArr = polylist->getInput_array();
		ProcessInputs(inputArr);
		int numInputs = inputArr.getCount();
		daeIDRef materialRef(polylist->getMaterial());
		materialRef.setContainer(polylist);
		cout << "Look up material: " << materialRef.getID() << "\n";
		int iMat = _materialIDMap[materialRef.getID()] - 1;
		int matFaceIndex = currentSurface + t;
		M[currentSurface].numF = CountFaces(polylist);
		M[currentSurface].startF = currentPolygon;
		M[currentSurface].vbo_F = 0;
		M[currentSurface].iMat = iMat;

		domListOfUInts pArr = polylist->getP()->getValue();
		// array with number of vertices for each polygon
		domPolylist::domVcountRef Vcount = polylist->getVcount();
		// iterate through all n-polygons
		int polyIndex = 0;
		for(int i = 0; i < (int) Vcount->getValue().getCount(); i++)
		{
			numVertices = (int) Vcount->getValue()[i];
			// loop trough all potential polygons			
			for(int j = 0; j < numVertices - 2; j++)
			{
				F[currentPolygon] = SLFace();
				F[currentPolygon].iA = GetOrCreateIndex(pArr, polyIndex);
				F[currentPolygon].iB = GetOrCreateIndex(pArr, polyIndex + j * numInputs + numInputs);
				F[currentPolygon].iC = GetOrCreateIndex(pArr, polyIndex + j * numInputs + numInputs * 2);
				currentPolygon++;
			}
			polyIndex += numVertices * numInputs;
		}	

		inputMap.clear();
		currentSurface++;
	}	
	return currentPolygon;
}

//-----------------------------------------------------------------------------
//! loads all trianglelists in trianglesArr
int SLMeshFileDAE::ExtractTriangles(domTriangles_Array trianglesArr, int currentPolygon, int currentSurface, SLFace* F, SLMatFaces* M)
{
	for (int t = 0; t < (int) trianglesArr.getCount(); t++)
	{
		// a set of triangles / faces
		domTriangles* triangles = trianglesArr[t];
		domInputLocalOffset_Array inputArr = triangles->getInput_array();
		ProcessInputs(inputArr);
		int numInputs = inputArr.getCount();		
		daeIDRef materialRef(triangles->getMaterial());
		materialRef.setContainer(triangles);	
		cout << "Look up material: " << materialRef.getID() << "\n";
		int iMat = _materialIDMap[materialRef.getID()] - 1;
		// primitive array / list of indexes
		domListOfUInts pArr = triangles->getP()->getValue();
		int numP = triangles->getCount();
		int matFaceIndex = currentSurface + t;
		M[currentSurface].numF = numP;
		M[currentSurface].startF = currentPolygon;
		M[currentSurface].vbo_F = 0;
		M[currentSurface].iMat = iMat;
		cout << "MATFACE POLYGON COUNT: " << numP << "\n";
		for(int polygon = 0; polygon < numP; polygon++)
		{
			int index = polygon * numInputs * 3;
			F[currentPolygon] = SLFace();
			F[currentPolygon].iA = GetOrCreateIndex(pArr, index);
			F[currentPolygon].iB = GetOrCreateIndex(pArr, index + numInputs);
			F[currentPolygon].iC = GetOrCreateIndex(pArr, index + 2 * numInputs);
			currentPolygon++;
		}
		inputMap.clear();
		currentSurface++;
	}	
	return currentPolygon;
}

//-----------------------------------------------------------------------------
//! extracts a geometry and returns it as mesh
SLMesh* SLMeshFileDAE::ExtractGeometry(domInstance_geometry* geom)
{
	cout << "-----------------------------\n";
	cout << "start extracting mesh\n";
	domGeometry* thisGeometry = dynamic_cast<domGeometry*>(geom->getUrl().getElement().cast());
	int geometryIndex = _geometryIDMap[thisGeometry->getID()];
	if(geometryIndex > 0)
	{
		// geometry was already loaded
		cout << "geometry already in library\n";
		return _s->geometry()[geometryIndex - 1];
	} 

	domMesh* mesh = thisGeometry->getMesh();
	BindMaterial(geom->getBind_material());

	if (mesh->getLines_array().getCount() > 0)
        cout << "Unsupported geometry types found: Lines" << "\n";
    if (mesh->getLinestrips_array().getCount() > 0)
        cout << "Unsupported geometry types found: Linestrips" << "\n";
    if (mesh->getPolygons_array().getCount() > 0)
        cout << "Unsupported geometry types found: Polygons" << "\n";
    if (mesh->getTrifans_array().getCount() > 0)
        cout << "Unsupported geometry types found: Trifans" << "\n";
    if (mesh->getTristrips_array().getCount() > 0)
        cout << "Unsupported geometry types found: Tristrips" << "\n";

    
	domPolylist_Array polyListArr = mesh->getPolylist_array();
	domTriangles_Array trianglesArr = mesh->getTriangles_array();

	int polyFaces = CountFaces(polyListArr);
	int polyMaterials = polyListArr.getCount();
	cout << "Poly List Faces: " << polyFaces << "\n";
	cout << "Poly List Materials: " << polyMaterials << "\n";

	int triaFaces = CountFaces(trianglesArr);
	int triaMaterials = trianglesArr.getCount();
	cout << "Triangle List Faces: " << triaFaces << "\n";	
	cout << "Triangle List Materials: " << triaMaterials << "\n";

	int numF = polyFaces + triaFaces;
	int numM = polyMaterials + triaMaterials;
	int iMat = 0;
	SLFace* F = new SLFace[numF];
	SLMatFaces* M = new SLMatFaces[numM];	
	M->numF = numF;
	M->startF = 0;
	M->vbo_F = 0;
	M->iMat = -1;
	int currentPolygon = 0;
	int currentSurface = 0;
	currentPolygon = ExtractPolylist(polyListArr, currentPolygon, currentSurface, F, M);
	currentPolygon = ExtractTriangles(trianglesArr, currentPolygon, currentSurface, F, M);
	
	int numV = tempVertices.size();
	cout << "TOTAL VERTICES & NORMALS: \"" << numV << "\"\n";
	SLVec3f* V = new SLVec3f[numV];
	SLVec3f* N = new SLVec3f[numV];
	SLVec2f* Tc;
	int i;
	// copy temp vectors to real vectors
	if(hasTextures)
	{
		Tc = new SLVec2f[numV];
		for(i=0; i < numV; i++)
		{
			Tc[i] = tempTextureCoordynates[i];
		}
	}
	
	for(i=0; i < numV; i++)
	{
		V[i] = tempVertices[i];
		N[i] = tempNormals[i];
	}
	// delete maps and temp arrays
	cout << "Mesh vertices: " << numV << "\n";
	domVertexArr = NULL;
	domNormalArr = NULL;
	domUVArr = NULL;

	map<int, map<int, int>>::iterator vPtr = vnIndexMap.begin();
	while (vPtr != vnIndexMap.end()) {
		 vnIndexMap[(*vPtr).first].clear();
		 vPtr++;
	}
	vnIndexMap.clear();
	initVNMap = true;
	inputMap.clear();
	tempVertices.clear();
	tempNormals.clear();
	tempTextureCoordynates.clear();
	_params.clear();

	SLMesh* realMesh = new SLMesh(_s, "testmesh", matGray);	
	realMesh->numV = numV;
	realMesh->V = V; 
	realMesh->N = N;
	if(hasTextures)
	{
		realMesh->Tc = Tc;
		hasTextures = false;
	}
	realMesh->numF = numF;
	realMesh->numM = numM;
	realMesh->F = F;
	realMesh->M = M;

	if(hasNormals == false)	
	{
		realMesh->calcNormals();
		hasNormals = true;
	}

	realMesh->buildAABB();
	
	cout << "finished extracting mesh\n";
	cout << "-----------------------------\n";
	// add geometry to library
	_s->geometry().push_back(realMesh);
	_geometryIDMap[thisGeometry->getID()] = _s->geometry().size();
	return realMesh;
}


//-----------------------------------------------------------------------------
//! sets the dae resource path
void SLMeshFileDAE::SetResourcePath(string path)
{
	cout << "SETTING RESOURCE PATH: \"" << path << "\"\n";
	_sPathname = path;
}

//-----------------------------------------------------------------------------
//! loads a collada file into a group, adds it to the scene and returns the group
SLGroup* SLMeshFileDAE::Load(SLstring filename, SLScene* scene)
{  
	cout << "\n--------------------------------------------------------------\n";
	_s = scene;  
	SLVMaterial& sceneMat = _s->material();
	SLVMesh& sceneGeometry = _s->geometry();
	int error = 0;
	// create holding group
    _g = new SLGroup();	
	// push back default material
    sceneMat.push_back(matGray);
	// create new dae
	//DAE* dae = new DAE();
	dae = new DAE();
	// construct file path
	string file = _sPathname + filename;
	// load dae file
    int err = dae->load(file.data());
    if (err != DAE_OK)
	{
       cout << "Error opening Collada file: " << file << "\n";
	   return _g;
	}
	else
	{
		cout << "Opened Collada file: " << file << "\n\n";
	}
    // default axis settings, Y-axis is up!
    domCOLLADA* domRoot = dae->getDom(file.data());
    if (domRoot->getAsset()->getUp_axis()->getValue() == UPAXISTYPE_Z_UP)
	{
        cout << "Z-UP" << "\n\n";
        _yUP = false;
	}
	// get scene from dae database
    daeDatabase* db = dae->getDatabase();
	domCOLLADA::domScene* dscene;    

    dae->getDatabase()->getElement((daeElement**)&dscene, 
                                         0, 
                                         NULL, 
										 "scene", 
                                         NULL);
	if(dscene == NULL)
	{
		cout << "SCENE IS NULL\n";
		return _g;
	}
	if(dscene->getInstance_visual_scene() == NULL)
	{
		cout << "visual scene can't be read\n";
		return _g;
	}
	// if there is a visual scene load it
    domVisual_scene* visualScene = dynamic_cast<domVisual_scene*>(dscene->getInstance_visual_scene()->getUrl().getElement().cast());
	// get the scenes instance node array and load all nodes recursively
    domNode_Array nodeArr = visualScene->getNode_array();	
    for (unsigned int n = 0; n < nodeArr.getCount(); n++)
	{
		ExtractNode(nodeArr[n], _g);
    }	
	if(!_yUP)
	{
		_g->rotate(270, 1, 0, 0);
	}
	dae->unload(file.data());
	dae->close(file.data());	
	cout << "\n--------------------------------------------------------------\n";
    cout << ("FINISHED LOADING COLLADA FILE\n");
    cout << "\n--------------------------------------------------------------\n";  
	return _g;
}