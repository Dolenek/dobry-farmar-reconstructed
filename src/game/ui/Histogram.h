#pragma once
#include "Control.h"
#include <vector>
namespace farmar::recovered {
class ProgressBar;
// Native cHistogram<_Bar>, 0046d420, 576 bytes. The bars are View-owned.
class Histogram final : public Control {
public:
    using Control::Control;
    void loadXML(const XmlNode&) override; // 0046d560: replaces and deletes old bars
    void applyValues(const std::vector<float>&); // 0046b730
    void setMaximum(float); // 0046b6f0, does not overwrite Threshold
    float threshold=100; // native +22c, XML property
    const std::vector<ProgressBar*>& bars() const noexcept {return bars_;}
private:
    std::vector<ProgressBar*> bars_; // native +230 allocator, +234 begin
};
}
