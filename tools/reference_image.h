#pragma once
// Test-only, uninitialized reference PE mapping. The original EXE has stripped
// relocations, so pure original routines which access data require 00400000.
// This file must never be included by a reconstructed runtime target.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstring>
#include <cstdint>
#include <string>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>
extern "C" __declspec(dllimport) void* referenceReservationBase();

class ReferenceImage {
public:
    explicit ReferenceImage(const std::filesystem::path& path) {
        // Reserve BEFORE allocating the multi-megabyte file vector: the CRT heap
        // can otherwise consume the very address range needed by this fixed PE.
        base_ = static_cast<unsigned char*>(referenceReservationBase());
        DWORD oldProtection;
        if (!VirtualProtect(base_, 3*1024*1024U, PAGE_READWRITE, &oldProtection)) base_ = nullptr;
        if (!base_) {
            const auto error = GetLastError();
            MEMORY_BASIC_INFORMATION info{};
            VirtualQuery(reinterpret_cast<void*>(0x400000U), &info, sizeof(info));
            throw std::runtime_error("Original preferred address unavailable: error="+std::to_string(error)+
                ", allocation="+std::to_string(reinterpret_cast<std::uintptr_t>(info.AllocationBase))+
                ", state="+std::to_string(info.State)+", type="+std::to_string(info.Type)+
                ", region="+std::to_string(info.RegionSize)+", executable="+
                std::to_string(reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr))));
        }
        struct Guard {
            unsigned char*& base;
            bool complete = false;
            ~Guard() { if (!complete) base=nullptr; }
        } guard{base_};
        std::ifstream input(path, std::ios::binary);
        if (!input) throw std::runtime_error("Cannot read reference PE");
        const std::vector<char> bytes{std::istreambuf_iterator<char>(input), {}};
        if (bytes.size()<sizeof(IMAGE_DOS_HEADER)) throw std::runtime_error("Truncated PE");
        const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(bytes.data());
        if (dos->e_magic!=IMAGE_DOS_SIGNATURE || dos->e_lfanew<0 ||
            static_cast<std::size_t>(dos->e_lfanew)+sizeof(IMAGE_NT_HEADERS32)>bytes.size())
            throw std::runtime_error("Invalid DOS header");
        const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS32*>(bytes.data()+dos->e_lfanew);
        if (nt->Signature!=IMAGE_NT_SIGNATURE || nt->FileHeader.Machine!=IMAGE_FILE_MACHINE_I386 ||
            nt->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC || nt->OptionalHeader.ImageBase!=0x400000U ||
            nt->OptionalHeader.SizeOfImage>3*1024*1024U || nt->OptionalHeader.SizeOfHeaders>bytes.size())
            throw std::runtime_error("Unexpected reference PE32 layout");
        const auto* sections = IMAGE_FIRST_SECTION(nt);
        if (reinterpret_cast<const char*>(sections+nt->FileHeader.NumberOfSections)>bytes.data()+bytes.size())
            throw std::runtime_error("Truncated PE sections");
        try {
            std::memcpy(base_, bytes.data(), nt->OptionalHeader.SizeOfHeaders);
            for (unsigned at=0; at<nt->FileHeader.NumberOfSections; ++at) {
                const auto& section = sections[at];
                if (static_cast<std::uint64_t>(section.PointerToRawData)+section.SizeOfRawData>bytes.size() ||
                    static_cast<std::uint64_t>(section.VirtualAddress)+section.SizeOfRawData>nt->OptionalHeader.SizeOfImage)
                    throw std::runtime_error("Invalid PE section range");
                std::memcpy(base_+section.VirtualAddress, bytes.data()+section.PointerToRawData, section.SizeOfRawData);
                DWORD old;
                const auto protection = (section.Characteristics&IMAGE_SCN_MEM_EXECUTE) ? PAGE_EXECUTE_READ :
                    ((section.Characteristics&IMAGE_SCN_MEM_WRITE) ? PAGE_READWRITE : PAGE_READONLY);
                const auto size = (std::max)(section.Misc.VirtualSize, section.SizeOfRawData);
                if (!VirtualProtect(base_+section.VirtualAddress, size, protection, &old))
                    throw std::runtime_error("Cannot protect reference section");
            }
            FlushInstructionCache(GetCurrentProcess(), base_, nt->OptionalHeader.SizeOfImage);
        } catch (...) { base_ = nullptr; throw; }
        // Imports, TLS, CRT initializers and the entry point are intentionally not run.
        guard.complete = true;
    }
    ~ReferenceImage() = default; // loader-owned reservation; test exits after oracle
    ReferenceImage(const ReferenceImage&) = delete;
    ReferenceImage& operator=(const ReferenceImage&) = delete;
    unsigned char* base() const noexcept { return base_; }
private:
    unsigned char* base_ = nullptr;
};
