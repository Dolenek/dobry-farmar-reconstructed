#pragma once
namespace farmar::recovered {
struct Matrix4 {
    float values[16]{};
    static Matrix4 identity() noexcept;
};
static_assert(sizeof(Matrix4)==64,"Original matrix storage");
void multiplyMatrix(Matrix4& output,const Matrix4& left,const Matrix4& right) noexcept; // 00591fe8
void transposeMatrix(Matrix4& output,const Matrix4& input) noexcept; // 005920cb
void orthographicMatrix(Matrix4& output,float left,float right,float bottom,float top,float nearPlane,float farPlane) noexcept; // 0059221a
}
