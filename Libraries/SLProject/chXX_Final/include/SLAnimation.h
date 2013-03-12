#ifndef SLANIMATION_H
#define SLANIMATION_H

#include <stdafx.h>
#include <SLKeyframe.h>

class SLShape;
class SLCurve;
//-----------------------------------------------------------------------------
//! Enumeration for animation modes
typedef enum
{  once,          //!< foreward once
   loop,          //!< foreward loop
   pingPong,      //!< forwards and backwards
   pingPongLoop   //!< forwards and backwards loop
} SLAnimMode;
//-----------------------------------------------------------------------------
//! Enumeration for animation easing curves
/*! 
Enumatrations copied from Qt class QEasingCurve. 
See http://qt-project.org/doc/qt-4.8/qeasingcurve.html#Type-enum
*/
typedef enum 
{  linear,     //!< linear easing with constant velocity
   inQuad,     //!< quadratic easing in, acceleration from zero velocity
   outQuad,    //!< quadratic easing out, decelerating to zero velocity
   inOutQuad,  //!< quadratic easing in and then out  
   outInQuad,  //!< quadratic easing out and then in
   inCubic,    //!< qubic in easing in, acceleration from zero velocity
   outCubic,   //!< qubic easing out, decelerating to zero velocity
   inOutCubic, //!< qubic easing in and then out 
   outInCubic, //!< qubic easing out and then in
   inQuart,    //!< quartic easing in, acceleration from zero velocity
   outQuart,   //!< quartic easing out, decelerating to zero velocity
   inOutQuart, //!< quartic easing in and then out 
   outInQuart, //!< quartic easing out and then in
   inQuint,    //!< quintic easing in, acceleration from zero velocity
   outQuint,   //!< quintic easing out, decelerating to zero velocity
   inOutQuint, //!< quintic easing in and then out 
   outInQuint, //!< quintic easing out and then in
   inSine,     //!< sine ieasing in, acceleration from zero velocity
   outSine,    //!< sine easing out, decelerating to zero velocity
   inOutSine,  //!< sine easing in and then out  
   outInSine   //!< sine easing out and then in
} SLEasingCurve;
//-----------------------------------------------------------------------------
//! SLAnimation implements simple keyframe animation.
/*!
An animation consists out of at least two keyframes. A keyframe is defines 
position (translation), rotation and scaling for a specific time point with in 
an animation. An animation interpolates between two keyframes the position, 
rotation and scaling.
The first keyframe's time will allways be 0. The _totalTime of an animation in
seconds is the sum of all keyframe times.
The animation is applied to the owner shapes local transform matrix in 
animate(SLShape* shape, SLfloat elapsedTimeSec) proportional to the elapsed
time of the frame.
The animation mode [once, loop, pingPong, pingPongLoop] determines what happens
if the total animation time is over.
Because the transform matrices of the owner shape is modified during animation,
the animation instance keep the original owner matrices in _om and _wm.
The animation easing determines how constant the velocity of the animation. It
is defined by an easing curve that is by default set to a linear motion with 
a constant speed. See the declaration of SLEasingCurve for all possible easing
curves.
*/
class SLAnimation : SLObject
{
   public:           //! Multiple keyframes animation ctor
                     SLAnimation (SLVKeyframe keyframes,
                                  SLVec3f* controls = 0,
                                  SLAnimMode mode = once, 
                                  SLEasingCurve easing = linear,
                                  SLstring name="myKeyframesAnimation");
                                  
                     //! Single keyframe animation ctor
                     SLAnimation (SLKeyframe keyframe,
                                  SLAnimMode mode = once,
                                  SLEasingCurve easing = linear, 
                                  SLstring name="myKeyframeAnimation");

                     //! Single translation animation
                     SLAnimation (SLfloat time,
                                  SLVec3f translation,
                                  SLAnimMode mode = once,
                                  SLEasingCurve easing = linear, 
                                  SLstring name="myTranslationAnimation");
                     
                     //! Single rotation animation
                     SLAnimation (SLfloat time,
                                  SLfloat angleDEG,
                                  SLVec3f rotationAxis,
                                  SLAnimMode mode = once,
                                  SLEasingCurve easing = linear,
                                  SLstring name="myRotationAnimation");
                     
                     //! Single scaling animation
                     SLAnimation (SLfloat time,
                                  SLfloat scaleX,
                                  SLfloat scaleY,
                                  SLfloat scaleZ,
                                  SLAnimMode mode = once,
                                  SLEasingCurve easing = linear,
                                  SLstring name="myScalingAnimation");
                     
                     //! Elliptic animation with 2 radiis on 2 axis
                     SLAnimation (SLfloat time,
                                  SLfloat radiusA, SLAxis axisA,
                                  SLfloat radiusB, SLAxis axisB,
                                  SLAnimMode mode = once,
                                  SLEasingCurve easing = linear,
                                  SLstring name="myEllipticAnimation");

                    ~SLAnimation ();

      void           init        (SLVec3f* controls);
	   void           animate     (SLShape* shape, SLfloat elapsedTimeSec);
      void           drawWS      ();

      SLfloat        totalTime   () {return _totalTime;}
      SLbool         isFinished  () {return _isFinished;}

   private:
      SLfloat        easing      (SLfloat time);

      SLVKeyframe    _keyframes;    //!< Vector with keyframes
      SLCurve*       _curve;        //!< Interpolation curve for translation
      SLfloat        _totalTime;    //!< Total cummultated time in seconds 
      SLfloat        _currentTime;  //!< Current time in seconds during anim.
      SLbool         _isFinished;   //!< Flag if animation is finished
      SLAnimMode     _mode;         //!< Animation mode
      SLEasingCurve  _easing;       //!< Easing curve type
      SLfloat        _direction;    //!< Direction 1=forewards -1=backwards
      SLMat4f        _om;           //!< Original object transform matrix
      SLMat4f        _wm;           //!< Original world transform matrix
};
//-----------------------------------------------------------------------------
#endif
