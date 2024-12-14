#ifndef VECTOR_H
#define VECTOR_H

class Vertex
{
 public:
  
  float x, y, z;

  Vertex() : x(0), y(0), z(0)
  {
  }
  
  Vertex(float _x, float _y, float _z) : x(_x), y(_y), z(_z)
  {
  }

};

#endif // VECTOR_H