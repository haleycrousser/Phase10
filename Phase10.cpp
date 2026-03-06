#include "Card/card.h"
#include "Deck/deck.h"
#include "Player/player.h"
#include "Game/game.h"
#include "Phase/phase.h"

#include <iostream>

using namespace std;

int main() {

    Deck deck;

    cout << "--------- Initialized Deck ---------\n";

    deck.printStandardDeck();

    deck.shuffleDeck();

    cout << "--------- Shuffled Deck ---------\n";

    deck.printStandardDeck();


    return 0;
}