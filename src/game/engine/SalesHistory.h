#pragma once
#include <cstdint>
#include <list>
namespace farmar::recovered {
class ProfileProperties;
struct SaleRecord {float time;std::uint32_t amount;};
// Actual cGame members +9f4..+a04 and their two original operations. Modern
// list storage is private; saved-game serialization must translate explicitly.
class SalesHistory final {
public:
    explicit SalesHistory(ProfileProperties& profile):profile_(profile) {} // 00413210 initializes sum/cutoff to zero
    void prune(float time); // 0040c2b0, original successor-skipping walk
    void record(std::uint32_t amount,float time); // 00410f10, persistent awEarn
    const std::list<SaleRecord>& records() const noexcept {return records_;}
    std::uint32_t total() const noexcept {return total_;}
    float cutoff() const noexcept {return cutoff_;}
private:
    ProfileProperties& profile_;
    std::list<SaleRecord> records_;
    std::uint32_t total_=0;float cutoff_=0;
};
}
