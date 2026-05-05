#include "deck.h"
#include <iostream>
#include <algorithm>
#include <random>
#include <stdexcept>
#include <thread>
#include <chrono>

using namespace std;

Deck::Deck() {
    buildDeck();
    shuffleDeck();
}

void Deck::buildDeck() {
    cards.clear();
    cards.reserve(108);

    auto pointsFor = [](int num) -> int { //set points based on card number
        return (num <= 9) ? 5 : 10;
    };

//variables for card properties, used to create card objects and push to deck vector

    const string colors[]   = { "Red", "Yellow", "Blue", "Green" };
    const char   variants[] = { 'a', 'b' };

//2 of each color/number or variant combination

    for (const string& color : colors)
        for (int num = 1; num <= 12; num++)
            for (char variant : variants)
                cards.push_back(Card(color, num, variant, pointsFor(num), false, false));

// 8 wilds, 4 skips
    for (int i = 0; i < 8; i++)
        cards.push_back(Card("Wild", 0, (i < 4) ? 'a' : 'b', 25, true, false));

    for (int i = 0; i < 4; i++)
        cards.push_back(Card("Skip", 0, (i < 2) ? 'a' : 'b', 15, false, true));
}

//easy shuffle using built-in random generator
void Deck::shuffleDeck() {
    mt19937 rng(random_device{}());
    shuffle(cards.begin(), cards.end(), rng);
}

//draw from top of deck (back of vector)
Card Deck::drawCard() {
    if (cards.empty()) throw runtime_error("Deck is empty!");
    Card top = cards.back();
    cards.pop_back();
    return top;
}

//view top card without removing it
Card Deck::peekCard() const {
    if (cards.empty()) throw runtime_error("Deck is empty!");
    return cards.back();
}

bool Deck::isEmpty() const { return cards.empty(); } //return value of whether deck is empty
int  Deck::size()    const { return (int)cards.size(); } //return number of cards remaining in deck

//debug function to print contents of deck
void Deck::printDeck() {
    this_thread::sleep_for(chrono::milliseconds(500));
    cout << "[Deck] " << cards.size() << " cards remaining:\n";
    for (int i = (int)cards.size() - 1; i >= 0; i--) {
        this_thread::sleep_for(chrono::milliseconds(500));
        cout << "  " << cards[i] << "\n";
    }
}