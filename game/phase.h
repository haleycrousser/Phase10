#ifndef PHASE_H
#define PHASE_H

#include "card.h"
#include <vector>
#include <functional>

class Phase {
public:

    //returns true if those cards satisfy the phase requirement.
    static bool checkPhase(int phaseNum, const std::vector<Card>& cards);


    static bool isSet  (std::vector<Card> cards, int size); // same number
    static bool isRun  (std::vector<Card> cards, int size); // consecutive numbers
    static bool isColor(std::vector<Card> cards, int size); // same color

private:
    // Tries every way to split cards into two groups of size1 / size2,
    // returns true if any split satisfies both checkers.
    static bool tryTwoComponents(
        std::vector<Card> cards,
        int size1, int size2,
        std::function<bool(std::vector<Card>)> check1,
        std::function<bool(std::vector<Card>)> check2
    );

    // one function per phase
    static bool phaseOne  (const std::vector<Card>& c); // 2 sets of 3
    static bool phaseTwo  (const std::vector<Card>& c); // set of 3 + run of 4
    static bool phaseThree(const std::vector<Card>& c); // set of 4 + run of 4
    static bool phaseFour (const std::vector<Card>& c); // run of 7
    static bool phaseFive (const std::vector<Card>& c); // run of 8
    static bool phaseSix  (const std::vector<Card>& c); // run of 9
    static bool phaseSeven(const std::vector<Card>& c); // 2 sets of 4
    static bool phaseEight(const std::vector<Card>& c); // 7 of one color 
    static bool phaseNine (const std::vector<Card>& c); // set of 5 + set of 2
    static bool phaseTen  (const std::vector<Card>& c); // set of 5 + set of 3
};

#endif