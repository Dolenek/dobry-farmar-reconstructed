#include "SpriteGeometry.h"
#include "GameColor.h"
#include "OriginalArithmetic.h"
namespace farmar::recovered {
namespace {
float interpolateColor(float tl,float tr,float bl,float br,float row,float column,unsigned channel) noexcept {
    // Spill points differ between RGBA channels in the original x87 instruction
    // sequence. Double intermediates retain its register precision; explicit
    // float conversions reproduce the recorded stores before the next operation.
    const double inverseRow=originalSubtract(1.0,row),inverseColumn=originalSubtract(1.0,column);
    double brRow=originalMultiply(br,row);
    if(channel>=2) brRow=static_cast<float>(brRow);
    double brTerm=originalMultiply(brRow,column);
    if(channel==0 || channel==3) brTerm=static_cast<float>(brTerm);
    const float blRow=static_cast<float>(originalMultiply(bl,row));
    double blTerm=originalMultiply(blRow,inverseColumn);
    if(channel!=3) blTerm=static_cast<float>(blTerm);
    double trRow=originalMultiply(tr,inverseRow);
    if(channel!=0) trRow=static_cast<float>(trRow);
    double trTerm=originalMultiply(trRow,column);
    if(channel!=1) trTerm=static_cast<float>(trTerm);
    const float tlRow=static_cast<float>(originalMultiply(tl,inverseRow));
    const float tlTerm=static_cast<float>(originalMultiply(tlRow,inverseColumn));
    double sum=originalAdd(tlTerm,trTerm);
    if(channel==0 || channel==3) sum=static_cast<float>(sum);
    const float firstThree=static_cast<float>(originalAdd(sum,blTerm));
    return static_cast<float>(originalAdd(firstThree,brTerm));
}
}
void writeSpriteGeometry(const SpriteDefinition& sprite,TextureScale textureScale,bool fullscreen,
    const DesktopImagePlacement& placement,SpriteVertex* vertices,std::uint16_t* indices,std::uint32_t baseVertex) noexcept {
    const auto& tl=sprite.corners[0]; const auto& tr=sprite.corners[1];
    const auto& bl=sprite.corners[2]; const auto& br=sprite.corners[3];
    const float rowX=bl.x-tl.x,rowY=bl.y-tl.y,columnX=tr.x-tl.x,columnY=tr.y-tl.y;
    const float parallelogramX=static_cast<float>(originalAdd(originalAdd(rowX,tl.x),columnX));
    const float bendX=br.x-parallelogramX;
    const float bendY=static_cast<float>(originalSubtract(br.y,originalAdd(originalAdd(rowY,tl.y),columnY)));
    const Color colors[4]={Color::fromARGB(tl.color),Color::fromARGB(tr.color),Color::fromARGB(bl.color),Color::fromARGB(br.color)};
    const unsigned side=sprite.subdivisions+1;
    for(unsigned y=0;y<side;++y) {
        const float row=static_cast<float>(originalDivide(originalUnsigned(y),originalUnsigned(sprite.subdivisions)));
        for(unsigned x=0;x<side;++x) {
            const float column=static_cast<float>(originalDivide(originalUnsigned(x),originalUnsigned(sprite.subdivisions)));
            auto& vertex=vertices[y*side+x];
            const float crossX=static_cast<float>(originalMultiply(originalMultiply(bendX,row),column));
            const float crossY=static_cast<float>(originalMultiply(originalMultiply(bendY,row),column));
            const float originX=static_cast<float>(originalAdd(originalMultiply(rowX,row),tl.x));
            const double originY=originalAdd(originalMultiply(rowY,row),tl.y);
            const float linearX=static_cast<float>(originalAdd(originX,originalMultiply(columnX,column)));
            const float linearY=static_cast<float>(originalAdd(originY,originalMultiply(columnY,column)));
            vertex.x=linearX+crossX; vertex.y=linearY+crossY;
            if(!fullscreen) { vertex.x-=0.5f; vertex.y-=0.5f; }
            else {
                vertex.x*=placement.scaleX; vertex.y*=placement.scaleY;
                vertex.x+=placement.offsetX; vertex.y+=placement.offsetY;
            }
            vertex.z=0;
            Color color;
            color.r=interpolateColor(colors[0].r,colors[1].r,colors[2].r,colors[3].r,row,column,0);
            color.g=interpolateColor(colors[0].g,colors[1].g,colors[2].g,colors[3].g,row,column,1);
            color.b=interpolateColor(colors[0].b,colors[1].b,colors[2].b,colors[3].b,row,column,2);
            color.a=interpolateColor(colors[0].a,colors[1].a,colors[2].a,colors[3].a,row,column,3);
            vertex.color=color.toARGB();
            const float deltaU=static_cast<float>(originalMultiply(originalSubtract(sprite.u1,sprite.u0),column));
            vertex.u=static_cast<float>(originalMultiply(originalAdd(deltaU,sprite.u0),textureScale.u));
            vertex.v=static_cast<float>(originalMultiply(static_cast<float>(originalAdd(originalMultiply(originalSubtract(sprite.v1,sprite.v0),row),sprite.v0)),textureScale.v));
        }
    }
    for(unsigned y=0;y<sprite.subdivisions;++y) for(unsigned x=0;x<sprite.subdivisions;++x) {
        const auto first=static_cast<std::uint16_t>(baseVertex+y*side+x);
        const auto below=static_cast<std::uint16_t>(baseVertex+(y+1)*side+x);
        auto* cell=indices+(y*sprite.subdivisions+x)*6;
        cell[0]=first; cell[1]=below; cell[2]=first+1;
        cell[3]=first+1; cell[4]=below; cell[5]=below+1;
    }
}
}
