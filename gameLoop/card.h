#ifndef CARD_H
#define CARD_H

#include <iostream>

class Card {
private:

    std::string color;
    int num;
    char variant;
    int points;

public:

    Card();
    Card(std::string color, int num, char variant, int points);
    friend std::ostream& operator<<(std::ostream& os, const Card& card);

};

#endif