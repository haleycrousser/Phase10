#include "card.h"
#include <iostream>

using namespace std;

//Stores the values for THIS one card
Card::Card(string color, int num, char variant, int points, bool wild, bool skip) {
    this->color   = color;
    this->num     = num;
    this->variant = variant;
    this->points  = points;
    this->wild    = wild;
    this->skip    = skip;
}

Card Card::createCard(string color, int num, char variant, int points) {

    string validColors[] = {"Red", "Yellow", "Blue", "Green"};
    int    validNums[]   = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    char   validVariants[] = {'a', 'b'};
    int    validPoints[] = {5, 10, 15, 25};

    bool wild = (color == "Wild");
    bool skip = (color == "Skip");

    return Card(color, num, variant, points, wild, skip);
}