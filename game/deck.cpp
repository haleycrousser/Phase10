#include "deck.h"
#include <iostream>
#include <algorithm>  // std::shuffle
#include <random>     // std::mt19937, std::random_device
#include <stdexcept>

using namespace std;

// ── Constructor ────────────────────────────────────────────────────────────
Deck::Deck() {
    buildDeck();
    shuffleDeck();
}

// ── buildDeck ──────────────────────────────────────────────────────────────
// Standard Phase 10 deck = 108 cards:
//   96 number cards  — 2 copies of 1-12 in each of 4 colours
//                      variant 'a' = first copy, 'b' = second copy
//    8 Wild cards    — 25 pts each
//    4 Skip cards    — 15 pts each
void Deck::buildDeck() {
    cards.clear();
    cards.reserve(108);

    // Point value by number
    //   1-9  →  5 pts
    //  10-12 → 10 pts
    auto pointsFor = [](int num) -> int {
        if (num <= 9) return 5;
        return 10;
    };

    const string colors[] = { "Red", "Yellow", "Blue", "Green" };
    const char   variants[] = { 'a', 'b' };  // two copies of each card

    // 96 number cards
    for (const string& color : colors) {
        for (int num = 1; num <= 12; num++) {
            for (char variant : variants) {
                cards.push_back(Card(color, num, variant, pointsFor(num), false, false));
            }
        }
    }

    // 8 Wild cards (no colour, no number — use 0 and '-' as placeholders)
    for (int i = 0; i < 8; i++) {
        char variant = (i < 4) ? 'a' : 'b';
        cards.push_back(Card("Wild", 0, variant, 25, true, false));
    }

    // 4 Skip cards
    for (int i = 0; i < 4; i++) {
        char variant = (i < 2) ? 'a' : 'b';
        cards.push_back(Card("Skip", 0, variant, 15, false, true));
    }
}

// ── shuffleDeck ────────────────────────────────────────────────────────────
// Fisher-Yates shuffle via std::shuffle with a Mersenne Twister seeded from
// the hardware random device — much better than rand().
void Deck::shuffleDeck() {
    mt19937 rng(random_device{}());
    shuffle(cards.begin(), cards.end(), rng);
}

// ── drawCard ───────────────────────────────────────────────────────────────
// Removes and returns the top card (back of vector).
Card Deck::drawCard() {
    if (cards.empty()) {
        throw runtime_error("Deck is empty!");
    }
    Card top = cards.back();
    cards.pop_back();
    return top;
}

bool Deck::isEmpty() const { return cards.empty(); }
int  Deck::size()    const { return (int)cards.size(); }

// ── printDeck ──────────────────────────────────────────────────────────────
void Deck::printDeck() {
    cout << "[Deck] " << cards.size() << " cards remaining:\n";
    for (int i = (int)cards.size() - 1; i >= 0; i--) {
        cout << "  " << cards[i] << "\n";
    }
}