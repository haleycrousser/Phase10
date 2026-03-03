#ifndef CARD_H  
#define CARD_H

#include <iostream>
#include <string>

class Card {

    public:
    std::string color;  
    int number;   
    char variant;
    int points;

    Card(std::string c = "", int n = 0, char v = 'a', int p = 0)
        : color(c), number(n), variant(v), points(p) {}

};


#endif