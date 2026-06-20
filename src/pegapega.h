#pragma once

#include "character.h"
#include "direction.h"

#include <string>

class Tilemap;

class PegaPega {
public:
    enum class Status { Inactive, Running, Won, Lost };

    static constexpr int GOAL = 10;
    static constexpr double START_TIME = 10.0;
    static constexpr double CATCH_BONUS = 2.0;

    Character npc;

    void start(Tilemap& map);
    void update(double dt);
    bool onPlayerMoved(Tilemap& map);

    bool isRunning() const;
    Status status() const;
    int catches() const;
    double timeLeft() const;
    GridPos position() const;
    std::string hudText() const;

private:
    Status status_ = Status::Inactive;
    GridPos position_{0, 0};
    int catches_ = 0;
    double timeLeft_ = 0.0;
};
