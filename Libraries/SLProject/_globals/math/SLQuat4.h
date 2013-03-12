//#############################################################################
//  File:      Globals/SLMat4.h
//  Author:    Marcus Hudritsch
//  Date:      February 2013
//  Copyright (c): 2002-2013 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLQUAT_H
#define SLQUAT_H

#include <SL.h>
#include <SLMath.h>
#include <SLMat4.h>
#include <SLVec4.h>

//-----------------------------------------------------------------------------
/*!
Quaternion class for angle-axis rotation representation. For rotations
quaternions must have unit length. See http://en.wikipedia.org/wiki/Quaternion
Quaternions can be interpolated at low cost with the method lerp or slerp.
*/
template <class T>
class SLQuat4 
{  
   public:        SLQuat4      ();
                  SLQuat4      (T x, T y, T z, T w);
                  SLQuat4      (const SLMat3<T>& m);
                  SLQuat4      (const T angleDEG, const SLVec3<T>& axis);
                  SLQuat4      (const SLVec3<T>& v0, const SLVec3<T>& v1);
                  SLQuat4      (const T pitch, const T yaw, const T roll);
    
      void        fromMat3     (const SLMat3<T>& m);
      void        fromAngleAxis(const T angleDEG, const SLVec3<T>& axis);
      void        fromEuler    (const T pitch, const T yaw, const T roll);
      void        fromVec3     (const SLVec3<T>& v0, const SLVec3<T>& v1);

      SLMat3<T>   toMat3      () const;
      SLMat4<T>   toMat4      () const;
      SLVec4<T>   toVec4      () const;
      void        toAngleAxis (T& angleDEG, SLVec3<T>& axis) const;
      
      T           dot         (const SLQuat4<T>& q) const;
      T           length      ();
      SLQuat4<T>  normalized  () const;
      T           normalize   ();
      SLQuat4<T>  inverted    () const;
      void        invert      ();
      SLQuat4<T>  conjugated  () const;
      void        conjugate   ();
      SLQuat4<T>  rotated     (const SLQuat4<T>& b) const;
      void        rotate      (const SLQuat4<T>& q);
      SLQuat4<T>  scaled      (T scale) const;
      void        scale       (T scale);
      SLQuat4<T>  lerp        (const SLQuat4<T>& q2, const T t) const;
      void        lerp        (const SLQuat4<T>& q1, 
                               const SLQuat4<T>& q2, const T t);
      SLQuat4<T>  slerp       (const SLQuat4<T>& q2, const T t) const;
      void        slerp       (const SLQuat4<T>& q1, 
                               const SLQuat4<T>& q2, const T t);
           
      SLQuat4<T>& operator=   (const SLQuat4<T> q);
      SLQuat4<T>  operator-   (const SLQuat4<T>& q) const;
      SLQuat4<T>  operator+   (const SLQuat4<T>& q) const;
      SLQuat4<T>  operator*   (const SLQuat4<T>& q) const;
      SLQuat4<T>  operator*   (const T s) const;
      SLbool      operator==  (const SLQuat4<T>& q) const;
      SLbool      operator!=  (const SLQuat4<T>& q) const;
      SLQuat4<T>& operator*=  (const SLQuat4<T>& q2);
      SLQuat4<T>& operator*=  (const T s);

   private:
      T  _x, _y, _z, _w;
};

//-----------------------------------------------------------------------------
template <class T>
inline SLQuat4<T>::SLQuat4() : _x(0), _y(0), _z(0), _w(1)
{
}

//-----------------------------------------------------------------------------
template <class T>
inline SLQuat4<T>::SLQuat4(T x, T y, T z, T w) : _x(x), _y(y), _z(z), _w(w)
{
}

//-----------------------------------------------------------------------------
template <class T>
inline SLQuat4<T>::SLQuat4(const SLMat3<T>& m)
{  fromMat3(m);
}

//-----------------------------------------------------------------------------
template <class T>
inline SLQuat4<T>::SLQuat4(const T angleDEG, const SLVec3<T>& axis)
{  fromAngleAxis(angleDEG, axis);
}

//-----------------------------------------------------------------------------
template <class T>
inline SLQuat4<T>::SLQuat4(const T pitch, const T yaw, const T roll)
{  fromEuler(pitch, yaw, roll);
}

//-----------------------------------------------------------------------------
template <class T>
void SLQuat4<T>::fromMat3 (const SLMat3<T>& m)
{
    // Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
    // article "Quaternion Calculus and Fast Animation".

    const int next[3] = {1, 2, 0};

    T trace = m.trace();
    T root;

    if (trace > (T)0)
    {
        // |_w| > 1/2, may as well choose _w > 1/2
        root = sqrt(trace + (T)1);  // 2w
        _w = ((T)0.5)*root;
        root = ((T)0.5)/root;       // 1/(4w)
        _x = (m(2,1) - m(1,2)) * root;
        _y = (m(0,2) - m(2,0)) * root;
        _z = (m(1,0) - m(0,1)) * root;
    }
    else
    {
        // |_w| <= 1/2
        int i = 0;
        if (m(1,1) > m(0,0)) i = 1;
        if (m(2,2) > m(i,i)) i = 2;
        int j = next[i];
        int k = next[j];

        root = sqrt(m(i,i) - m(j,j) - m(k,k) + (T)1);
        T* quat[3] = { &_x, &_y, &_z };
        *quat[i] = ((T)0.5)*root;
        root = ((T)0.5)/root;
        _w = (m(k,j) - m(j,k))*root;
        *quat[j] = (m(j,i) + m(i,j))*root;
        *quat[k] = (m(k,i) + m(i,k))*root;
    }
}

//-----------------------------------------------------------------------------
template <class T>
void SLQuat4<T>::fromVec3(const SLVec3<T>& v0, const SLVec3<T>& v1)
{  
   // Code from "The Shortest Arc Quaternion" 
   // by Stan Melax in "Game Programming Gems".
   if (v0 == -v1)
   {  fromAxisAngle(SLVec3<T>(1, 0, 0), SL_PI);
      return;
   }
      
   SLVec3<T> c = v0.Cross(v1);
   T d = v0.dot(v1);
   T s = sqrt((1 + d) * (T)2);

   _x = c._x / s;
   _y = c._y / s;
   _z = c._z / s;
   _w = s * (T)0.5;
}

//-----------------------------------------------------------------------------
template <class T>
void SLQuat4<T>::fromAngleAxis(const T angleDEG, const SLVec3<T>& axis)
{
   T angleRAD = angleDEG * SL_DEG2RAD;
   _w = (T)cos(angleRAD * (T)0.5);
   _x = _y = _z = (T)sin(angleRAD * (T)0.5);
   _x *= axis.x;
   _y *= axis.y;
   _z *= axis.z;
}

//-----------------------------------------------------------------------------
template <class T>
void SLQuat4<T>::fromEuler(T pitch, T yaw, T roll)
{
	// Basically we create 3 Quaternions, one for pitch, one for yaw, one for roll
	// and multiply those together.
	// The calculation below does the same, just shorter
 
	T p = pitch * SL_DEG2RAD * (T)0.5;
	T y = yaw   * SL_DEG2RAD * (T)0.5;
	T r = roll  * SL_DEG2RAD * (T)0.5;
 
	T sinp = (T)sin(p);
	T siny = (T)sin(y);
	T sinr = (T)sin(r);
	T cosp = (T)cos(p);
	T cosy = (T)cos(y);
	T cosr = (T)cos(r);
 
	_x = sinr * cosp * cosy - cosr * sinp * siny;
	_y = cosr * sinp * cosy + sinr * cosp * siny;
	_z = cosr * cosp * siny - sinr * sinp * cosy;
	_w = cosr * cosp * cosy + sinr * sinp * siny;
 
	normalize();
}

//-----------------------------------------------------------------------------
template <class T>
SLMat3<T> SLQuat4<T>::toMat3() const
{
   T  x2 = _x *(T)2;  
   T  y2 = _y *(T)2;  
   T  z2 = _z *(T)2;

   T wx2 = _w * x2;  T wy2 = _w * y2;  T wz2 = _w * z2;
   T xx2 = _x * x2;  T xy2 = _x * y2;  T xz2 = _x * z2;
   T yy2 = _y * y2;  T yz2 = _y * z2;  T zz2 = _z * z2;

   SLMat3<T> m(1 -(yy2 + zz2),    xy2 - wz2,     xz2 + wy2,
                   xy2 + wz2, 1 -(xx2 + zz2),    yz2 - wx2,
                   xz2 - wy2,     yz2 + wx2, 1 -(xx2 + yy2));
   return m;
}

//-----------------------------------------------------------------------------
template <class T>
SLMat4<T> SLQuat4<T>::toMat4() const
{
   T  x2 = _x *(T)2;  
   T  y2 = _y *(T)2;  
   T  z2 = _z *(T)2;

   T wx2 = _w * x2;  T wy2 = _w * y2;  T wz2 = _w * z2;
   T xx2 = _x * x2;  T xy2 = _x * y2;  T xz2 = _x * z2;
   T yy2 = _y * y2;  T yz2 = _y * z2;  T zz2 = _z * z2;

   SLMat4<T> m(1 -(yy2 + zz2),    xy2 - wz2,     xz2 + wy2,  0,
                   xy2 + wz2, 1 -(xx2 + zz2),    yz2 - wx2,  0,
                   xz2 - wy2,     yz2 + wx2, 1 -(xx2 + yy2), 0,
                           0,             0,              0, 1);
   return m;
}

//-----------------------------------------------------------------------------
template <class T>
inline SLVec4<T> SLQuat4<T>::toVec4() const
{
   return SLVec4<T>(_x, _y, _z, _w);
}

//-----------------------------------------------------------------------------
template <typename T>
void SLQuat4<T>::toAngleAxis (T& angleDEG, SLVec3<T>& axis) const
{
    // The quaternion representing the rotation is
    // q = cos(A/2) + sin(A/2)*(_x*i+_y*j+_z*k)

    T sqrLen = _x*_x + _y*_y + _z*_z;

    if (sqrLen > SL_EPSILON)
    {
        angleDEG = ((T)2) * acos(_w) * SL_RAD2DEG;
        T len = sqrt(sqrLen);
        axis.x = _x / len;
        axis.y = _y / len;
        axis.z = _z / len;
    }
    else
    {
        // Angle is 0 (mod 2*pi), so any axis will do.
        angleDEG = (T)0;
        axis.x   = (T)1;
        axis.y   = (T)0;
        axis.z   = (T)0;
    }
}

//-----------------------------------------------------------------------------
template<class T>
SLQuat4<T>& SLQuat4<T>::operator= (const SLQuat4<T> q)
{  _x = q._x;
   _y = q._y;
   _z = q._z;
   _w = q._w;
   return(*this);
}
//-----------------------------------------------------------------------------
template <class T>
SLQuat4<T> SLQuat4<T>::operator- (const SLQuat4<T>& q) const
{
   return SLQuat4<T>(_x - q._x, _y - q._y, _z - q._z, _w - q._w);
}

//-----------------------------------------------------------------------------
template <class T>
SLQuat4<T> SLQuat4<T>::operator+ (const SLQuat4<T>& q) const
{
   return SLQuat4<T>(_x + q._x, _y + q._y, _z + q._z, _w + q._w);
}

//-----------------------------------------------------------------------------
template <class T>
SLQuat4<T> SLQuat4<T>::operator* (const SLQuat4<T>& q) const
{
   return rotated(q);
}

//-----------------------------------------------------------------------------
template <class T>
SLQuat4<T> SLQuat4<T>::operator* (const T s) const
{
   return scaled(s);
}

//-----------------------------------------------------------------------------
template <class T>
bool SLQuat4<T>::operator== (const SLQuat4<T>& q) const
{
   return _x == q._x && _y == q._y && _z == q._z && _w == q._w;
}

//-----------------------------------------------------------------------------
template <class T>
bool SLQuat4<T>::operator!= (const SLQuat4<T>& q) const
{
   return !(*this == q);
}
//-----------------------------------------------------------------------------
template<class T>
SLQuat4<T>& SLQuat4<T>::operator*= (const SLQuat4<T>& q2)
{  
   SLQuat4<T> q;
   SLQuat4<T>& q1 = *this;
    
   q._w = q1._w * q2._w - q1._x * q2._x - q1._y * q2._y - q1._z * q2._z;
   q._x = q1._w * q2._x + q1._x * q2._w + q1._y * q2._z - q1._z * q2._y;
   q._y = q1._w * q2._y + q1._y * q2._w + q1._z * q2._x - q1._x * q2._z;
   q._z = q1._w * q2._z + q1._z * q2._w + q1._x * q2._y - q1._y * q2._x;
    
   q.normalize();
   return *this;
}
//-----------------------------------------------------------------------------
template<class T>
SLQuat4<T>& SLQuat4<T>::operator*= (const T s)
{  
    _x *= s;
    _y *= s;
    _z *= s;
    _w *= s;
   return *this;
}

//-----------------------------------------------------------------------------
template <class T>
inline T SLQuat4<T>::dot(const SLQuat4<T>& q) const
{
    return _x * q._x + _y * q._y + _z * q._z + _w * q._w;
}

//-----------------------------------------------------------------------------
template <class T>
inline T SLQuat4<T>::length()
{
    return sqrt(_x*_x + _y*_y + _z*_z + _w*_w);
}

//-----------------------------------------------------------------------------
template <class T>
SLQuat4<T> SLQuat4<T>::normalized() const
{
   T len = length();
   SLQuat4<T> normalized;

   if (len > SL_EPSILON)
   {
      T invLen = ((T)1)/len;
      normalized._x = _x *= invLen;
      normalized._y = _y *= invLen;
      normalized._z = _z *= invLen;
      normalized._w = _w *= invLen;
   } else
   {  // set invalid result to flag the error.
      normalized._x = (T)0;
      normalized._y = (T)0;
      normalized._z = (T)0;
      normalized._w = (T)0;
   }

   return normalized;
}

//-----------------------------------------------------------------------------
template <class T>
inline T SLQuat4<T>::normalize()
{
   T len = length();

   if (len > SL_EPSILON)
   {
      T invLen = ((T)1)/len;
      _x *= invLen;
      _y *= invLen;
      _z *= invLen;
      _w *= invLen;
   } else
   {  // set invalid result to flag the error.
      len = (T)0;
      _x = (T)0;
      _y = (T)0;
      _z = (T)0;
      _w = (T)0;
   }

   return len;
}

//----------------------------------------------------------------------------
template <class T>
SLQuat4<T> SLQuat4<T>::inverted () const
{
   SLQuat4<T> inverse;
   T norm = _x*_x + _y*_y + _z*_z + _w*_w;

   if (norm > (T)0)
   {  
      // for non-unit quaternions we have to normalize
      T invNorm  = ((T)1) / norm;
      inverse._x = -_x * invNorm;
      inverse._y = -_y * invNorm;
      inverse._z = -_z * invNorm;
      inverse._w =  _w * invNorm;
   } else
   {  
      // return an invalid result to flag the error.
      inverse._x = (T)0;
      inverse._y = (T)0;
      inverse._z = (T)0;
      inverse._w = (T)0;
   }

   return inverse;
}

//----------------------------------------------------------------------------
template <class T>
void SLQuat4<T>::invert()
{
   T norm = _x*_x + _y*_y + _z*_z + _w*_w;

   if (norm > (T)0)
   {  
      // for non-unit quaternions we have to normalize
      T invNorm  = ((T)1) / norm;
      _x = -_x * invNorm;
      _y = -_y * invNorm;
      _z = -_z * invNorm;
      _w =  _w * invNorm;
   } else
   {  
      // return an invalid result to flag the error.
      _x = (T)0;
      _y = (T)0;
      _z = (T)0;
      _w = (T)0;
   }
}

//----------------------------------------------------------------------------
template <class T>
SLQuat4<T> SLQuat4<T>::conjugated() const
{  
   // for a unit quaternion the conjugate is equal to the inverse
   return SLQuat4(-_x, -_y, -_z, _w);
}

//----------------------------------------------------------------------------
template <class T>
void SLQuat4<T>::conjugate()
{  
   // for a unit quaternion the conjugate is equal to the inverse
   _x = -_x;
   _y = -_y;
   _z = -_z;
}

//-----------------------------------------------------------------------------
template <class T>
inline SLQuat4<T> SLQuat4<T>::rotated(const SLQuat4<T>& b) const
{
   SLQuat4<T> q;
   q._w = _w*b._w - _x*b._x - _y*b._y - _z*b._z;
   q._x = _w*b._x + _x*b._w + _y*b._z - _z*b._y;
   q._y = _w*b._y + _y*b._w + _z*b._x - _x*b._z;
   q._z = _w*b._z + _z*b._w + _x*b._y - _y*b._x;
   q.normalize();
   return q;
}

//-----------------------------------------------------------------------------
template <class T>
inline void SLQuat4<T>::rotate(const SLQuat4<T>& q2)
{
   SLQuat4<T> q;
   SLQuat4<T>& q1 = *this;
    
   q._w = q1._w * q2._w - q1._x * q2._x - q1._y * q2._y - q1._z * q2._z;
   q._x = q1._w * q2._x + q1._x * q2._w + q1._y * q2._z - q1._z * q2._y;
   q._y = q1._w * q2._y + q1._y * q2._w + q1._z * q2._x - q1._x * q2._z;
   q._z = q1._w * q2._z + q1._z * q2._w + q1._x * q2._y - q1._y * q2._x;
    
   q.normalize();
   *this = q;
}

//-----------------------------------------------------------------------------
template <class T>
inline SLQuat4<T> SLQuat4<T>::scaled(T s) const
{
    return SLQuat4<T>(_x * s, _y * s, _z * s, _w * s);
}

//-----------------------------------------------------------------------------
template <class T>
inline void SLQuat4<T>::scale(T s)
{
    _x *= s;
    _y *= s;
    _z *= s;
    _w *= s;
}

//-----------------------------------------------------------------------------
//! Linear interpolation
template <class T>
inline SLQuat4<T> SLQuat4<T>::lerp(const SLQuat4<T>& q2, const T t) const
{  
   SLQuat4<T> q = scaled(1-t) + q2.scaled(t);
   q.normalize();
   return q;
}
//-----------------------------------------------------------------------------
//! Linear interpolation
template <class T>
inline void SLQuat4<T>::lerp(const SLQuat4<T>& q1, 
                             const SLQuat4<T>& q2, const T t)
{  
   *this = q1.scaled(1-t) + q2.scaled(t);
   normalize();
}

//-----------------------------------------------------------------------------
//! Spherical linear interpolation
template <class T>
inline SLQuat4<T> SLQuat4<T>::slerp(const SLQuat4<T>& q2, const T t) const
{
   // Ken Shoemake's famous method.
	assert(t>=0 && t<=1 && "Wrong t in SLQuat4::slerp");

   T cosAngle = dot(q2);
    
   if (cosAngle > 1 - SL_EPSILON) 
   {
      SLQuat4<T> result = q2 + (*this - q2).scaled(t);
      result.normalize();
      return result;
   }
    
   if (cosAngle < 0) cosAngle = 0;
   if (cosAngle > 1) cosAngle = 1;
    
   T theta0 = acos(cosAngle);
   T theta  = theta0 * t;
    
   SLQuat4<T> v2 = (q2 - scaled(cosAngle));
   v2.normalize();
    
   SLQuat4<T> q = scaled(cos(theta)) + v2.scaled(sin(theta));
   q.normalize();
   return q;
}

//-----------------------------------------------------------------------------
//! Spherical linear interpolation
template <class T>
inline void SLQuat4<T>::slerp(const SLQuat4<T>& q1,
                              const SLQuat4<T>& q2, const T t)
{
   // Ken Shoemake's famous method.
	assert(t>=0 && t<=1 && "Wrong t in SLQuat4::slerp");

   T cosAngle = q1.dot(q2);
    
   if (cosAngle > 1 - SL_EPSILON) 
   {
      *this = q2 + (q1 - q2).scaled(t);
      normalize();
      return;
   }
    
   if (cosAngle < 0) cosAngle = 0;
   if (cosAngle > 1) cosAngle = 1;
    
   T theta0 = acos(cosAngle);
   T theta  = theta0 * t;
    
   SLQuat4<T> v2 = (q2 - q1.scaled(cosAngle));
   v2.normalize();
    
   *this = q1.scaled(cos(theta)) + v2.scaled(sin(theta));
   normalize();
}

//-----------------------------------------------------------------------------
typedef SLQuat4<SLfloat> SLQuat4f;
//-----------------------------------------------------------------------------
#endif
