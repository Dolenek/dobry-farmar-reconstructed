#include "PathNames.h"
#include <iostream>
#include <string>

int main() {
    using farmar::recovered::PathNames;
    int failures = 0;
    auto check = [&](const farmar::recovered::String& actual, const std::string& expected) {
        if (actual.view() != expected) {
            std::cerr << "Expected '" << expected << "', got '" << actual.view() << "'\n";
            ++failures;
        }
    };
    check(PathNames::getFileExt("Data/Images/FARM.JNG"), "jng");
    check(PathNames::getFileExt("archive.tar.GZ"), "gz");
    check(PathNames::getFileExt(".PROFILE"), "profile");
    check(PathNames::getFileExt("Farm."), "");
    check(PathNames::getFileExt("Farm"), "");
    check(PathNames::getFileExt(""), "");
    // Verified quirks: no path-separator check for extension; no '\\' handling for name.
    check(PathNames::getFileExt("Folder.DOT/File"), "dot/file");
    check(PathNames::getFileName("Data/Images/FARM.JNG"), "farm.jng");
    check(PathNames::getFileName("Data\\Images\\FARM.JNG"), "data\\images\\farm.jng");
    check(PathNames::getFileName("Data/"), "");
    check(PathNames::getFileName(""), "");
    const auto& extension = PathNames::getFileExt("first.JNG");
    const auto& name = PathNames::getFileName("Folder/FIRST.JNG");
    const auto* extensionStorage = &extension;
    check(name, "first.jng");
    check(extension, "jng");
    if (&PathNames::getFileExt("second.XML") != extensionStorage) ++failures;
    check(extension, "xml"); // Original returns a reference to reused static storage.
    check(name, "first.jng"); // Separate storage for the second method.
    return failures ? 1 : 0;
}
