#include "NativeImage.h"
#include "JpegColor.h"
#include <cstdio>
#pragma push_macro("FAR")
#undef FAR
// IJG INT32 is long; keep it distinct from Windows INT32 (int).
#define INT32 FarmarJpegInt32
extern "C" {
#include <jpeglib.h>
#include <jerror.h>
}
#undef INT32
#pragma pop_macro("FAR")
#include <csetjmp>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>
static_assert(JPEG_LIB_VERSION==61,"Original executable JPEG ABI version");
static_assert(sizeof(jpeg_decompress_struct)==424,"Original executable JPEG struct size");
namespace farmar::recovered {
namespace {
// 005975c7 uses IJG61, default output components/IDCT/upsampling and a
// memory source. This reader links pinned IJG61 separately from JNGLoad.
struct JpegState {
    jpeg_decompress_struct image{};
    struct Error {jpeg_error_mgr api{};std::jmp_buf target;} error;
    struct Source {jpeg_source_mgr api{};const JOCTET* data;std::size_t size;bool supplied;} source;
    unsigned char* pixels=nullptr;
    unsigned width=0,height=0,pitch=0,components=0;bool ycc=false;
};
void jpegError(j_common_ptr image) {auto* error=reinterpret_cast<JpegState::Error*>(image->err);std::longjmp(error->target,1);}
void jpegMessage(j_common_ptr) {} //00597584 formats a message without displaying it
void jpegSourceInit(j_decompress_ptr) {}
boolean jpegSourceFill(j_decompress_ptr image) {
    auto* source=reinterpret_cast<JpegState::Source*>(image->src);
    if(source->supplied) ERREXIT(image,JERR_INPUT_EOF); // malformed EOF behavior is not yet native-verified
    source->api.next_input_byte=source->data;source->api.bytes_in_buffer=source->size;source->supplied=true;return TRUE;
}
void jpegSourceSkip(j_decompress_ptr image,long count) {
    if(count<=0) return;
    if(static_cast<unsigned long>(count)>image->src->bytes_in_buffer) ERREXIT(image,JERR_INPUT_EOF);
    image->src->next_input_byte+=count;image->src->bytes_in_buffer-=count;
}
void jpegSourceTerm(j_decompress_ptr) {}
// All state modified after setjmp lives on the heap. No C++ objects are
// constructed in this frame while libjpeg may invoke the longjmp error exit.
bool decodeJpeg(JpegState* state,const void* data,std::size_t size) {
    state->image.err=jpeg_std_error(&state->error.api);state->error.api.error_exit=&jpegError;state->error.api.output_message=&jpegMessage;
    if(setjmp(state->error.target)) return false;
    jpeg_create_decompress(&state->image);
    state->source.data=static_cast<const JOCTET*>(data);state->source.size=size;state->source.supplied=false;
    auto& source=state->source.api;source.init_source=&jpegSourceInit;source.fill_input_buffer=&jpegSourceFill;source.skip_input_data=&jpegSourceSkip;
    source.resync_to_restart=&jpeg_resync_to_restart;source.term_source=&jpegSourceTerm;source.next_input_byte=nullptr;source.bytes_in_buffer=0;
    state->image.src=&source;
    if(jpeg_read_header(&state->image,TRUE)!=JPEG_HEADER_OK) return false;
    state->ycc=state->image.jpeg_color_space==JCS_YCbCr;
    if(state->ycc) state->image.out_color_space=JCS_YCbCr;
    if(!jpeg_start_decompress(&state->image)) return false;
    state->width=state->image.output_width;state->height=state->image.output_height;state->components=state->image.output_components;
    if(!state->width || !state->height || (state->components!=1 && state->components!=3)) return false;
    if(state->width>(std::numeric_limits<unsigned>::max)()/4) return false;
    state->pitch=state->width*(state->components==1 ? 1:4);
    if(state->height>(std::numeric_limits<std::size_t>::max)()/state->pitch) return false;
    state->pixels=static_cast<unsigned char*>(std::malloc(std::size_t(state->pitch)*state->height));if(!state->pixels) return false;
    JSAMPARRAY row=(*state->image.mem->alloc_sarray)(reinterpret_cast<j_common_ptr>(&state->image),JPOOL_IMAGE,state->width*state->components,1);
    while(state->image.output_scanline<state->height) {
        auto* output=state->pixels+std::size_t(state->image.output_scanline)*state->pitch;
        if(jpeg_read_scanlines(&state->image,row,1)!=1) return false;
        if(state->components==1) std::memcpy(output,row[0],state->width);
        else for(unsigned x=0;x<state->width;++x) {
            const auto* input=row[0]+3*x;
            const auto rgb=state->ycc ? jpegYCbCrToRGB(input[0],input[1],input[2],jpegUsesMMX() && x<(state->width&~7U)):
                std::array<unsigned char,3>{input[0],input[1],input[2]};
            output[4*x]=rgb[2];output[4*x+1]=rgb[1];output[4*x+2]=rgb[0];output[4*x+3]=0;
        }
    }
    return jpeg_finish_decompress(&state->image)!=FALSE;
}
struct Cleanup {void operator()(JpegState* state) const {if(state->image.mem) jpeg_destroy_decompress(&state->image);std::free(state->pixels);delete state;}};
}
NativeImage decodeJpegImage(const void* data,std::size_t size) {
    std::unique_ptr<JpegState,Cleanup> state(new JpegState);
    if(!decodeJpeg(state.get(),data,size)) throw std::invalid_argument("Original JPEG image decoder rejected input");
    NativeImage result;result.planes.push_back({state->components==1 ? D3DFMT_L8:D3DFMT_X8R8G8B8,state->width,state->height,state->pitch,
        {state->pixels,state->pixels+std::size_t(state->pitch)*state->height}});return result;
}
}
