// Test-only parser oracle. Maps the original image without entry-point or DLL
// initialization. Supplies our allocator through its recovered internal ABI.
// No original code is linked into the reconstruction or any runtime target.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "reference_image.h"
#include <expat.h>
#include <cstdlib>
#include <process.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace {
struct Event {
    char kind;
    std::wstring text;
    std::vector<std::wstring> attributes;
    bool operator==(const Event& other) const { return kind==other.kind && text==other.text && attributes==other.attributes; }
};
using Events = std::vector<Event>;
void start(void* context, const wchar_t* name, const wchar_t** attributes) {
    Event event{'S', name, {}};
    for (auto at=attributes; *at; ++at) event.attributes.emplace_back(*at);
    static_cast<Events*>(context)->push_back(std::move(event));
}
void end(void* context, const wchar_t* name) { static_cast<Events*>(context)->push_back({'E', name, {}}); }
void characters(void* context, const wchar_t* text, int length) {
    auto& events = *static_cast<Events*>(context);
    if (!events.empty() && events.back().kind=='C') events.back().text.append(text, length);
    else events.push_back({'C', std::wstring(text, length), {}});
}
struct API {
    decltype(&XML_SetUserData) user;
    decltype(&XML_SetElementHandler) elements;
    decltype(&XML_SetCharacterDataHandler) text;
    decltype(&XML_Parse) parse;
    decltype(&XML_GetErrorCode) error;
    decltype(&XML_GetCurrentLineNumber) line;
    decltype(&XML_GetCurrentColumnNumber) column;
    decltype(&XML_ParserFree) free;
};
struct Result {
    Events events;
    int status;
    XML_Error error;
    int line, column;
    bool operator==(const Result& other) const {
        return events==other.events && status==other.status && error==other.error && line==other.line && column==other.column;
    }
};
XML_Parser createOriginal(void* function, const XML_Memory_Handling_Suite* memory) {
    XML_Parser result;
    // 0056e5c0: memsuite in EAX, encoding / separator / DTD on stack.
    __asm {
        mov eax, memory
        push 0
        push 0
        push 0
        call function
        add esp, 12
        mov result, eax
    }
    return result;
}
Result run(const API& api, XML_Parser parser, const std::string& bytes, unsigned chunk) {
    if (!parser) throw std::runtime_error("Parser allocation failed");
    Result result{};
    api.user(parser, &result.events);
    api.elements(parser, start, end);
    api.text(parser, characters);
    std::size_t at = 0;
    do {
        const auto count = (std::min)(static_cast<std::size_t>(chunk), bytes.size()-at);
        result.status = api.parse(parser, bytes.data()+at, static_cast<int>(count), at+count==bytes.size());
        at += count;
        if (!result.status) break;
    } while (at < bytes.size());
    result.error = api.error(parser);
    result.line = api.line(parser);
    result.column = api.column(parser);
    api.free(parser);
    return result;
}
}
int runMain(int argc, wchar_t** argv) {
    if (argc < 3) { std::cerr << "Usage: xml-reference-test original.exe input.xml [...]\n"; return 2; }
    ReferenceImage image(std::filesystem::absolute(argv[1]));
    auto* base = image.base();
    std::cerr << "Reference mapped\n";
    API original{
        reinterpret_cast<decltype(API::user)>(base+0x167c20),
        reinterpret_cast<decltype(API::elements)>(base+0x167c40),
        reinterpret_cast<decltype(API::text)>(base+0x167c60),
        reinterpret_cast<decltype(API::parse)>(base+0x169ce0),
        reinterpret_cast<decltype(API::error)>(base+0x168340),
        reinterpret_cast<decltype(API::line)>(base+0x167df0),
        reinterpret_cast<decltype(API::column)>(base+0x167e40),
        reinterpret_cast<decltype(API::free)>(base+0x169b90)
    };
    const API recovered{XML_SetUserData, XML_SetElementHandler, XML_SetCharacterDataHandler,
                        XML_Parse, XML_GetErrorCode, XML_GetCurrentLineNumber, XML_GetCurrentColumnNumber, XML_ParserFree};
    const XML_Memory_Handling_Suite memory{std::malloc, std::realloc, std::free};
    const auto errorString = reinterpret_cast<decltype(&XML_ErrorString)>(base+0x167e90);
    for (unsigned at=0; at<40; ++at) {
        const auto a = errorString(static_cast<XML_Error>(at));
        const auto b = XML_ErrorString(static_cast<XML_Error>(at));
        if ((a==nullptr)!=(b==nullptr) || (a && std::wstring(a)!=b)) {
            std::cerr << "Error text mismatch: " << at << '\n'; return 1;
        }
    }
    unsigned comparisons = 40;
    std::cerr << "Error texts compared\n";
    for (int input=2; input<argc; ++input) {
        std::ifstream file(std::filesystem::path(argv[input]), std::ios::binary);
        if (!file) { std::cerr << "Cannot read input\n"; return 2; }
        const std::string bytes{std::istreambuf_iterator<char>(file), {}};
        for (const auto chunk : {1U, 7U, 4096U, 0x7fffffffU}) {
            std::cerr << "Comparing " << input-2 << '/' << chunk << '\n';
            const auto a = run(original, createOriginal(base+0x16e5c0, &memory), bytes, chunk);
            const auto b = run(recovered, XML_ParserCreate_MM(nullptr, &memory, nullptr), bytes, chunk);
            if (!(a==b)) {
                std::cerr << "Parser mismatch input " << input-2 << ", chunk " << chunk
                    << ", original " << a.status << '/' << a.error << '/' << a.line << '/' << a.column
                    << ", recovered " << b.status << '/' << b.error << '/' << b.line << '/' << b.column
                    << ", events " << a.events.size() << '/' << b.events.size() << '\n';
                return 1;
            }
            ++comparisons;
        }
    }
    std::cout << comparisons << '\n';
    return 0;
}
int wmain(int argc, wchar_t** argv) {
    try { return runMain(argc, argv); }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 2; }
}
