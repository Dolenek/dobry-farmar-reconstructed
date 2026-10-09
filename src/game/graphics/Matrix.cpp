#include "GameMatrix.h"
#include "OriginalArithmetic.h"
namespace farmar::recovered {
Matrix4 Matrix4::identity() noexcept { Matrix4 output; for(unsigned i=0;i<4;++i) output.values[i*5]=1; return output; }
void multiplyMatrix(Matrix4& output,const Matrix4& left,const Matrix4& right) noexcept {
    Matrix4 result;
    for(unsigned row=0;row<4;++row) for(unsigned column=0;column<4;++column) {
        // Original x87 stack pairs product 0 with 2, and 1 with 3. Keeping
        // intermediates wider than float and this grouping is significant.
        const double p0=originalMultiply(left.values[row*4],right.values[column]);
        const double p1=originalMultiply(left.values[row*4+1],right.values[4+column]);
        const double p2=originalMultiply(left.values[row*4+2],right.values[8+column]);
        const double p3=originalMultiply(left.values[row*4+3],right.values[12+column]);
        result.values[row*4+column]=static_cast<float>(originalAdd(originalAdd(p0,p2),originalAdd(p1,p3)));
    }
    output=result;
}
void transposeMatrix(Matrix4& output,const Matrix4& input) noexcept {
    Matrix4 result;
    for(unsigned row=0;row<4;++row) for(unsigned column=0;column<4;++column)
        result.values[row*4+column]=input.values[column*4+row];
    output=result;
}
void orthographicMatrix(Matrix4& output,float left,float right,float bottom,float top,float nearPlane,float farPlane) noexcept {
    // x87 retains X reciprocal in a register but spills Y and Z to float.
    const double x=originalDivide(1.0,originalSubtract(right,left));
    const float y=static_cast<float>(originalDivide(1.0,originalSubtract(top,bottom)));
    const float z=static_cast<float>(originalDivide(1.0,originalSubtract(farPlane,nearPlane)));
    output={};
    output.values[0]=static_cast<float>(originalMultiply(x,2.0));
    output.values[5]=y*2.f;
    output.values[10]=z;
    output.values[12]=static_cast<float>(originalMultiply(-originalAdd(left,right),x));
    output.values[13]=static_cast<float>(originalMultiply(-originalAdd(bottom,top),y));
    output.values[14]=static_cast<float>(originalMultiply(-double(z),nearPlane));
    output.values[15]=1;
}
}
