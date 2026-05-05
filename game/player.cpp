#include "player.h"
#include <iostream>
#include <stdexcept>
#include <thread>
#include <chrono>

using namespace std;

Player::Player()
    : player_id(0), name("Unknown"), currentPhase(1), score(0), completedPhase(false), skipped(false) //player constructor with default values
{}

Player::Player(int id, string name)
    : player_id(id), name(name), currentPhase(1), score(0), completedPhase(false), skipped(false) //set player values with given id and name
{}

void Player::addCard(Card c) { //add a card to hand
    hand.push_back(c);
}

void Player::removeCard(int index) { //remove card from hand by index
    if (index < 0 || index >= (int)hand.size()) {
        throw out_of_range("removeCard: index " + to_string(index) +
                           " out of range (hand size " +
                           to_string(hand.size()) + ")");
    }
    hand.erase(hand.begin() + index);
}

int Player::handScore() const { //tally up score of cards still in hand
    int total = 0;
    for (const Card& c : hand) total += c.points;
    return total;
}

void Player::printHand() const { //just a print function for debugging
    this_thread::sleep_for(chrono::milliseconds(500));
    cout << "[Player " << player_id + 1 << " | " << name
         << " | Phase " << currentPhase
         << " | Score " << score << "]\n";
    if (hand.empty()) {
        this_thread::sleep_for(chrono::milliseconds(500));
        cout << "  (empty hand)\n";
        return;
    }
    for (int i = 0; i < (int)hand.size(); i++) {
        this_thread::sleep_for(chrono::milliseconds(500));
        cout << "  [" << i << "] " << hand[i] << "\n";
    }
}