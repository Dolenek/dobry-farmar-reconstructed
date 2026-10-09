#pragma once
namespace farmar::recovered {
class ProfileProperties;
void initializeGameProfile(ProfileProperties& values); // cGame::onInitProfile 00411390
const char* awardPropertyName(unsigned index); // 0040ab50, indices 0..20
}
