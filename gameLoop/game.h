#ifndef GAME_H
#define GAME_H

#include "card.h"
#include "deck.h"
#include "player.h"

class Game {
	void turn_startPickUp (Card userPick, Deck standardOrDiscard);
	void turn_endPlaceDown(Card userPick);
	void printAllPlayerHands();
};

#endif
