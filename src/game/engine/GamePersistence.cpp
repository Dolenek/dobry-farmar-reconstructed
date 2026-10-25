#include "GamePersistence.h"
#include "FileIndex.h"
#include "Profile.h"
#include "Farm.h"
#include "GameMenu.h"
#include "Gameplay.h"
#include "SalesHistory.h"
#include "OriginalArithmetic.h"
#include <array>
#include <cstring>
#include <vector>
namespace farmar::recovered {
namespace {
struct SavedSeed {
    Point2 position{};unsigned kind,count,status;unsigned char unpaid,padding[3];
};
static_assert(sizeof(SavedSeed)==24 && sizeof(SaleRecord)==8 && sizeof(wchar_t)==2,"Original binary save widths");
template<class T> void write(iFile& file,const T& value) {file.write(&value,sizeof value);}
template<class T> void read(iFile& file,T& value) {file.read(&value,sizeof value);}
}
String GamePersistence::filename() const {
    std::string directory(profile_.directory().view());
    if(directory.empty() || directory.back()!='/') directory+='/';
    directory+=profile_.headers().currentID().view();directory+=".save";return String(directory);
}
void GamePersistence::save() {
    auto file=files_.create(filename());
    const auto seeds=farm_.seedPackages();write(*file,unsigned(seeds.size()));
    for(auto* area:seeds) {
        auto& seed=static_cast<SeedPackage&>(*area);const auto& geometry=seed.geometry();const auto& origin=farm_.movingPicture().geometry();
        // Native writes three unspecified padding bytes. Emit zeroes; readers
        // do not inspect them. Every meaningful field retains the native width.
        SavedSeed saved{};saved.position={float(originalSubtract(geometry.x,origin.x)),float(originalSubtract(geometry.y,origin.y))};
        saved.kind=seed.kind();saved.count=seed.count();saved.status=seed.status();saved.unpaid=seed.unpaid;write(*file,saved);
    }
    //0040f480 copies the list before writing. Preserve that snapshot boundary.
    const auto sales=sales_.records();write(*file,unsigned(sales.size()));write(*file,sales_.cutoff());
    for(const auto& sale:sales) write(*file,sale);
    const auto& messages=hud_.messages();write(*file,unsigned(messages.size()));
    for(const auto& message:messages) {
        write(*file,static_cast<unsigned char>(message.read));write(*file,message.text.size());file->write(message.text.data(),message.text.size()*2);
    }
    const auto coefficients=gameplay_.coefficients();write(*file,unsigned(coefficients.size()));file->write(coefficients.data(),unsigned(coefficients.size()*4));
}
void GamePersistence::restore() {
    const auto path=filename();if(!files_.isExist(path)) save();auto file=files_.open(path);
    unsigned count=0;read(*file,count);std::vector<SavedSeed> seeds(count);file->read(seeds.data(),count*24);
    for(const auto& seed:seeds) farm_.restoreSeed(seed.kind,seed.count,seed.position,seed.status,seed.unpaid!=0);
    count=0;float cutoff=0;read(*file,count);read(*file,cutoff);
    for(unsigned i=0;i<count;++i) {SaleRecord sale{};read(*file,sale);sales_.record(sale.amount,sale.time);}
    sales_.restoreCutoff(cutoff);
    count=0;read(*file,count);std::vector<GameMessage> messages(count);
    for(auto& message:messages) {
        unsigned char flag=0;unsigned length=0;read(*file,flag);message.read=flag!=0;read(*file,length);
        if(length) {std::vector<wchar_t> text(std::size_t(length)+1,0);file->read(text.data(),length*2);message.text=WString(text.data());}
    }
    hud_.assignMessages(messages);
    count=0;read(*file,count);std::vector<float> coefficients(count,0);file->read(coefficients.data(),count*4);gameplay_.restoreCoefficients(coefficients);
}
}
