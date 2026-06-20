// pegapega.h
// Contrato do mini-jogo Pega-Pega: guarda o estado (parado/rodando/venceu/
// perdeu), o tempo restante, o numero de capturas e a posicao do alvo (NPC).
#pragma once

#include "character.h"
#include "types.h"

#include <string>

class Tilemap;

class PegaPega {
public:
    // Estados possiveis do mini-jogo.
    enum class Status { Inactive, Running, Won, Lost };

    // Meta de capturas para vencer, tempo inicial e bonus de tempo por captura.
    static constexpr int GOAL = 10;
    static constexpr double START_TIME = 10.0;
    static constexpr double CATCH_BONUS = 2.0;

    // O alvo a ser capturado.
    Character npc;

    // Inicia uma partida, avanca o cronometro e reage a cada movimento do jogador
    // (retorna 'true' quando ocorre uma captura).
    void start(Tilemap& map);
    void update(double dt);
    bool onPlayerMoved(Tilemap& map);

    // Consultas de estado para a logica do jogo e o desenho do HUD.
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
