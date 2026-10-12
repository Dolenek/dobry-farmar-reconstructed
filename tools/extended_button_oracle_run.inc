const auto runExtendedButtons=[&](auto* tag) {
#include "extended_button_oracle_cases.inc"
};
if(extendedPictureButtons) runExtendedButtons(static_cast<ExtendedPictureButton*>(nullptr));
else if(extendedAnimationButtons) runExtendedButtons(static_cast<ExtendedAnimationButton*>(nullptr));
else runExtendedButtons(static_cast<ExtendedTextAnimationButton*>(nullptr));
