// pasazer.h - Definicje funkcji procesów pasażerów, dzieci i rodziców
#ifndef PASAZER_H
#define PASAZER_H

void proces_pasazer(int id_pasazera);
void proces_dziecko(int id_pasazera);
void proces_rodzic(int id_pasazera, int idx_dziecka);

#endif
