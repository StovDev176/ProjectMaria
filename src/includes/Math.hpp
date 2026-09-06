#pragma once
#include <iostream>
#include <cmath>
#include "raylib.h"
#include <algorithm>
#include "raymath.h"
#include <cstring>
struct vector3 {
  float x, y, z;

  vector3() : x(0.0f), y(0.0f), z(0.0f) {} 

  vector3(float x, float y, float z) : x(x), y(y), z(z) {}
  
  float Magnitude() const {
    return std::sqrt(x*x + y*y + z*z);
  }
  
  vector3 Unit() const {
    float mag = Magnitude();
    return vector3(x/mag, y/mag, z/mag);
  }

  vector3 operator+(const vector3& other) const {
    return vector3(x + other.x, y + other.y, z + other.z);
  }
  
  vector3 operator-(const vector3& other) const {
    return vector3(x - other.x, y - other.y, z - other.z);
  }
  
  vector3 operator*(float scalar) const {
    return vector3(x * scalar, y * scalar, z * scalar);
  }

  vector3 operator/(float scalar) const {
    return vector3(x / scalar, y / scalar, z / scalar);
  }

  float Dot(const vector3& other) const {
    return ((x * other.x) + (y * other.y) + (z * other.z));
  }

  vector3 Lerp(const vector3& other, float t) const {
    return vector3(x + (other.x - x) * t, y + (other.y - y) * t, z + (other.z - z) * t);
  }

  void operator+=(const vector3& other) {
    x += other.x;
    y += other.y;
    z += other.z;
  }
  void operator-=(const vector3& other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
  }
  operator Vector3() const {
        return Vector3{ x, y, z };
  }
};

std::ostream& operator<<(std::ostream& os, const vector3& v) {
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}

struct Matrix4 {
    float m[16];

    explicit Matrix4(bool makeIdentity) {
        if (makeIdentity) {
          *this = Matrix4();
        } else {
          Zero();
        }
    }

    Matrix4() {
      m[0]  = 1.0f; m[1]  = 0.0f; m[2]  = 0.0f; m[3]  = 0.0f;
      m[4]  = 0.0f; m[5]  = 1.0f; m[6]  = 0.0f; m[7]  = 0.0f;
      m[8]  = 0.0f; m[9]  = 0.0f; m[10] = 1.0f; m[11] = 0.0f;
      m[12] = 0.0f; m[13] = 0.0f; m[14] = 0.0f; m[15] = 1.0f;
    }
    Matrix4(
        float m0, float m4, float m8,  float m12,
        float m1, float m5, float m9,  float m13,
        float m2, float m6, float m10, float m14,
        float m3, float m7, float m11, float m15
    ) {
        m[0] = m0; m[4] = m4; m[8]  = m8;  m[12] = m12;
        m[1] = m1; m[5] = m5; m[9]  = m9;  m[13] = m13;
        m[2] = m2; m[6] = m6; m[10] = m10; m[14] = m14;
        m[3] = m3; m[7] = m7; m[11] = m11; m[15] = m15;
    }

    static Matrix4 Identity() {
        return Matrix4();
    }

    void Zero() {
      for (int i = 0; i < 16; ++i) {
        m[i] = 0.0f;
      }
    }

    float& operator[](int index) { return m[index];}
    static Matrix4 CreateTranslation(const vector3& pos) {
      Matrix4 result;
        result.m[12] = pos.x; 
        result.m[13] = pos.y;
        result.m[14] = pos.z;
        return result;
    }

    static Matrix4 CreateScale(const vector3& scale) {
      Matrix4 result;
      result.m[0] = scale.x;
      result.m[5] = scale.y;
      result.m[10] = scale.z;
      return result;
    }
    static Matrix4 CreateRotationX(float angleRad) {
      Matrix4 result;
      result.m[5] = cos(angleRad);
      result.m[6] = -sin(angleRad);
      result.m[9] = sin(angleRad);
      result.m[10] = cos(angleRad);
      return result;
    }
    static Matrix4 CreateRotationY(float angleRad) {
      Matrix4 result;
      result.m[0] = cos(angleRad);
      result.m[2] = sin(angleRad);
      result.m[8] = -sin(angleRad);
      result.m[10] = cos(angleRad);
      return result;
    }
    static Matrix4 CreateRotationZ(float angleRad) {
      Matrix4 result;
      result.m[0] = cos(angleRad);
      result.m[1] = sin(angleRad);
      result.m[4] = -sin(angleRad);
      result.m[5] = cos(angleRad);
      return result;
    }
    static Matrix4 CreateRotationXYZ(float angleRadX, float angleRadY, float angleRadZ) {
      Matrix4 result;
      Matrix4 rotX = CreateRotationX(angleRadX);
      Matrix4 rotY = CreateRotationY(angleRadY);
      Matrix4 rotZ = CreateRotationZ(angleRadZ);
      return rotX * rotY * rotZ;
    }
    static Matrix4 CreateRotationEuler(const vector3& rotationRad) {
      Matrix4 result;
      float cx = cos(rotationRad.x);
      float sx = sin(rotationRad.x);
      float cy = cos(rotationRad.y);
      float sy = sin(rotationRad.y);
      float cz = cos(rotationRad.z);
      float sz = sin(rotationRad.z);

      result.m[0] = cy * cz;
      result.m[1] = cx * sz + sx * sy * cz;
      result.m[2] = sx * sz - cx * sy * cz;
      result.m[4] = -cy * sz;
      result.m[5] = cx * cz - sx * sy * sz;
      result.m[6] = sx * cz + cx * sy * sz;
      result.m[8] = sy;
      result.m[9] = -sx * cy;
      result.m[10] = cx * cy;
      return result;
    }
    static Matrix4 CreateTRS(const vector3& pos, const vector3& rotEulerRad, const vector3& scale) {
      Matrix4 t = CreateTranslation(pos);
      Matrix4 r = CreateRotationEuler(rotEulerRad);
      Matrix4 s = CreateScale(scale);

      return t * r * s; 
    }

    Matrix4 Invert() const {
      Matrix rlMat;
      std::memcpy(&rlMat, m, sizeof(float) * 16);

      Matrix rlInv = MatrixInvert(rlMat);

      Matrix4 result;
      std::memcpy(result.m, &rlInv, sizeof(float) * 16);
      return result;
    }

    Matrix4 operator*(const Matrix4& b) const {
      Matrix4 result(false); 

      result.m[0]  = m[0]*b.m[0]  + m[4]*b.m[1]  + m[8]*b.m[2]   + m[12]*b.m[3];
      result.m[1]  = m[1]*b.m[0]  + m[5]*b.m[1]  + m[9]*b.m[2]   + m[13]*b.m[3];
      result.m[2]  = m[2]*b.m[0]  + m[6]*b.m[1]  + m[10]*b.m[2]  + m[14]*b.m[3];
      result.m[3]  = m[3]*b.m[0]  + m[7]*b.m[1]  + m[11]*b.m[2]  + m[15]*b.m[3];

      result.m[4]  = m[0]*b.m[4]  + m[4]*b.m[5]  + m[8]*b.m[6]   + m[12]*b.m[7];
      result.m[5]  = m[1]*b.m[4]  + m[5]*b.m[5]  + m[9]*b.m[6]   + m[13]*b.m[7];
      result.m[6]  = m[2]*b.m[4]  + m[6]*b.m[5]  + m[10]*b.m[6]  + m[14]*b.m[7];
      result.m[7]  = m[3]*b.m[4]  + m[7]*b.m[5]  + m[11]*b.m[6]  + m[15]*b.m[7];

      result.m[8]  = m[0]*b.m[8]  + m[4]*b.m[9]  + m[8]*b.m[10]  + m[12]*b.m[11];
      result.m[9]  = m[1]*b.m[8]  + m[5]*b.m[9]  + m[9]*b.m[10]  + m[13]*b.m[11];
      result.m[10] = m[2]*b.m[8]  + m[6]*b.m[9]  + m[10]*b.m[10] + m[14]*b.m[11];
      result.m[11] = m[3]*b.m[8]  + m[7]*b.m[9]  + m[11]*b.m[10] + m[15]*b.m[11];

      result.m[12] = m[0]*b.m[12] + m[4]*b.m[13] + m[8]*b.m[14]  + m[12]*b.m[15];
      result.m[13] = m[1]*b.m[12] + m[5]*b.m[13] + m[9]*b.m[14]  + m[13]*b.m[15];
      result.m[14] = m[2]*b.m[12] + m[6]*b.m[13] + m[10]*b.m[14] + m[14]*b.m[15];
      result.m[15] = m[3]*b.m[12] + m[7]*b.m[13] + m[11]*b.m[14] + m[15]*b.m[15];

      return result;
  }

  Matrix4& operator*=(const Matrix4& b) {
    *this = *this * b;
    return *this;
  }
  vector3 GetTranslation() const {
    return vector3(m[12], m[13], m[14]);
  }
  vector3 GetScale() const {
    float scaleX = std::sqrt(m[0] * m[0] + m[1] * m[1] + m[2] * m[2]);
    float scaleY = std::sqrt(m[4] * m[4] + m[5] * m[5] + m[6] * m[6]);
    float scaleZ = std::sqrt(m[8] * m[8] + m[9] * m[9] + m[10] * m[10]);
    return vector3(scaleX, scaleY, scaleZ);
  }
  vector3 GetRotation() const {
    vector3 scale = GetScale();

    if (scale.x == 0.0f || scale.y == 0.0f || scale.z == 0.0f) {
        return vector3(0.0f, 0.0f, 0.0f);
    }

    float r0 = m[0] / scale.x;  float r4 = m[4] / scale.y;  float r8  = m[8]  / scale.z;
    float r1 = m[1] / scale.x;  float r5 = m[5] / scale.y;  float r9  = m[9]  / scale.z;
    float r2 = m[2] / scale.x;  float r6 = m[6] / scale.y;  float r10 = m[10] / scale.z;

    vector3 rotation;

    float sinY = std::clamp(r8, -1.0f, 1.0f);
    rotation.y = std::asin(sinY);

    if (std::abs(sinY) < 0.99999f) {
        rotation.x = std::atan2(-r9, r10);
        rotation.z = std::atan2(-r4, r0);
    } else {
        rotation.x = std::atan2(r1, r5);
        rotation.z = 0.0f;
    }

    return rotation; 
}
};

float cubicLerp(float t) {
  return t * t * (3.0f - 2.0f * t);
}

float sineLerp(float t) {
  return -(std::cos(3.14159f * t) - 1.0f) / 2.0f;
}

inline vector3 raylibVector3To_vector3(const Vector3& vec) {
  return vector3(vec.x, vec.y, vec.z);
}
