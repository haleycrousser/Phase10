#include "card.h"

using namespace std;

Card::Card(string color, int num, char variant, int points, bool wild, bool skip) {
    this->color   = color;
    this->num     = num;
    this->variant = variant;
    this->points  = points;
    this->wild    = wild;
    this->skip    = skip;
}

Card Card::createCard(string color, int num, char variant, int points) { //create card
    bool wild = (color == "Wild");
    bool skip = (color == "Skip");
    return Card(color, num, variant, points, wild, skip);
}

ostream& operator<<(ostream& os, const Card& card) { //print obj!
    os << "[" << card.color << " | " << card.num
       << " | " << card.variant << " | " << card.points << " pts]";
    return os;
}