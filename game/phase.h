#ifndef PHASE_H
#define PHASE_H

#include "player.h"

class Phase {
	bool isPhase(Player currentPhase);  // determines whether player can place, switch case
    bool isPhaseOne; //2 sets of 3
    bool isPhaseTwo; //set of 3 & 1 run of 4
    bool isPhaseThree; //set of 4 & 1 run of 4
    bool isPhaseFour; //run of 7
    bool isPhaseFive; //run of 8
    bool isPhaseSix; //run of 9
    bool isPhaseSeven; //2 sets of 4
    bool isPhaseEight; //7 of one color
    bool isPhaseNine; //1 set of 5 & 1 set of 2
    bool isPhaseTen; //1 set of 5 & 1 set of 3
};

#endif