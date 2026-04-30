#include "player.h"
#include <iostream>
#include <stdexcept>

using namespace std;

Player::Player()
    : player_id(0), name("Unknown"), currentPhase(1), score(0), completedPhase(false)
{}

Player::Player(int id, string name)
    : player_id(id), name(name), currentPhase(1), score(0), completedPhase(false)
{}

void Player::addCard(Card c) {
    hand.push_back(c);
}

void Player::removeCard(int index) {
    if (index < 0 || index >= (int)hand.size()) {
        throw out_of_range("removeCard: index " + to_string(index) +
                           " out of range (hand size " +
                           to_string(hand.size()) + ")");
    }
    hand.erase(hand.begin() + index);
}

int Player::handScore() const {
    int total = 0;
    for (const Card& c : hand) total += c.points;
    return total;
}

void Player::printHand() const {
    cout << "[Player " << player_id + 1 << " | " << name
         << " | Phase " << currentPhase
         << " | Score " << score << "]\n";
    if (hand.empty()) { cout << "  (empty hand)\n"; return; }
    for (int i = 0; i < (int)hand.size(); i++) {
        cout << "  [" << i << "] " << hand[i] << "\n";
    }
}