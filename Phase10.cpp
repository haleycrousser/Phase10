#include "Card/card.h"
#include "Deck/deck.h"
#include "Player/player.h"
#include "Game/game.h"
#include "Phase/phase.h"

#include <iostream>

using namespace std;

int main() {

    Deck deck;
    Player players;

    cout << "--------- Initialized Deck ---------\n";

    deck.printStandardDeck();

    deck.shuffleDeck();

    cout << "--------- Shuffled Deck ---------\n";

    deck.printStandardDeck();

    cout << "How many players? (2-6): ";
    cin >> players.playersAmt;

    cout << "\n---------" << players.playersAmt << " Players in game---------" << endl;


    return 0;
}