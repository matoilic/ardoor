##############################################################################
#  File:      chXX_Final.pro
#  Purpose:   QMake project definition file for the final chapter application
#  Author:    Marcus Hudritsch
#  Date:      September 2012 (HS12)
#  Copyright: Marcus Hudritsch, Switzerland
#             THIS SOFTWARE IS PROVIDED FOR EDUCATIONAL PURPOSE ONLY AND
#             WITHOUT ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED.
##############################################################################

TEMPLATE = app
TARGET = chXX_Final
DEFINES += "SL_STARTSCENE=cmdSceneRevolver"
DEFINES += SL_RT1

include(../_globals/_globals.pro)

HEADERS += \
    ../_globals/GL/glUtils.h \
    ../_globals/GL/SLGLBuffer.h \
    ../_globals/GL/SLGLShader.h \
    ../_globals/GL/SLGLShaderProg.h \
    ../_globals/GL/SLGLShaderProgGeneric.h \
    ../_globals/GL/SLGLShaderUniform.h \
    ../_globals/GL/SLGLState.h \
    ../_globals/GL/SLGLTexture.h \
    ../_globals/math/SLCurve.h \
    ../_globals/math/SLCurveBezier.h \
    ../_globals/math/SLMat3.h \
    ../_globals/math/SLMat4.h \
    ../_globals/math/SLMath.h \
    ../_globals/math/SLPlane.h \
    ../_globals/math/SLQuat4.h \
    ../_globals/math/SLVec2.h \
    ../_globals/math/SLVec3.h \
    ../_globals/math/SLVec4.h \
    ../_globals/math/TriangleBoxIntersect.h \
    ../_globals/MeshLoader/SL3DSMesh.h \
    ../_globals/MeshLoader/SL3DSMeshFile.h \
    ../_globals/SL/SL.h \
    ../_globals/SL/SLDrawBits.h \
    ../_globals/SL/SLEventHandler.h \
    ../_globals/SL/SLFileSystem.h \
    ../_globals/SL/SLImage.h \
    ../_globals/SL/SLInterface.h \
    ../_globals/SL/SLObject.h \
    ../_globals/SL/SLTexFont.h \
    ../_globals/SL/SLTimer.h \
    ../_globals/SL/SLUtils.h \
    ../_globals/SL/SLVector.h \
    ../_globals/SL/stdafx.h \
    ../_globals/SpacePartitioning/SLAccelStruct.h \
    ../_globals/SpacePartitioning/SLUniformGrid.h \
    include/SLAABBox.h \
    include/SLAnimation.h \
    include/SLBox.h \
    include/SLButton.h \
    include/SLCamera.h \
    include/SLCone.h \
    include/SLCylinder.h \
    include/SLGroup.h \
    include/SLKeyframe.h \
    include/SLLight.h \
    include/SLLightRect.h \
    include/SLLightSphere.h \
    include/SLMaterial.h \
    include/SLMesh.h \
    include/SLNode.h \
    include/SLPolygon.h \
    include/SLRay.h \
    include/SLRaytracer.h \
    include/SLRectangle.h \
    include/SLRefGroup.h \
    include/SLRefShape.h \
    include/SLRevolver.h \
    include/SLSamples2D.h \
    include/SLScene.h \
    include/SLSceneView.h \
    include/SLShape.h \
    include/SLSphere.h \
    include/SLText.h

SOURCES += \
    ../_globals/GL/glUtils.cpp \
    ../_globals/GL/SLGLBuffer.cpp \
    ../_globals/GL/SLGLShader.cpp \
    ../_globals/GL/SLGLShaderProg.cpp \
    ../_globals/GL/SLGLState.cpp \
    ../_globals/GL/SLGLTexture.cpp \
    ../_globals/GUI/glfw/glfwMain.cpp \
    ../_globals/math/SLCurveBezier.cpp \
    ../_globals/math/SLPlane.cpp \
    ../_globals/MeshLoader/SL3DSMesh.cpp \
    ../_globals/MeshLoader/SL3DSMeshFile.cpp \
    ../_globals/SL/SL.cpp \
    ../_globals/SL/SLFileSystem.cpp \
    ../_globals/SL/SLImage.cpp \
    ../_globals/SL/SLInterface.cpp \
    ../_globals/SL/SLTexFont.cpp \
    ../_globals/SL/SLTimer.cpp \
    ../_globals/SpacePartitioning/SLUniformGrid.cpp \
    source/SLAABBox.cpp \
    source/SLAnimation.cpp \
    source/SLBox.cpp \
    source/SLButton.cpp \
    source/SLCamera.cpp \
    source/SLCone.cpp \
    source/SLCylinder.cpp \
    source/SLGroup.cpp \
    source/SLLight.cpp \
    source/SLLightRect.cpp \
    source/SLLightSphere.cpp \
    source/SLMaterial.cpp \
    source/SLMesh.cpp \
    source/SLPolygon.cpp \
    source/SLRay.cpp \
    source/SLRaytracer.cpp \
    source/SLRectangle.cpp \
    source/SLRefGroup.cpp \
    source/SLRefShape.cpp \
    source/SLRevolver.cpp \
    source/SLSamples2D.cpp \
    source/SLScene.cpp \
    source/SLSceneView.cpp \
    source/SLScene_onLoad.cpp \
    source/SLShape.cpp \
    source/SLSphere.cpp \
    source/SLText.cpp \
    source/SLPhotonMapper.cpp \
    source/SLPhotonMap.cpp

OTHER_FILES += \
    ../_globals/oglsl/BumpNormal.frag \
    ../_globals/oglsl/BumpNormal.vert \
    ../_globals/oglsl/BumpNormalEarth.frag \
    ../_globals/oglsl/BumpNormalParallax.frag \
    ../_globals/oglsl/Color.frag \
    ../_globals/oglsl/ColorAttribute.vert \
    ../_globals/oglsl/ColorUniform.vert \
    ../_globals/oglsl/Diffuse.frag \
    ../_globals/oglsl/Diffuse.vert \
    ../_globals/oglsl/Earth.frag \
    ../_globals/oglsl/ErrorTex.frag \
    ../_globals/oglsl/ErrorTex.vert \
    ../_globals/oglsl/FontTex.frag \
    ../_globals/oglsl/FontTex.vert \
    ../_globals/oglsl/PerPixBlinn.frag \
    ../_globals/oglsl/PerPixBlinn.vert \
    ../_globals/oglsl/PerPixBlinnTex.frag \
    ../_globals/oglsl/PerPixBlinnTex.vert \
    ../_globals/oglsl/PerVrtBlinn.frag \
    ../_globals/oglsl/PerVrtBlinn.vert \
    ../_globals/oglsl/PerVrtBlinnTex.frag \
    ../_globals/oglsl/PerVrtBlinnTex.vert \
    ../_globals/oglsl/Reflect.frag \
    ../_globals/oglsl/Reflect.vert \
    ../_globals/oglsl/RefractReflect.frag \
    ../_globals/oglsl/RefractReflect.vert \
    ../_globals/oglsl/RefractReflectDisp.frag \
    ../_globals/oglsl/RefractReflectDisp.vert \
    ../_globals/oglsl/Terrain.frag \
    ../_globals/oglsl/Terrain.vert \
    ../_globals/oglsl/Terrain_Loesung.frag \
    ../_globals/oglsl/Terrain_Loesung.vert \
    ../_globals/oglsl/TextureOnly.frag \
    ../_globals/oglsl/TextureOnly.vert \
    ../_globals/oglsl/Wave.frag \
    ../_globals/oglsl/Wave.vert \
    ../_globals/oglsl/WaveRefractReflect.vert

