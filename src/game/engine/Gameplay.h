#pragma once
#include "GameplayEvents.h"
#include "GameRandom.h"
namespace farmar::recovered {
class ResourceManager;class ProfileProperties;class Gameplay;
// Required world operations: no default implementation or replacement game.
// The recovered world controller must supply the original ten dispatches and
// the actual contract controller's +94 state availability (0042f030).
class GameplayEventExecutor {
public:
    virtual ~GameplayEventExecutor()=default;
    virtual bool contractAvailable() const=0;
    virtual void execute(GameplayEvent&)=0;
};
class GameplayLifetime {
protected:
    explicit GameplayLifetime(Gameplay*);
    ~GameplayLifetime();
};
class Gameplay final:private GameplayLifetime {
public:
    Gameplay(ResourceManager&,ProfileProperties&,GameRandom&,GameplayEventExecutor&); // 004b59c0
    Gameplay(const XmlNode&,ProfileProperties&,GameRandom&,GameplayEventExecutor&); // explicit XML input
    ~Gameplay(); // 004aeb10
    Gameplay(const Gameplay&)=delete;
    Gameplay& operator=(const Gameplay&)=delete;
    void step(); // 004b5c60, at most one event; contract may be skipped
    void applyCoefficients(); // 004ae570, cumulative updates
    std::vector<float> coefficients() const {return coefficients_;} // 004b4ac0 copy
    void restoreCoefficients(const std::vector<float>&); // 004b4ad0 copy then apply
    std::int32_t effectivePrice(unsigned product); // 004b5ea0, MEffPrice; native accepts index 19
    std::vector<float> priceForecast(unsigned product,float interval,unsigned count); // 004b62a0
    const GameplayEventDefinitions& definitions() const noexcept {return definitions_;}
    float endSeconds() const noexcept {return endSeconds_;}
    std::size_t currentEventIndex() const noexcept {return currentEvent_;}
    static Gameplay* instance() noexcept;
private:
    ProfileProperties& profile_;GameRandom& random_;GameplayEventExecutor& executor_;
    GameplayEventDefinitions definitions_;
    float endSeconds_=0;
    std::vector<float> coefficients_;
    std::size_t currentEvent_=0;
    void initialize();
    void regenerateCoefficients();
};
}
