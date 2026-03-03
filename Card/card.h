#ifndef CARD_H  
#define CARD_H

#include <iostream>
#include <string>

class Card {

    public:
    std::string color;  // single color
    int number;         // single number
    char variant;       // single variant

    // constructor
    Card(const std::string& c, int n, char v)
        : color(c), number(n), variant(v) {}

};


#endif