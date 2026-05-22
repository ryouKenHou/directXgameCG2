#pragma once

#include <cmath>
#include "Vector3.hpp"

struct Matrix4x4 {
    float m[4][4];
    Matrix4x4() {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                m[i][j] = (i == j) ? 1.0f : 0.0f; // Identity matrix
            }
        }
    }

    Matrix4x4(float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33) {
        m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
        m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
        m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
        m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
    }

    Matrix4x4 operator*(const Matrix4x4& other) const {
        Matrix4x4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.m[i][j] = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    result.m[i][j] += m[i][k] * other.m[k][j];
                }
            }
        }
        return result;
    }

    Matrix4x4 operator*(const float scalar) const {
        Matrix4x4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.m[i][j] = m[i][j] * scalar;
            }
        }
        return result;
    }

    Matrix4x4 operator+(const Matrix4x4& other) const {
        Matrix4x4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.m[i][j] = m[i][j] + other.m[i][j];
            }
        }
        return result;
    }

    Matrix4x4 operator-(const Matrix4x4& other) const {
        Matrix4x4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.m[i][j] = m[i][j] - other.m[i][j];
            }
        }
        return result;
    }

    // Static member functions for operations between two matrices
    static Matrix4x4 Add(const Matrix4x4& a, const Matrix4x4& b) {
        return a + b;
    }

    static Matrix4x4 Subtract(const Matrix4x4& a, const Matrix4x4& b) {
        return a - b;
    }

    static Matrix4x4 Multiply(float scalar, const Matrix4x4& m) {
        return m * scalar;
    }

    static Matrix4x4 Multiply(const Matrix4x4& a, const Matrix4x4& b) {
        return a * b;
    }

    static Matrix4x4 Identity() {
        return Matrix4x4();
    }

    static Matrix4x4 Transpose(const Matrix4x4& m) {
        Matrix4x4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.m[i][j] = m.m[j][i];
            }
        }
        return result;
    }

    static Matrix4x4 Inverse(const Matrix4x4& m) {
        \
            float Result[4][4];
        float tmp[12];
        float src[16];
        float det;

        /* transpose matrix */
        for (int i = 0; i < 4; i++)
        {
            src[i + 0] = m.m[i][0];
            src[i + 4] = m.m[i][1];
            src[i + 8] = m.m[i][2];
            src[i + 12] = m.m[i][3];
        }

        /* calculate pairs for first 8 elements (cofactors) */
        tmp[0] = src[10] * src[15];
        tmp[1] = src[11] * src[14];
        tmp[2] = src[9] * src[15];
        tmp[3] = src[11] * src[13];
        tmp[4] = src[9] * src[14];
        tmp[5] = src[10] * src[13];
        tmp[6] = src[8] * src[15];
        tmp[7] = src[11] * src[12];
        tmp[8] = src[8] * src[14];
        tmp[9] = src[10] * src[12];
        tmp[10] = src[8] * src[13];
        tmp[11] = src[9] * src[12];

        /* calculate first 8 elements (cofactors) */
        Result[0][0] = tmp[0] * src[5] + tmp[3] * src[6] + tmp[4] * src[7];
        Result[0][0] -= tmp[1] * src[5] + tmp[2] * src[6] + tmp[5] * src[7];
        Result[0][1] = tmp[1] * src[4] + tmp[6] * src[6] + tmp[9] * src[7];
        Result[0][1] -= tmp[0] * src[4] + tmp[7] * src[6] + tmp[8] * src[7];
        Result[0][2] = tmp[2] * src[4] + tmp[7] * src[5] + tmp[10] * src[7];
        Result[0][2] -= tmp[3] * src[4] + tmp[6] * src[5] + tmp[11] * src[7];
        Result[0][3] = tmp[5] * src[4] + tmp[8] * src[5] + tmp[11] * src[6];
        Result[0][3] -= tmp[4] * src[4] + tmp[9] * src[5] + tmp[10] * src[6];
        Result[1][0] = tmp[1] * src[1] + tmp[2] * src[2] + tmp[5] * src[3];
        Result[1][0] -= tmp[0] * src[1] + tmp[3] * src[2] + tmp[4] * src[3];
        Result[1][1] = tmp[0] * src[0] + tmp[7] * src[2] + tmp[8] * src[3];
        Result[1][1] -= tmp[1] * src[0] + tmp[6] * src[2] + tmp[9] * src[3];
        Result[1][2] = tmp[3] * src[0] + tmp[6] * src[1] + tmp[11] * src[3];
        Result[1][2] -= tmp[2] * src[0] + tmp[7] * src[1] + tmp[10] * src[3];
        Result[1][3] = tmp[4] * src[0] + tmp[9] * src[1] + tmp[10] * src[2];
        Result[1][3] -= tmp[5] * src[0] + tmp[8] * src[1] + tmp[11] * src[2];

        /* calculate pairs for second 8 elements (cofactors) */
        tmp[0] = src[2] * src[7];
        tmp[1] = src[3] * src[6];
        tmp[2] = src[1] * src[7];
        tmp[3] = src[3] * src[5];
        tmp[4] = src[1] * src[6];
        tmp[5] = src[2] * src[5];

        tmp[6] = src[0] * src[7];
        tmp[7] = src[3] * src[4];
        tmp[8] = src[0] * src[6];
        tmp[9] = src[2] * src[4];
        tmp[10] = src[0] * src[5];
        tmp[11] = src[1] * src[4];

        /* calculate second 8 elements (cofactors) */
        Result[2][0] = tmp[0] * src[13] + tmp[3] * src[14] + tmp[4] * src[15];
        Result[2][0] -= tmp[1] * src[13] + tmp[2] * src[14] + tmp[5] * src[15];
        Result[2][1] = tmp[1] * src[12] + tmp[6] * src[14] + tmp[9] * src[15];
        Result[2][1] -= tmp[0] * src[12] + tmp[7] * src[14] + tmp[8] * src[15];
        Result[2][2] = tmp[2] * src[12] + tmp[7] * src[13] + tmp[10] * src[15];
        Result[2][2] -= tmp[3] * src[12] + tmp[6] * src[13] + tmp[11] * src[15];
        Result[2][3] = tmp[5] * src[12] + tmp[8] * src[13] + tmp[11] * src[14];
        Result[2][3] -= tmp[4] * src[12] + tmp[9] * src[13] + tmp[10] * src[14];
        Result[3][0] = tmp[2] * src[10] + tmp[5] * src[11] + tmp[1] * src[9];
        Result[3][0] -= tmp[4] * src[11] + tmp[0] * src[9] + tmp[3] * src[10];
        Result[3][1] = tmp[8] * src[11] + tmp[0] * src[8] + tmp[7] * src[10];
        Result[3][1] -= tmp[6] * src[10] + tmp[9] * src[11] + tmp[1] * src[8];
        Result[3][2] = tmp[6] * src[9] + tmp[11] * src[11] + tmp[3] * src[8];
        Result[3][2] -= tmp[10] * src[11] + tmp[2] * src[8] + tmp[7] * src[9];
        Result[3][3] = tmp[10] * src[10] + tmp[4] * src[8] + tmp[9] * src[9];
        Result[3][3] -= tmp[8] * src[9] + tmp[11] * src[10] + tmp[5] * src[8];

        /* calculate determinant */
        det = src[0] * Result[0][0] + src[1] * Result[0][1] + src[2] * Result[0][2] + src[3] * Result[0][3];
        /* calculate matrix inverse */
        det = 1.0f / det;

        Matrix4x4 FloatResult;
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                FloatResult.m[i][j] = Result[i][j] * det;
            }
        }
        return FloatResult;
    }

    static Matrix4x4 MakeTranslateMatrix(const Vector3& translation) {
        return Matrix4x4(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            translation.x, translation.y, translation.z, 1
        );
    }

    static Matrix4x4 MakeScaleMatrix(const Vector3& scale) {
        return Matrix4x4(
            scale.x, 0, 0, 0,
            0, scale.y, 0, 0,
            0, 0, scale.z, 0,
            0, 0, 0, 1
        );
    }

    static Vector3 TransformPoint(const Matrix4x4& m, const Vector3& point) {
        float x = m.m[0][0] * point.x + m.m[1][0] * point.y + m.m[2][0] * point.z + m.m[3][0];
        float y = m.m[0][1] * point.x + m.m[1][1] * point.y + m.m[2][1] * point.z + m.m[3][1];
        float z = m.m[0][2] * point.x + m.m[1][2] * point.y + m.m[2][2] * point.z + m.m[3][2];
        float w = m.m[0][3] * point.x + m.m[1][3] * point.y + m.m[2][3] * point.z + m.m[3][3];
        if (w != 0.0f) {
            x /= w;
            y /= w;
            z /= w;
        }
        return Vector3(x, y, z);
    }

    static Matrix4x4 MakeRotationXMatrix(float radian) {
        float cosTheta = cos(radian);
        float sinTheta = sin(radian);
        return Matrix4x4(
            1, 0, 0, 0,
            0, cosTheta, sinTheta, 0,
            0, -sinTheta, cosTheta, 0,
            0, 0, 0, 1
        );
    }

    static Matrix4x4 MakeRotationYMatrix(float radian) {
        float cosTheta = cos(radian);
        float sinTheta = sin(radian);
        return Matrix4x4(
            cosTheta, 0, -sinTheta, 0,
            0, 1, 0, 0,
            sinTheta, 0, cosTheta, 0,
            0, 0, 0, 1
        );
    }
    static Matrix4x4 MakeRotationZMatrix(float radian) {
        float cosTheta = cos(radian);
        float sinTheta = sin(radian);
        return Matrix4x4(
            cosTheta, sinTheta, 0, 0,
            -sinTheta, cosTheta, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        );
    }

    static Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotation, const Vector3& translation) {
        Matrix4x4 translateMatrix = MakeTranslateMatrix(translation);
        Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);
        Matrix4x4 rotationXMatrix = MakeRotationXMatrix(rotation.x);
        Matrix4x4 rotationYMatrix = MakeRotationYMatrix(rotation.y);
        Matrix4x4 rotationZMatrix = MakeRotationZMatrix(rotation.z);
        return scaleMatrix * (rotationXMatrix * rotationYMatrix * rotationZMatrix) * translateMatrix;
    }

    static Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspect, float nearZ, float farZ) {
        float f = 1.0f / tan(fovY / 2.0f);
        return Matrix4x4(
            f / aspect, 0, 0, 0,
            0, f, 0, 0,
            0, 0, farZ / (farZ - nearZ), 1,
            0, 0, (-nearZ * farZ) / (farZ - nearZ), 0
        );
    }

    static Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearZ, float farZ) {
        return Matrix4x4(
            2 / (right - left), 0, 0, 0,
            0, 2 / (top - bottom), 0, 0,
            0, 0, 1 / (farZ - nearZ), 0,
            (right + left) / (left - right), (top + bottom) / (bottom - top), nearZ / (nearZ - farZ), 1
        );
    }

    static Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minZ, float maxZ) {
        return Matrix4x4(
            width / 2, 0, 0, 0,
            0, -height / 2, 0, 0,
            0, 0, maxZ - minZ, 0,
            left + width / 2, top + height / 2, minZ, 1
        );
    }
};  



struct Transform {
    Vector3 scale;
    Vector3 rotation;
    Vector3 translation;    
    
};