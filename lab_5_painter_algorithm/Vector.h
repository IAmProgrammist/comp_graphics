#ifndef VECTOR_H
#define VECTOR_H

#include "Matrix.h"

class Vector
{
 public:
  
  double x, y, z, w;

  Vector() : x(0), y(0), z(0), w(1.0f)
  {
  }
  
  Vector(double _x, double _y, double _z, double _w = 1.0f) : x(_x), y(_y), z(_z), w(_w)
  {
  }


  Vector operator * (const Matrix& A) const
  {
      Vector E;
      E.x = x * A.M[0][0] + y * A.M[1][0] + z * A.M[2][0] + w * A.M[3][0];
      E.y = x * A.M[0][1] + y * A.M[1][1] + z * A.M[2][1] + w * A.M[3][1];
      E.z = x * A.M[0][2] + y * A.M[1][2] + z * A.M[2][2] + w * A.M[3][2];
      E.w = x * A.M[0][3] + y * A.M[1][3] + z * A.M[2][3] + w * A.M[3][3];
      return E;
  }

};

#endif // VECTOR_H