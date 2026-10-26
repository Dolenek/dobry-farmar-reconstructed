#include "NativeImage.h"
#include <algorithm>
#include <cstring>
#include <cstdint>
#include <stdexcept>
namespace farmar::recovered {
NativeImage decodeJpegImage(const void*,std::size_t);
namespace {
struct Reader {
    const unsigned char* data; std::size_t size;
    void check(std::size_t offset,std::size_t count) const {
        if(offset>size || count>size-offset) throw std::invalid_argument("Truncated image data");
    }
    std::uint32_t word(std::size_t offset) const { check(offset,4); std::uint32_t value; std::memcpy(&value,data+offset,4); return value; }
    unsigned short half(std::size_t offset) const { check(offset,2); unsigned short value; std::memcpy(&value,data+offset,2); return value; }
    unsigned byte(std::size_t offset) const { check(offset,1); return data[offset]; }
};
NativeImage dds(const Reader& input) { // 0059847a
    input.check(0,128); NativeImage result;
    const unsigned caps=input.word(112)&0xfc00U;
    if(caps && caps!=0xfc00U) throw std::invalid_argument("Incomplete DDS cube faces");
    result.faces=caps ? 6:1; result.mipLevels=(std::max)(1U,input.word(28));
    const unsigned fourcc=input.word(84),flags=input.word(80),bits=input.word(88);
    D3DFORMAT format;
    if(fourcc>=0x31545844U && fourcc<=0x35545844U && (fourcc&0xffffffU)==0x545844U) format=static_cast<D3DFORMAT>(fourcc);
    else if(flags==0x41 && bits==32 && input.word(104)==0xff000000U) format=D3DFMT_A8R8G8B8;
    else if(flags==0x40 && bits==32) format=D3DFMT_X8R8G8B8;
    else if(flags==0x40 && bits==16 && input.word(96)==0x7e0) format=D3DFMT_R5G6B5;
    else if(flags==0x40 && bits==16 && input.word(96)==0x3e0) format=D3DFMT_X1R5G5B5;
    else if(flags==0x41 && bits==16 && input.word(104)==0x8000) format=D3DFMT_A1R5G5B5;
    else if(flags==0x41 && bits==16 && input.word(104)==0xf000) format=D3DFMT_A4R4G4B4;
    else throw std::invalid_argument("Unsupported original DDS pixel format");
    std::size_t offset=128;
    for(unsigned face=0;face<result.faces;++face) {
        unsigned width=input.word(16),height=input.word(12);
        if(!width || !height) throw std::invalid_argument("Zero DDS dimensions");
        for(unsigned mip=0;mip<result.mipLevels;++mip) {
            const bool compressed=static_cast<unsigned>(format)>0x100;
            const unsigned pitch=compressed ? ((width+3)/4)*(format==D3DFMT_DXT1 ? 8:16):width*(format<=D3DFMT_X8R8G8B8 ? 4:2);
            const unsigned rows=compressed ? (height+3)/4:height;
            const std::size_t count=std::size_t(pitch)*rows; input.check(offset,count);
            result.planes.push_back({format,width,height,pitch,{input.data+offset,input.data+offset+count}});
            offset+=count; width=(std::max)(1U,width/2); height=(std::max)(1U,height/2);
        }
    }
    return result;
}
NativeImage tga(const Reader& input) { // 0059781c, direct color/grayscale branches
    input.check(0,18);
    const unsigned type=input.byte(2),bits=input.byte(16),width=input.half(12),height=input.half(14);
    if(input.byte(1)!=0 || (type!=2 && type!=3 && type!=10 && type!=11) || !width || !height)
        throw std::invalid_argument("Unsupported original TGA image type");
    D3DFORMAT format;
    if((type&3)==3 && bits==8) format=D3DFMT_L8;
    else if((type&3)==2 && bits==15) format=D3DFMT_X1R5G5B5;
    else if((type&3)==2 && bits==16) format=D3DFMT_A1R5G5B5;
    else if((type&3)==2 && bits==24) format=D3DFMT_R8G8B8;
    else if((type&3)==2 && bits==32) format=D3DFMT_A8R8G8B8;
    else throw std::invalid_argument("Unsupported original TGA pixel format");
    const unsigned pixelBytes=(bits+7)/8,pitch=width*pixelBytes;
    NativeImage result; result.planes.push_back({format,width,height,pitch,std::vector<unsigned char>(std::size_t(pitch)*height)});
    auto& plane=result.planes.front(); std::size_t offset=18+input.byte(0);
    const bool bottom=(input.byte(17)&0x20)==0,right=(input.byte(17)&0x10)!=0;
    for(unsigned y=0;y<height;++y) {
        unsigned x=0;
        while(x<width) {
            const unsigned packet=(type&8) ? input.byte(offset++):width-1;
            const unsigned count=(packet&0x7f)+1;
            // Uncompressed rows are copied in one span by the original code.
            const unsigned run=(type&8) ? count:width;
            if(run>width-x) throw std::invalid_argument("TGA packet exceeds original row");
            for(unsigned i=0;i<run;++i) {
                input.check(offset,pixelBytes);
                const unsigned dx=right ? width-1-(x+i):x+i,dy=bottom ? height-1-y:y;
                std::memcpy(plane.bytes.data()+std::size_t(dy)*pitch+dx*pixelBytes,input.data+offset,pixelBytes);
                if(!(type&8) || !(packet&0x80)) offset+=pixelBytes;
            }
            if((type&8) && (packet&0x80)) offset+=pixelBytes;
            x+=run;
        }
    }
    return result;
}
NativeImage bmp(const Reader& input) { // 0059878d -> 00596f2d, non-indexed BI_RGB
    input.check(0,54); const unsigned header=input.word(14),width=input.word(18),bits=input.half(28);
    const auto height=static_cast<std::int32_t>(input.word(22));
    if(header<40 || input.word(2)>input.size || input.half(26)!=1 || input.word(30)!=0 || !height || !width || width>0x7fffffffU)
        throw std::invalid_argument("Unsupported original BMP header/compression");
    D3DFORMAT format;
    if(bits==16) format=D3DFMT_X1R5G5B5;
    else if(bits==24) format=D3DFMT_R8G8B8;
    else if(bits==32) format=D3DFMT_X8R8G8B8;
    else throw std::invalid_argument("Unsupported original BMP pixel format");
    const unsigned rows=height<0 ? static_cast<unsigned>(-static_cast<std::int64_t>(height)):height;
    const unsigned pitch=(width*(bits/8)+3)&~3U;
    // Original ignores BITMAPFILEHEADER.bfOffBits and begins after DIB/palette.
    const std::size_t offset=14+header+std::size_t(input.word(46))*4,count=std::size_t(pitch)*rows;
    input.check(offset,count); NativeImage result;
    result.planes.push_back({format,width,rows,pitch,std::vector<unsigned char>(count)});
    for(unsigned y=0;y<rows;++y) std::memcpy(result.planes[0].bytes.data()+std::size_t(y)*pitch,
        input.data+offset+std::size_t(height<0 ? y:rows-1-y)*pitch,pitch);
    return result;
}
}
NativeImage decodeNativeImage(const void* data,std::size_t size) {
    const Reader input{static_cast<const unsigned char*>(data),size};
    if(size>=3 && input.half(0)==0xd8ff && input.byte(2)==0xff) return decodeJpegImage(data,size);
    if(size>=4 && input.word(0)==0x20534444U) return dds(input);
    if(size>=2 && input.half(0)==0x4d42) return bmp(input);
    return tga(input);
}
}
