// pegapega.cpp
// As regras do mini-jogo Pega-Pega: sorteia um tile caminhavel para o alvo,
// conta as capturas, controla o tempo (cada captura da bonus) e decide vitoria
// (ao atingir a meta) ou derrota (quando o tempo zera). Tambem monta o texto do
// HUD.
#include "pegapega.h"

#include "tilemap.h"

#include <cstdio>

// Inicia uma partida: configura o NPC (com uma paleta de cor diferente do jogador),
// sorteia a primeira posicao do alvo, zera o placar/cronometro e poe o mapa no modo
// Pega-Pega.
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

// Avanca o cronometro a cada quadro; se o tempo zerar, o jogador perde.
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

// Chamado quando o jogador anda. Se ele caiu sobre o alvo, conta uma captura, da
// o bonus de tempo e ou vence (atingiu a meta) ou sorteia uma nova posicao do alvo.
// Devolve 'true' apenas quando houve captura.
bool PegaPega::onPlayerMoved(Tilemap& map) {
    if (status_ != Status::Running) {
        return false;
    }
    // So conta se o jogador estiver exatamente na celula do alvo.
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

// Monta a linha de texto do HUD conforme o estado: vitoria, fim de jogo ou a
// partida em andamento (mostrando capturas e tempo restante).
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
