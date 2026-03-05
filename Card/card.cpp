#include "card.h"

#include <iostream>
#include <string>

std::ostream& operator<<(std::ostream& os, const Card& card) {
    if (card.variant == 'w') { //print wilds
        os << "Wild (25 pts)";
    }
    else if (card.variant == 's') { //print skips
        os << " Skip (15 pts)";
    }
    else { //print color cards
        os << card.color << " " 
           << card.number << card.variant 
           << " (" << card.points << " pts)";
    }

    return os;
}