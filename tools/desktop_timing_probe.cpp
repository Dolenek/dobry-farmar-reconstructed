// Read-only DWM counters in a disposable, externally time-bounded process.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dwmapi.h>
#include <iostream>
int main() {
    DWM_TIMING_INFO value{};value.cbSize=sizeof(value);
    const auto result=DwmGetCompositionTimingInfo(nullptr,&value);
    std::cout<<"{\"hresult\":"<<static_cast<unsigned long>(result)
        <<",\"refresh\":"<<value.cRefresh<<",\"frame\":"<<value.cFrame
        <<",\"frame_displayed\":"<<value.cFrameDisplayed<<",\"frame_complete\":"<<value.cFrameComplete
        <<",\"qpc_vblank\":"<<value.qpcVBlank<<",\"frames_outstanding\":"<<value.cFramesOutstanding
        <<",\"refresh_rate_numerator\":"<<value.rateRefresh.uiNumerator
        <<",\"refresh_rate_denominator\":"<<value.rateRefresh.uiDenominator<<"}\n";
    return FAILED(result) ? 1:0;
}
