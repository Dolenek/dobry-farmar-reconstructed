// Integrated reconstructed resource path: original file index -> iFile -> XML
// tree or JNGLoad. Diagnostic only; not presented as the reconstructed game EXE.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "FileIndex.h"
#include "XmlDocument.h"
#include "Settings.h"
#include "JNGLoad.h"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace farmar::recovered;
namespace {
String ansi(const wchar_t* text) {
    const auto count = WideCharToMultiByte(CP_ACP, 0, text, -1, nullptr, 0, nullptr, nullptr);
    if (!count) throw std::runtime_error("Cannot convert diagnostic path to original ANSI");
    std::string result(count, '\0');
    WideCharToMultiByte(CP_ACP, 0, text, -1, result.data(), count, nullptr, nullptr);
    return String(std::string_view(result.data(), count-1));
}
template<class Char> void quoted(std::ostream& output, std::basic_string_view<Char> text) {
    output << '"';
    for (const auto character : text) {
        const auto unit = sizeof(Char)==1 ? static_cast<unsigned char>(character) : static_cast<unsigned short>(character);
        if (unit=='"' || unit=='\\') output << '\\' << static_cast<char>(unit);
        else if (unit<32 || unit>126) output << "\\u" << std::hex << std::setw(4) << std::setfill('0') << unit << std::dec;
        else output << static_cast<char>(unit);
    }
    output << '"';
}
void tree(std::ostream& output, const XmlNode& node) {
    output << "{\"attributes\":{";
    bool comma=false;
    for (const auto& attribute : node.attributes()) {
        if (comma) output << ',';
        comma=true;
        quoted(output, std::wstring_view(attribute.first)); output << ':';
        quoted(output, attribute.second.view());
    }
    output << "},\"text\":"; quoted(output, node.text().view());
    output << ",\"children\":[";
    comma=false;
    for (const auto& child : node.children()) {
        if (comma) output << ',';
        comma=true;
        output << "{\"name\":"; quoted(output, std::string_view(child.first));
        output << ",\"node\":"; tree(output, *child.second); output << '}';
    }
    output << "]}";
}
std::vector<unsigned char> read(FileIndex& files, const String& name) {
    auto file=files.open(name);
    std::vector<unsigned char> result(file->size());
    unsigned at=0;
    while (at<result.size()) {
        const auto count=file->read(result.data()+at, (std::min)(8192U, static_cast<unsigned>(result.size()-at)));
        if (!count || count>result.size()-at) throw std::runtime_error("Invalid resource read count");
        at+=count;
    }
    unsigned char extra;
    if (file->read(&extra, 1)) throw std::runtime_error("Resource exceeds cached size");
    const auto size=file->size();
    file->close(); file->close();
    // cRealFile returns zero after close; cZipFile retains its cached entry size.
    if ((file->size()!=size && file->size()!=0) || file->read(&extra, 1)!=0)
        throw std::runtime_error("Closed resource state mismatch");
    return result;
}
}
int wmain(int argc, wchar_t** argv) {
    if (argc!=5 && argc!=6) {
        std::cerr << "Usage: resource-inspect xml|file|image|index|settings data-root resource output [common-root]\n"; return 2;
    }
    try {
        FileIndex files(ansi(argv[2]), String("pack"));
        if (argc==6) files.mountCommon(ansi(argv[5]));
        const auto name=ansi(argv[3]);
        std::ofstream output(std::filesystem::path(argv[4]), std::ios::binary);
        if (!output) throw std::runtime_error("Cannot create diagnostic output");
        const std::wstring mode=argv[1];
        if (mode==L"settings") {
            Settings settings(files);
            output << '{'; bool sectionComma=false;
            for (const auto& section : settings.sections()) {
                if (sectionComma) output << ',';
                sectionComma=true;
                quoted(output, std::string_view(section.first)); output << ":{";
                bool propertyComma=false;
                for (const auto& property : section.second) {
                    if (propertyComma) output << ',';
                    propertyComma=true;
                    quoted(output, std::string_view(property.first)); output << ':';
                    quoted(output, property.second.view());
                }
                output << '}';
            }
            output << "}\n";
        } else if (mode==L"xml") {
            XmlDocument document(files, name);
            tree(output, document); output << '\n';
        } else if (mode==L"file") {
            const auto bytes=read(files, name);
            output.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
        } else if (mode==L"image") {
            const auto bytes=read(files, name);
            unsigned long width=0, height=0;
            unsigned long* pixels=nullptr;
            readMNG(bytes.data(), width, height, pixels);
            output.write(reinterpret_cast<const char*>(&width), 4);
            output.write(reinterpret_cast<const char*>(&height), 4);
            output.write(reinterpret_cast<const char*>(pixels), static_cast<std::size_t>(width)*height*4);
            freeData(pixels);
        } else if (mode==L"index") {
            output << '{'; bool comma=false;
            for (const auto& entry : files.files()) {
                if (comma) output << ','; comma=true;
                quoted(output, std::string_view(entry.first)); output << ':';
                quoted(output, std::string_view(entry.second));
            }
            output << "}\n";
        } else throw std::runtime_error("Unknown diagnostic mode");
        if (!output) throw std::runtime_error("Diagnostic output failed");
        return 0;
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
    catch (const char* error) { std::cerr << error << '\n'; return 1; }
}
