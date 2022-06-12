#ifndef VMATH_H
#define VMATH_H
typedef float f32;
#define FLOAT_MIN 0.0001

#define PI 3.14159

#include <math.h>



#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__clang__)
#include <x86intrin.h>
#else 
#error No valid compiler available for build!
#end
#endif

struct Vector2 {
    union {
        struct {
            f32 x, y;
        };
        f32 data[2];
    };
    
};

struct Vector3 {
    union {
        struct {
            f32 x,y,z;
        };
        f32 arr[3];
        f32 data[3];
    };
    Vector3() {}
    Vector3(f32 x, f32 y, f32 z) : x(x), y(y), z(z){}
    f32& operator[](int index) {
        return data[index];
    }
    Vector3& operator*= (const Vector3& rhs) {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        return (*this);
    }
    Vector3& operator*= (const f32 scale) {
        x *= scale;
        y *= scale;
        z *= scale;
        return (*this);
    }

    void normalize(){
        f32 mag = sqrt(z*z + y*y + x*x);
        x /= mag; y /= mag; z /= mag;
    }

    f32 mag() const {
        return  sqrt(z*z + y*y + x*x);
    }

    f32 dist(const Vector3& other) {
        f32 dx = other.x - x, dy =  other.y - y, dz = other.z - z;
        return sqrt((dx*dx + dy*dy + dz*dz));
    }
    
};

struct Vector4 {
    union {
        struct {
            f32 x,y,z,w;
        };
        f32 data[4];
        f32 arr[4];
    };
    Vector4(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}
    Vector4(f32 f) {
        arr[0] = f; arr[1] = f; arr[2] = f; arr[3] = f;
    }
    Vector4(const Vector3& in, f32 h) {
        x = in.x, y = in.y, z = in.z, w = h;
    }

    Vector4() {}

    Vector3 v3() {
        return Vector3(x, y, z);
    }
    f32& operator[](int index) {
        return data[index];
    }
    Vector4& operator*= (const Vector4& rhs) {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        w *= rhs.w;
        return (*this);
    }
    Vector4& operator*= (const f32 scale) {
        x *= scale;
        y *= scale;
        z *= scale;
        w *= scale;
        return (*this);
    }

    f32 dist(const Vector4& other) {
        f32 dx = other.x - x, dy =  other.y - y, dz = other.z - z;
        return sqrt((dx*dx + dy*dy + dz*dz));
    }
    
    
};

struct Matrix3 {
    union {
        f32 data[3][3];
        struct {
            f32 _00,  _01,  _02,
            _10,  _11,  _12,
            _20,  _21,  _22;
        };
        struct {
            Vector3 c1, c2, c3;
        };
    };
    
    Matrix3(f32 __00, f32 __01, f32 __02, 
            f32 __10, f32 __11, f32 __12, 
            f32 __20, f32 __21, f32 __22) {
        _00 = __00; _01 = __10; _02 = __20;
        _10 = __01; _11 = __11; _12 = __21;
        _20 = __02; _21 = __12; _22 = __22;
    }

    Matrix3(Vector3 x, Vector3 y, Vector3 z) : c1(x), c2(y), c3(z) {}

    Matrix3 transpose() {
        return Matrix3(_00, _01, _02, _10, _11, _12, _20, _21, _22);
        
    }

    Matrix3& operator*= (f32 scale) {
        data[0][0] *= scale;
        data[0][1] *= scale;
        data[0][2] *= scale;
        data[1][0] *= scale;
        data[1][1] *= scale;
        data[1][2] *= scale;
        data[2][0] *= scale;
        data[2][1] *= scale;
        data[2][2] *= scale;
        return (*this);
    }

    f32& operator()(int i, int j) {
        return data[j][i];
    }
    const f32& operator()(int i, int j) const {
        return data[j][i];
    }

    Vector3& operator[](int vec) {
        return *(reinterpret_cast<Vector3*>(&data[vec][0]));
    }
    
    
};


struct alignas(64) Matrix4 {
    union {
        f32 data[4][4];
        struct {
            f32 _00,  _01,  _02,  _03,
            _10,  _11,  _12,  _13,
            _20,  _21,  _22,  _23,
            _30,  _31,  _32,  _33;
        };
        struct {
            Vector4 _v1, _v2, _v3, _v4;
            
            
            
        };
    };
    // column major (vectors are the columns)
    // When the user wants 10, I need to give them mat(1,0), which is data[0][1]
    // According to RTR 4, the last 4 values in memory are the translations
    // This implies I should do this type of storage, so the vectors are contiguous
    Matrix4(f32 __00, f32 __01, f32 __02, f32 __03,
            f32 __10, f32 __11, f32 __12, f32 __13,
            f32 __20, f32 __21, f32 __22, f32 __23,
            f32 __30, f32 __31, f32 __32, f32 __33) {
        _00 = __00; _01 = __10; _02 = __20; _03 = __30;
        _10 = __01; _11 = __11; _12 = __21; _13 = __31;
        _20 = __02; _21 = __12; _22 = __22; _23 = __32;
        _30 = __03; _31 = __13; _32 = __23; _33 = __33;
    }
    
    Matrix4(const Vector4& v1, const Vector4& v2, const Vector4& v3, const Vector4& v4) {
        _v1 = v1; _v2 = v2; _v3 = v3; _v4 = v4;
    }

    Matrix4& operator*= (f32 scale) {
        data[0][0] *= scale;
        data[0][1] *= scale;
        data[0][2] *= scale;
        data[0][3] *= scale;
        data[1][0] *= scale;
        data[1][1] *= scale;
        data[1][2] *= scale;
        data[1][3] *= scale;
        data[2][0] *= scale;
        data[2][1] *= scale;
        data[2][2] *= scale;
        data[2][3] *= scale;
        data[3][0] *= scale;
        data[3][1] *= scale;
        data[3][2] *= scale;
        data[3][3] *= scale;
         
        return (*this);
    }

    Matrix4(f32 _00, f32 _01, f32 _02,
            f32 _10, f32 _11, f32 _12,
            f32 _20, f32 _21, f32  _22) {
        data[0][0] = _00;
        data[0][1] = _10;
        data[0][2] = _20;
        data[0][3] = 0.0f;
        data[1][0] = _01;
        data[1][1] = _11;
        data[1][2] = _21;
        data[1][3] = 0.0f;
        data[2][0] = _02;
        data[2][1] = _12;
        data[2][2] = _22;
        data[2][3] = 0.0f;
        data[3][0] = 0.0f;
        data[3][1] = 0.0f;
        data[3][2] = 0.0f;
        data[3][3] = 1.0f;
    }
    Matrix4() {}
    f32& operator()(int i, int j) {
        return data[j][i];
    }
    const f32& operator()(int i, int j) const {
        return data[j][i];
    }
    
    
    Vector4& operator[](int vec) {
        return *(reinterpret_cast<Vector4*>(&data[vec][0]));
    }

    Vector3& v3(int vec) {
        return *(reinterpret_cast<Vector3*>(&data[vec][0]));
    }
    
};


// Remember that the i, j, k are the imaginary parts
// The w is like the other part
// Properties of these lend well to different types of operations
struct Quaternion {
    union {
        struct {
            f32 i, j, k, w;
        };
        f32 data[4];
    };
    
    Quaternion(f32 i, f32 j, f32 k, f32 w) {
        i = i; j = j; k = k; w = w;
    }
    
    Quaternion(const Vector3& vec, f32 w) {
        data[0] = vec.x;
        data[1] = vec.y;
        data[2] = vec.z;
        data[3] = w;
    }

    Quaternion(const Vector4& vec) {
        data[0] = vec.x;
        data[1] = vec.y;
        data[2] = vec.z;
        data[3] = vec.w;
    }
    f32 operator[](int index) {
        return data[index];
    }
    
};

/* What is the simplest way to represent this? other than its worldspace transform*/
struct CoordinateSpace {
    Vector3 origin; // in world space coordinates
    Vector3 r, s, t;

    CoordinateSpace() {
        r = Vector3(1.0f, 0.0f, 0.0f);
        s = Vector3(0.0f, 1.0f, 0.0f);
        t = Vector3(0.0f, 0.0f, 1.0f);
        origin = Vector3(0.0f, 0.0f, 0.0f);
    }
    CoordinateSpace(Vector3 r, Vector3 s,Vector3 t, Vector3 o ) : r(r), s(s), t(t), origin(o) {}
    void rotate(Matrix3& rotation);
    
};




inline Vector4 Cross(const Vector4& a ,const Vector4& b, f32 w) {
    return Vector4(a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x, w);
}

inline Vector3 Cross(const Vector3& a ,const Vector3& b) {
    return Vector3(a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x);
}



inline Vector3 operator+(const Vector3& lhs, const Vector3& rhs) {
    return Vector3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}

inline Vector3 operator-(const Vector3& lhs, const Vector3& rhs) {
    return Vector3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

inline Vector4 operator+(const Vector4& lhs, const Vector4& rhs) {
    return Vector4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w);    
}


inline Vector4 operator-(const Vector4& lhs, const Vector4& rhs) {
    return Vector4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w);    
}

inline Vector3 operator*(const Vector3& lhs, const Vector3& rhs) {
    return Vector3(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);    
}


inline Vector3 operator/(const Vector3& lhs, const Vector3& rhs) {
    return Vector3(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z);    
}


inline Vector4 operator*(const Vector4& lhs, const Vector4& rhs) {
    return Vector4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w);    
}


inline Vector4 operator/(const Vector4& lhs, const Vector4& rhs) {
    return Vector4(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w);    
}

inline Vector4 operator*(const Vector4& lhs, f32 scale) {
    return Vector4(lhs.x*scale, lhs.y*scale, lhs.z* scale, lhs.w * scale);
}


inline Vector4 operator/(const Vector4& lhs, f32 scale) {
    return Vector4(lhs.x/scale, lhs.y/scale, lhs.z/ scale, lhs.w / scale);
}

inline Vector4 operator*( f32 scale, const Vector4& rhs) {
    return Vector4(rhs.x*scale, rhs.y*scale, rhs.z* scale, rhs.w * scale);
}


inline Vector4 operator/(f32 scale, const Vector4& rhs) {
    return Vector4(rhs.x/scale, rhs.y/scale, rhs.z/ scale, rhs.w / scale);
}


inline Vector3 operator*(const Vector3& lhs, f32 scale) {
    return Vector3(lhs.x*scale, lhs.y*scale, lhs.z* scale);
}


inline Vector3 operator/(const Vector3& lhs, f32 scale) {
    return Vector3(lhs.x/scale, lhs.y/scale, lhs.z/ scale);
}

inline Vector3 operator*( f32 scale, const Vector3& rhs) {
    return Vector3(rhs.x*scale, rhs.y*scale, rhs.z* scale);
}


inline Vector3 operator/(f32 scale, const Vector3& rhs) {
    return Vector3(rhs.x/scale, rhs.y/scale, rhs.z/ scale);
}

inline f32 Dot(const Vector3& lhs, const Vector3& rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}


inline f32 Dot(const Vector4& lhs, const Vector4& rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}



struct Plane {
    f32 x, y, z, d;
    Plane(f32 _x, f32 _y, f32 _z, f32 _d) {
        x = _x; y = _y; z = _z; d = _d;
    }

    Plane(Vector3 p1, Vector3 p2, Vector3 p3) {
        Vector3 v1  = p3 - p1;
        Vector3 v2 = p3 - p2;
        Vector3 cp = Cross(v1, v2);
        cp.normalize();
        x = cp.x;
        y = cp.y;
        z = cp.z;
        d = -1.0f * Dot(v1, cp);
    }

    Plane(Vector3 xyz, f32 _d) {
        x = xyz.x; y = xyz.y; z = xyz.z; d = _d;
    }

    Plane() {}

    Vector3 GetNormal()  {
        return Vector3(x, y, z);
    }
};



inline f32 Dot(const Vector3& point, const Plane& plane) {
    return point.x * plane.x + point.y * plane.y + point.z * plane.z + plane.d;
}


inline Quaternion operator* (Quaternion& in, f32 scale) {
    in.data[0] *= scale;
    in.data[1] *= scale;
    in.data[2] *= scale;
    in.data[3] *= scale;
    return in;
    
}

inline Quaternion operator* (f32 scale, Quaternion& in) {
    in.data[0] *= scale;
    in.data[1] *= scale;
    in.data[2] *= scale;
    in.data[3] *= scale;
    return in;    
}

// with (4x4)(4x1) = 4x1
#if 0
inline Vector4 operator*( Matrix4& lhs, Vector4&rhs) {
    f32 a, b, c, d;
    a = lhs(0,0)*rhs.data[0] + lhs(0, 1)* rhs.data[1] + lhs(0, 2)* rhs.data[2] + lhs(0, 3) * rhs.data[3];
    b= lhs(1,0)*rhs.data[0] + lhs(1, 1)* rhs.data[1] + lhs(1, 2)* rhs.data[2] + lhs(1, 3) * rhs.data[3];
    c= lhs(2,0)*rhs.data[0] + lhs(2, 1)* rhs.data[1] + lhs(2, 2)* rhs.data[2] + lhs(2, 3) * rhs.data[3];
    d= lhs(3,0)*rhs.data[0] + lhs(3, 1)* rhs.data[1] + lhs(3, 2)* rhs.data[2] + lhs(3, 3) * rhs.data[3];
    return Vector4(a, b, c, d);
    
}
#endif
/* det 2x2 */
inline f32 Det(f32 ul, f32 ur, f32 ll, f32 lr) {
    return ul*lr - ll*ur;
}

/* determinant of a matrix */
inline f32 Det(Matrix3& in) {
    f32 l, c, r;
    
    l = in(0, 0) * Det(in(1,1), in(1,2), in(2,1), in(2,2) );
    c = in(0, 1) * Det(in(1, 0), in(1, 2) ,in(2,0), in(2,2));
    r = in(0, 2) * Det(in(1,0), in(1,1), in(2,0), in(2,1));
    return l - c + r;
}


/* determines if the given rotation does a reflection. If it does, then this will put the vertices in the wrong order */
inline bool IsReflection(Matrix4 transform) {
    Matrix3 upper3x3 = Matrix3(transform(0, 0), transform(0, 1), transform(0,2),
                                     transform(1, 0), transform(1,1), transform(1,2),
                                     transform(2,0), transform(2,1), transform(2,2));
    
    return Det(upper3x3) < 0;
}

inline Quaternion Conjugate(const Quaternion& in) {
    return Quaternion(-in.data[0], -in.data[1], -in.data[2], in.data[3]);
}


inline Matrix4 ConstructTranslation(const Vector3 &linearTranslation){
    
    return Matrix4(1, 0, 0, linearTranslation.x,
                   0, 1, 0, linearTranslation.y,
                   0, 0, 1, linearTranslation.z,
                   0, 0, 0, 1);
}

/* Rotates CCW about x axis */
inline Matrix4 RotateX(float t) {
    f32 c = cosf(t);
    f32 s = sinf(t);
    return Matrix4(1,0, 0,0,
                   0, c, -s, 0,
                   0, s, c, 0,
                   0, 0, 0, 1);
    
    
}

/* Rotate about the y axis */
inline Matrix4 RotateY(float t) {
    f32 c = cosf(t);
    f32 s = sinf(t);
    return Matrix4(c,0, s,0,
                   0, 1, 0, 0,
                   -s, 0, c, 0,
                   0, 0, 0, 1);
    
}

/* Rotate about the Z axis */
inline Matrix4 RotateZ(float t){
    f32 c = cosf(t);
    f32 s = sinf(t);
    
    return Matrix4(c, -s, 0, 0,
                   s, c, 0, 0,
                   0, 0, 1, 0,
                   0, 0, 0, 0);
}

inline Matrix3 RotateX3(float t) {
    f32 c = cosf(t);
    f32 s = sinf(t);
    return Matrix3(1.0f ,0.0f, 0.0f,
                   0.0f, c, -s,
                   0.0f, s, c);

    
    
}

/* Rotate about the y axis */
inline Matrix3 RotateY3(float t) {
    f32 c = cosf(t);
    f32 s = sinf(t);
    return Matrix3(c,0.0f, s,
                   0.0f, 1.0f, 0.0f, 
                   -s, 0.0f, c);
                  
    
}

/* Rotate about the Z axis */
inline Matrix3 RotateZ3(float t){
    f32 c = cosf(t);
    f32 s = sinf(t);
    
    return Matrix3(c, -s, 0.0f,
                   s, c, 0.0f, 
                   0.0f, 0.0f, 1.0f);
}




inline Matrix3 operator* ( Matrix3 &lhs,  Matrix3& rhs) {
    f32 _00 = lhs(0,0)* rhs(0,0) + lhs(0,1)*rhs(1,0) + lhs(0, 2) * rhs(2,0);
    f32 _01 = lhs(0,0)* rhs(0,1) + lhs(0,1)*rhs(1,1) + lhs(0, 2) * rhs(2,1);
    f32 _02 = lhs(0,0)* rhs(0,2) + lhs(0,1)*rhs(1,2) + lhs(0, 2) * rhs(2,2);
    f32 _10 = lhs(1,0)* rhs(0,0) + lhs(1,1)*rhs(1,0) + lhs(1, 2) * rhs(2,0);
    f32 _11 = lhs(1,0)* rhs(0,1) + lhs(1,1)*rhs(1,1) + lhs(1, 2) * rhs(2,1); // same column as 01
    f32 _12 = lhs(1,0)* rhs(0,2) + lhs(1,1)*rhs(1,2) + lhs(1, 2) * rhs(2,2);// same column as 02
    f32 _20 = lhs(2,0)* rhs(0,0) + lhs(2,1)*rhs(1,0) + lhs(2, 2) * rhs(2,0);
    f32 _21 = lhs(2,0)* rhs(0,1) + lhs(2,1)*rhs(1,1) + lhs(2, 2) * rhs(2,1);
    f32 _22 = lhs(2,0)* rhs(0,2) + lhs(2,1)*rhs(1,2) + lhs(2, 2) * rhs(2,2);
    
    
    return Matrix3(_00, _01, _02, _10, _11, _12, _20, _21, _22);
}

inline Matrix3 operator*(f32 scale, Matrix3& rhs) {
    return Matrix3(rhs(0,0)*scale, rhs(0,1)*scale, rhs(0,2)*scale,
                   rhs(1,0)*scale, rhs(1,1)*scale, rhs(1,2)*scale,
                   rhs(2,0)*scale, rhs(2,1)*scale, rhs(2,2)*scale);
}

inline Matrix4 operator*(Matrix4& lhs, Matrix4& rhs) {


    Matrix4 emptyMatrix;
    __m128 col0 = _mm_load_ps(&lhs.data[0][0]);
    __m128 col1 = _mm_load_ps(&lhs.data[1][0]);
    __m128 col2 = _mm_load_ps(&lhs.data[2][0]);
    __m128 col3 = _mm_load_ps(&lhs.data[3][0]);

    for (int i = 0; i < 4; ++i) {
        __m128 b0 = _mm_set1_ps(rhs.data[i][0]);
        __m128 b1 = _mm_set1_ps(rhs.data[i][1]);
        __m128 b2 = _mm_set1_ps(rhs.data[i][2]);
        __m128 b3 = _mm_set1_ps(rhs.data[i][3]);
        __m128 res = _mm_add_ps(_mm_add_ps(_mm_mul_ps(col0, b0), _mm_mul_ps(col1, b1)), _mm_add_ps(_mm_mul_ps(col2, b2), _mm_mul_ps(col3, b3)));
        _mm_store_ps(&emptyMatrix.data[i][0], res);
    }

    return emptyMatrix;
    
}
 
inline Vector4 operator*(Matrix4& lhs, Vector4& rhs) {
    Vector4 targetVector;
    
    __m128 col0 = _mm_load_ps(&lhs.data[0][0]);
    __m128 col1 = _mm_load_ps(&lhs.data[1][0]);
    __m128 col2 = _mm_load_ps(&lhs.data[2][0]);
    __m128 col3 = _mm_load_ps(&lhs.data[3][0]);

    __m128 bv0 = _mm_set1_ps(rhs.data[0]);
    __m128 bv1 = _mm_set1_ps(rhs.data[1]);
    __m128 bv2 = _mm_set1_ps(rhs.data[2]);
    __m128 bv3 = _mm_set1_ps(rhs.data[3]);

    __m128 res = _mm_add_ps(_mm_add_ps(_mm_mul_ps(col0, bv0), _mm_mul_ps(col1, bv1)), _mm_add_ps(_mm_mul_ps(col2, bv2), _mm_mul_ps(col3, bv3)));
    _mm_store_ps(&targetVector.data[0], res);
    return targetVector;
}

inline Quaternion operator* (Quaternion& lhs, Quaternion& rhs) {
    Vector3 lhs_v = Vector3(lhs.i, lhs.j, lhs.k);
    Vector3 rhs_v = Vector3(rhs.i, rhs.j, rhs.k);
    /* emplace this here */
    Vector3 imaginaryPart = Cross(lhs_v, rhs_v) + (rhs.w * lhs_v) + (lhs.w * rhs_v);
    f32 w = lhs.w * rhs.w + Dot(lhs_v, rhs_v);
    return Quaternion(imaginaryPart, w);
    
}

inline Matrix4 operator* (f32 scale, Matrix4& matrix) {
    Matrix4 matrixOut = (matrix *= scale);
    return matrixOut;
    
}

/* this really is the adjoint of the upper 3x3 */
inline Matrix3 adjointUpper3x3( Matrix4 &in) {
    return Matrix3(Det(in(1, 1), in(1,2), in(2, 1), in(2, 2)), -Det(in(0,1), in(0,2), in(2,1), in(2,2)), Det(in(0,1), in(0,2), in(1,1), in(1,2)),
                   -Det(in(1,0), in(1,2), in(2,0), in(2,2)), Det(in(0,0), in(0,2), in(2,0), in(2,2)), -Det(in(0,0), in(0,2), in(1,0), in(1,2)),
                   Det(in(1,0), in(1,1), in(2,0), in(2,1)), -Det(in(0,0), in(0,1), in(2,0), in(2,1)), Det(in(0,0), in(0,1), in(1,0), in(1,1)) );
}

inline Matrix3 Adjoint3x3( Matrix3 &in) {
    return Matrix3(Det(in(1, 1), in(1,2), in(2, 1), in(2, 2)), -Det(in(0,1), in(0,2), in(2,1), in(2,2)), Det(in(0,1), in(0,2), in(1,1), in(1,2)),
                   -Det(in(1,0), in(1,2), in(2,0), in(2,2)), Det(in(0,0), in(0,2), in(2,0), in(2,2)), -Det(in(0,0), in(0,2), in(1,0), in(1,2)),
                   Det(in(1,0), in(1,1), in(2,0), in(2,1)), -Det(in(0,0), in(0,1), in(2,0), in(2,1)), Det(in(0,0), in(0,1), in(1,0), in(1,1)) );
}

inline Matrix3 Inv3x3(Matrix3& in) {
    Matrix3 m = Adjoint3x3(in);
    f32 invdet = 1/Det(in);
    m *= invdet;
    return m;
}


inline Matrix4 InvTransform(Matrix4& in) {
    Vector3& _0 = in.v3(0);
    Vector3& _1 = in.v3(1);
    Vector3& _2 = in.v3(2);
    Vector3& _3 = in.v3(3);
    
    Vector3 c01 = Cross(_0,_1), c23 = Cross(_2,_3);
    f32 scale = 1.0f  / Dot(c01,_2); //the bottom row is 0 0 0 1, so this is fine
    Vector3 v = _2 * scale;
    c01 *= scale;
    c23 *= scale;
    Vector3 r0 = Cross(_1, v);
    Vector3 r1 = Cross(v, _0);
    
    return Matrix4(r0.x, r0.y, r0.z, -Dot(_1, c23),
                          r1.x, r1.y, r1.z, Dot(_0, c23),
                          c01.x, c01.y, c01.z, -Dot(_3, c01),
                          0.0f, 0.0f, 0.0f, 1.0f);
    
}

inline f32 Lerp(f32 start, f32 end, f32 t) {
    return start + (end-start)*t;
}


/* The idea with rotating about an arbitrary axis is that you can do a change of base, then rotate around say the x axis only
 * but we can also rotate about an arbitrary vector using the quaternions, therefore we will use the quaternion
 * The quaternion q = (sinx uq, cosx) will rotate about an axis uq by x degrees
 * Does not check to see if the vector is a unit vector
 * RETURNS the quaternion that is used for this transform
 */
inline Quaternion RotationQuaternion(const Vector3& axis, f32 phi ) {
    f32 s = sinf(phi/2);
    f32 c = cosf(phi/2);
    
    return Quaternion(s*axis.x, s*axis.y, s*axis.z, c);
}

// Assumes that the input is a unit quaternion
inline Vector4 QuaternionRotatePoint(Quaternion& in, const Vector4& point) {
    Quaternion conj = Conjugate(in);
    Quaternion pure(point);
    pure = pure * conj;
    Quaternion tmp = in * pure;
    return Vector4(tmp.i, tmp.j, tmp.k, tmp.w); 
}

// Returns a change of basis matrix
inline Matrix4 ChangeOfBasis(Vector4& r, Vector4& u, Vector4& v) {
    return Matrix4(r, u, v, Vector4(0));
}

// Change the model coord space with the dot of the eye and the coordinate axes
// We want to express a point in world space as the sum of elements of our new basis
// (e.g) in world space the point (4,2,3) is just the sum of 4<1,0,0> + 2<0,1,0> + 3<0,0,1>
// But we also want to subtract elements of the distance included that are already included in the origin
// If we look at the matrix mult we see DOT PRODUCTS -> <rx, ry, rz> * <x, y, z> 
inline Matrix4 WorldObjectMatrix(const CoordinateSpace& modelCoordSpace){
    return Matrix4(modelCoordSpace.r.x, modelCoordSpace.r.y, modelCoordSpace.r.z, -Dot(modelCoordSpace.origin, modelCoordSpace.r ),
                   modelCoordSpace.s.x, modelCoordSpace.s.y, modelCoordSpace.s.z, -Dot(modelCoordSpace.origin, modelCoordSpace.s),
                   modelCoordSpace.t.x, modelCoordSpace.t.y, modelCoordSpace.t.z, -Dot(modelCoordSpace.origin, modelCoordSpace.t),
            0, 0, 0,1);
}

// The dot of the world space coordinate system is always directly onto x, y, z since there is no rotation, so we should be good
// We want to take our point in obj space and express each's contributions to x,y, z respectivel 
// Then we can add the origin, and the dots like the one earlier are just all 1 for each bases
inline Matrix4 ObjectWorldMatrix(const CoordinateSpace& modelCoordSpace) {
    return Matrix4(modelCoordSpace.r.x, modelCoordSpace.s.x, modelCoordSpace.t.x, modelCoordSpace.origin.x,
        modelCoordSpace.r.y, modelCoordSpace.s.y, modelCoordSpace.t.y, modelCoordSpace.origin.y,
        modelCoordSpace.r.z, modelCoordSpace.s.z, modelCoordSpace.t.z, modelCoordSpace.origin.z,
            0, 0, 0, 1);
}

// gl asks for the near and far plane to be expressed positively, but it still represents direction along negative z axis
// therefore the 3rd column of the usual projection matrix has its sign flipped
inline Matrix4 ProjMatrix(f32 vFOV, f32 aspectRatio, f32 nearPlane, f32 farPlane) {


    f32 c = 1.0f/ tanf(vFOV/2);
    // this maps n, f -> -1 to 1 
    #if 1
    return Matrix4(c/aspectRatio, 0, 0, 0,
                   0, c, 0, 0,
                   0, 0, -(farPlane + nearPlane)/(farPlane - nearPlane), -2*(farPlane*nearPlane)/(farPlane - nearPlane),
                   0, 0, -1, 0); 
    #else 
    // n, f -> 0 to 1
    return Matrix4(c/aspectRatio, 0, 0, 0,
                   0, c, 0, 0,
                   0, 0, -farPlane/(farPlane-nearPlane), -nearPlane*farPlane/(farPlane-nearPlane),
                   0, 0, -1, 0); 
    #endif
}

// PVM is the right order? Make sure that after division by w these are all in the unit cube
inline Matrix4 ModelViewProjection(const CoordinateSpace& objSpace, const CoordinateSpace& cameraSpace, f32 vFOV, f32 aspectRatio, f32 nearPlane, f32 farPlane) {
    Matrix4 m = ObjectWorldMatrix(objSpace);
    Matrix4 v = WorldObjectMatrix(cameraSpace);
    Matrix4 p = ProjMatrix(vFOV, aspectRatio, nearPlane, farPlane);
    Matrix4 vm = v*m;
    return (p*vm);
}

inline Matrix4 ModelView(const CoordinateSpace& objSpace, const CoordinateSpace& cameraSpace) {
    Matrix4 m = ObjectWorldMatrix(objSpace);
    Matrix4 v = WorldObjectMatrix(cameraSpace);
    return v*m;
}

inline void CoordinateSpace::rotate( Matrix3& rotation) {
        Matrix3 m(r, s, t);
        m = m.transpose();
        m = rotation * m;
        m = m.transpose();
        r = m[0]; s = m[1]; t = m[2];
    }

inline Matrix3 NormalTransform(Matrix3& transform) {
    Matrix3 transpose = transform.transpose();
    return Inv3x3(transpose);
}

inline Matrix3 Matrix3x3(const Matrix4& in) {
    return Matrix3(in(0,0), in(0,1), in(0,2),
                   in(1,0), in(1,1), in(1,2),
                   in(2,0), in(2,1), in(2,2));
}

// Do not use this on a vector of length 0....
inline bool CheckParallel( const Vector3& lhs, const  Vector3& rhs) {
    f32 d = Dot(lhs, rhs);
    f32 magprod = lhs.mag() * rhs.mag();
    return fabsf(d - magprod) < .01f;
}


// So we want this to have an up dir of (0, 1, 0)
// We also want to look at something in the negative z direction
// Make sure it is in front of us
inline Matrix4 LookAt(const Vector3 &at, const Vector3 &eye) {
    Vector3 up = Vector3(0, 1, 0);

    Vector3 z = at - eye;
    z *= -1; // now z points in the "wrong" direction
    if (CheckParallel(z, up)) {
        up = Vector3(.707f, .707f, 0.0f);
    }
    Vector3 x = Cross(up, z); // it is perpendicular now to y and to the new axis. cp wraps around
    Vector3 y = Cross(z, x); // now this is perpendicular to both x and z... we should have a basis
    z.normalize();
    y.normalize();
    x.normalize();
    CoordinateSpace coordSpace(x, y, z, eye);

    return WorldObjectMatrix(coordSpace);
}

#endif