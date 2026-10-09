#include "GameWString.h"
#include <iostream>
#include <stdexcept>

using farmar::recovered::WString;
void check(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
int main() {
    try {
        WString text(L"1234567");
        check(text.capacity()==7, "UTF-16 SSO boundary");
        text.append(L"8", 1);
        check(text.view()==L"12345678" && text.capacity()==15, "UTF-16 first heap capacity");
        text.append(text);
        check(text.view()==L"1234567812345678" && text.capacity()==23, "Self append across heap growth");
        text.assign(text.data()+6, 4);
        check(text.view()==L"7812", "Aliased substring assignment");
        const auto capacity=text.capacity();
        text.erase();
        check(text.empty() && text.capacity()==capacity && text.data()[0]==0, "Erase retains allocation");
        const wchar_t units[]{L'\0', L'\u0100', L'\u00ff', static_cast<wchar_t>(0xffff)};
        text.assign(units, 4);
        check(text.size()==4 && text.compare(1, 1, units+2, 1)==1, "Unsigned UTF-16 unit order");
        text.append(text.data(), 4);
        check(text.size()==8 && text.data()[4]==0 && text.data()[7]==0xffff, "Embedded NUL append");
        WString copy(text);
        check(copy.view()==text.view(), "Independent UTF-16 copy");
        copy.erase(1, 2);
        check(copy.size()==6 && copy.data()[1]==0xffff && text.size()==8, "UTF-16 erase units");
        bool threw=false;
        try { copy.assign(text, 9); } catch (const std::out_of_range&) { threw=true; }
        check(threw, "Invalid substring offset");
        std::cout << "UTF-16 ownership, aliasing and boundaries passed\n";
        return 0;
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
