//#############################################################################
//  File:      SLNode.h
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLNODE_H
#define SLNODE_H

#include <stdafx.h>

class SLSceneView;
class SLGroup;
class SLRay;
class SLAABBox;

//-----------------------------------------------------------------------------
//! Virtual base class for nodes in a doubled linked list for the scenegraph
/*!      
SLNode is a virtual base class for all other node classes. A node can be linked
to a previous and a next node. If a node is a children node the _parent pointer
will point to the parent group. For a minimal hierachical scenegraph structur
we would only need the pointer to the next sibling node. But for simpler and
faster algorithms we need the pointer to the previous node and the parent group.
The virtual method init, draw, copy and updateStats must be implemented by all 
deriving nodes.
*/
class SLNode: public SLObject
{  public:
                        SLNode(SLstring name) : SLObject(name)
                        {  _next = _prev = 0;
                           _parent = 0;
                           _refs = _depth = 0;
                           stateGL = SLGLState::getInstance();
                        }
      virtual          ~SLNode() {}
      
      SLGLState*        stateGL;    //!< pointer to the global SLGLState instance
      
      /*! 
      Initializes a node for its OpenGL rendering. Because a rendering context
      must be present for this purpose, inheriting nodes should not call this
      method within the constructor. The parameter currentDepth passes the 
      _actual depth of the node within the entire scenegraph tree.
      */
      virtual void      init        (SLSceneView* sv, 
                                     SLint currentDepth) = 0;
      /*!
      calls the animation of each individual object
      */
      virtual SLbool    animate     (SLfloat time) = 0;
      /*!
      draw is the OpenGL rendering method that is called when the scene graph is
      traversed. Every node of the scene, visible or invisible, group or leaf 
      node must implement this virtual function defined in this base class.
      */
      virtual void      draw        (SLSceneView* sv) = 0;
      /*!
      returns a copy of itself.
      */
      virtual SLNode*   copy        () = 0;
      /*! 
      Updates the statistic counters of the parent node
      */
      virtual void      updateStats (SLGroup* parent) = 0; 
      /*!
      buildAABB calculates a bounding box around shape nodes.
      */
      virtual SLAABBox& buildAABB  () = 0;    
      /*!
      updateAABB updates the bounding box around shape nodes.
      */
      virtual SLAABBox& updateAABB () = 0;  
      /*!
      update world matrix for shapes where the local matrix or an ancestors matrix has changed.
      */
      virtual void      updateWM    (SLbool ancestorGotUpdated) = 0;
      /*!
      cull is frustum culling traversal to determine the shapes inside the
      view frustum. This method is called after update and before draw.
      */
      virtual void      cull        (SLSceneView* sv) = 0;
      /*!
      intersection method for a ray with a node
      */
      virtual SLbool    hit         (SLRay* ray) = 0; 
      /*!
      preShade calculates the texture coordinates of the intersection 
      point and the resulting color. It is called from within the shade method
      */
      virtual void      preShade    (SLRay* ray) = 0;

      // Setters
               void     next        (SLNode* next)    {_next = next;}
               void     prev        (SLNode* prev)    {_prev = prev;}
               void     parent      (SLNode* parent)  {_parent = parent;}
               void     incRefs     ()                {_refs++;}
               void     decRefs     ()                {_refs--;}
               void     depth       (SLint depth)     {_depth = depth;}
                        
      // Getters        
               SLNode*  next        () {return _next;}
               SLNode*  prev        () {return _prev;}
               SLNode*  parent      () {return _parent;}
               SLint    refs        () {return _refs;}
               SLint    depth       () {return _depth;}
               
   private:
               SLNode*  _next;      //!< pointer to next node in the group
               SLNode*  _prev;      //!< pointer to previous node in the group
               SLNode*  _parent;    //!< pointer to parent group 
               SLint    _refs;      //!< counter for references to this node
               SLint    _depth;     //!< depth of the node in a scene tree
};
//-----------------------------------------------------------------------------
#endif
