#include "pegapega.h"

#include "tilemap.h"

#include <cstdio>

void PegaPega::start(Tilemap& map) {
    npc.setSheet(map.character.sheet());
    npc.setPaletteRow(Character::DEFAULT_PALETTE_ROW + Character::PALETTE_BLOCK_ROWS);
    npc.setFacing(Direction::S);

    status_ = Status::Running;
    position_ = map.randomWalkableTile(map.player);
    catches_ = 0;
    timeLeft_ = START_TIME;

    map.setGameMode(Tilemap::GameMode::PegaPega);
}

void PegaPega::update(double dt) {
    if (status_ != Status::Running) {
        return;
    }
    timeLeft_ -= dt;
    if (timeLeft_ <= 0.0) {
        timeLeft_ = 0.0;
        status_ = Status::Lost;
    }
}

bool PegaPega::onPlayerMoved(Tilemap& map) {
    if (status_ != Status::Running) {
        return false;
    }
    if (map.player.row != position_.row || map.player.col != position_.col) {
        return false;
    }

    ++catches_;
    timeLeft_ += CATCH_BONUS;
    if (catches_ >= GOAL) {
        status_ = Status::Won;
    } else {
        position_ = map.randomWalkableTile(map.player);
    }
    return true;
}

bool PegaPega::isRunning() const {
    return status_ == Status::Running;
}

PegaPega::Status PegaPega::status() const {
    return status_;
}

int PegaPega::catches() const {
    return catches_;
}

double PegaPega::timeLeft() const {
    return timeLeft_;
}

GridPos PegaPega::position() const {
    return position_;
}

std::string PegaPega::hudText() const {
    char buffer[160];
    if (status_ == Status::Won) {
        std::snprintf(buffer, sizeof(buffer), "PEGA-PEGA  VOCE VENCEU!  Capturas: %d/%d",
                      catches_, GOAL);
    } else if (status_ == Status::Lost) {
        std::snprintf(buffer, sizeof(buffer), "PEGA-PEGA  FIM DE JOGO  Capturas: %d/%d",
                      catches_, GOAL);
    } else {
        std::snprintf(buffer, sizeof(buffer), "PEGA-PEGA  Capturas: %d/%d  Tempo: %.1fs",
                      catches_, GOAL, timeLeft_);
    }
    return std::string(buffer);
}
