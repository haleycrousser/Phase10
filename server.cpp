#include "csapp/csapp.h"
#include "gameLoop/game.h"

#include <cstring>

using namespace std;
 
int main() {
 
    ////////// CREATE SERVER //////////
    SOCKET serverfd = createServer("8080");
 
    ////////// WAIT FOR CLIENT //////////
    SOCKET clientfd = listenForClient(serverfd);
 
    ////////// GAME STUFF //////////
//--- WOULD YOU LIKE TO START THE GAME ? -------------------------------
    char buf[256] = {};
    const char *question = "Are you ready to start the game? (y/n): ";

    while (true) {
        // Ask the client
        if (send(clientfd, question, (int)strlen(question), 0) == SOCKET_ERROR) {
            NET_ERR("send failed");
            closeServer(clientfd, serverfd);
            return 1;
        }

        memset(buf, 0, sizeof(buf));
        int n = recv(clientfd, buf, sizeof(buf) - 1, 0);
        buf[n] = '\0';

        if (strcmp(buf, "y") == 0) {
            const char *msg = "Starting the game!";
            send(clientfd, msg, (int)strlen(msg), 0);
            break;
        } else {
            const char *msg = "Okay, waiting...";
            send(clientfd, msg, (int)strlen(msg), 0);
        }
    }

////////// GAME STARTS HERE //////////
    Game game;

    const char *question2 = "How many players would you like to play against? (2-6): ";
    if (send(clientfd, question2, (int)strlen(question2), 0) == SOCKET_ERROR) NET_ERR("send failed");
    memset(buf, 0, sizeof(buf)); int n = recv(clientfd, buf, sizeof(buf) - 1, 0); buf[n] = '\0';
    game.totalPlayers = atoi(buf);

    game.game_start();


    ////////// CLEAN UP //////////
    closeServer(clientfd, serverfd);
    return 0;
}
 


/*
    
    Would you like to start the game?

    User: Yes - game starts // No - Program exits

    You (Player 1) are the dealer! Press 1 to deal cards:

    User: 1 // other: Invalid...

    Here is your hand:
    (Yellow1, Blue1, Red1, WILD, SKIP, Yellow2, Green2, Red4, Red6, Yellow11)

----------------------------------------------------------------------

    Player 2 Turn:
    DISCARD: EMPTY & DECK: Red11a

    Player 2 Picks up Red11a.
    Player 2 does not meet the qualifications for phase 1.
    Player 2 Lays down Yellow4.

----------------------------------------------------------------------

    Player 3 Turn:
    DISCARD: Yellow4 & DECK: Green2

    Player 2 Picks up Green2.
    Player 2 does not meet the qualifications for phase 1.
    Player 2 Lays down Blue3.

----------------------------------------------------------------------

    Player 4 Turn:
    DISCARD: Blue3 & DECK: Red11

    Player 2 Picks up Red11.
    Player 2 does not meet the qualifications for phase 1.
    Player 2 Lays down Red4.

----------------------------------------------------------------------

    YOUR TURN
    Your Hand:
    (Yellow1, Blue1, Red1, WILD, SKIP, Yellow2, Green2, Red4, Red6, Yellow11)

    DISCARD: Red4 & DECK: WILD

    Which card would you like to pick up? Red4 or WILD

    User: WILD

    You Pick up WILD
    You meet the qualifications for phase 1! Lay down your cards?

    User: Yes

    You have:
    (Yellow1, Blue1, Red1, WILD, SKIP, Yellow2, Green2, Red4, Red6, Yellow11, WILD)
    One set of three 1: Yellow1, Blue1, Red1.
    One set of three 2: Yellow2, Green2, WILD.
    One set of three 2: Red4, WILD, WILD.
    One set of three 2: Red6, WILD, WILD.
    One set of three 2: Yellow11, WILD, WILD.

    Type the numbers you'd like to lay down.

    USER: 1 2

    YOU (Player 1) placed down phase1:

    States of Deck:
    DISCARD: Red4 & DECK: Yellow10

    Player 1 Cards:
   1: (Yellow1, Blue1, Red1.) & 2: (Yellow2, Green2, WILD.)

   Before you discard, you have cards you can place down.
   CARDS: WILD
   Place down?

   User: Yes

   Where?

   User: 2

   Which card do you choose to discard?
   ( SKIP, Red4, Red6, Yellow11)

   User: SKIP

----------------------------------------------------------------------

    Player 2's turn was skipped.

----------------------------------------------------------------------

    Player 3's Turn:
    DISCARD: Blue3 & DECK: Red11
    PLAYER 1'S CARDS: 1: (Yellow1, Blue1, Red1) & 2: (Yellow2, Green2, WILD, WILD)



    
    */