#ifndef CARD_H  
#define CARD_H

#include <iostream>
#include <string>

class Card {

    std::string color;  
    int number;   
    char variant;
    int points;

    public:

    Card(std::string c = "", int n = 0, char v = 'a', int p = 0) //constructor
        : color(c), number(n), variant(v), points(p) {}

    friend std::ostream& operator<<(std::ostream& os, const Card& card); //print card object

};


#endif