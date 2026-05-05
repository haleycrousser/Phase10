#include "game.h"
#include "phase.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <map>
#include <thread>
#include <chrono>

using namespace std;

Game::Game(int n, vector<socket_t>& sockets) //constructor, sets up players and sockets for the game
    : num_players(n), socks(sockets), game_over(false), current_player(0)
{
    players.resize(n);
    rios.resize(n);
    for (int i = 0; i < n; i++) {
        players[i].player_id = i; //give each player their own id
        rio_readinitb(&rios[i], socks[i]); //set up robust input for each socket
    }
}

void Game::player_loop(int player_id, socket_t fd) { //player loop, only player 0 actually runs the game logic
    if (player_id == 0) { //first player is basically the host/game runner
        run();
    } else { //other players just wait until the game is over
        unique_lock<mutex> lock(turn_mutex);
        turn_cv.wait(lock, [this] { return game_over.load(); }); //keeps the thread alive until game ends
    }
}

void Game::run() { //running function for the game, loops through player turns until game_over
    broadcast("\n=== Dealing cards... ===\n");
    dealCards(); //start the first round by dealing everyone 10 cards

    while (!game_over) { //main game loop
        for (int i = 0; i < num_players && !game_over; i++) { //each player gets a turn
            current_player = i; //track whose turn it is

            broadcast("\n----------------------------------------\n");
            broadcast("Player " + to_string(i + 1) + "'s turn!\n");
            broadcast("----------------------------------------\n");
            pause();

            if (players[i].skipped) { //skip card makes this player lose their turn
                players[i].skipped = false; //clear skip after using it
                broadcast("Player " + to_string(i + 1) + " is skipped!\n");
                pause();
                continue;
            }

            for (int j = 0; j < num_players; j++) { //tell everyone else to wait during this player's turn
                if (j != i)
                    send_to(j, "Waiting for Player " + to_string(i + 1) + "...\n");
            }

            //draw
            drawPhase(i);
            sendHand(i); //show updated hand after drawing
            pause();

            //lay down phase if not already done
            if (!players[i].completedPhase) {
                send_to(i, "\nLay down your phase? (y/n):\n");
                string resp = recv_from(i);
                if (resp == "y" || resp == "Y") {
                    layDownPhase(i);
                    pause();
                }
            }

            //auto hit becuase having player do it manually is too slow and tedious, especially with multiple cards to hit
            if (players[i].completedPhase) {
                bool anyPhaseArea = false; //checks if there is anywhere to hit cards
                for (int j = 0; j < num_players; j++) { //look for at least one completed phase area
                    if (!players[j].phaseArea.empty()) { anyPhaseArea = true; break; }
                }
                if (anyPhaseArea) {
                    hitPhase(i);
                    pause();
                }
            }

            if (!players[i].hand.empty()) //discard
                discardCard(i);

            //check if player has gone out, then tally scores and start new round if so
            if (players[i].hand.empty()) {
                pause();
                broadcast("\n*** Player " + to_string(i + 1) + " has gone out! ***\n");
                pause();

                tallyScores();
                pause();

                bool gameWon = false; //used to see if someone passed phase 10
                for (int j = 0; j < num_players; j++) { //update phases after the round ends
                    if (players[j].completedPhase) { //only completed players move to the next phase
                        players[j].currentPhase++;
                        broadcast("Player " + to_string(j + 1) +
                                  " advances to Phase " +
                                  to_string(players[j].currentPhase) + "!\n");
                        if (players[j].currentPhase > 10) gameWon = true;
                    } else {
                        broadcast("Player " + to_string(j + 1) +
                                  " stays on Phase " +
                                  to_string(players[j].currentPhase) + ".\n");
                    }
                }

                if (gameWon) {
                    checkGameWon();
                    game_over = true;
                    break;
                }

                broadcast("\n=== Starting new round! ===\n");
                deck = Deck(); //reset deck for the new round
                discardPile.clear(); //clear old discards before the next round
                dealCards(); //deal everyone a new hand for the next round
            }
        }
    }

    turn_cv.notify_all(); //wake up waiting player threads when game is done
}

void Game::dealCards() { //deal cards and reset round-specific player data
    for (int i = 0; i < num_players; i++) {
        players[i].hand.clear(); //clear old hand from previous round
        players[i].phaseArea.clear(); //clear any laid down phase cards
        players[i].completedPhase = false; //player has to complete their phase again this round
        players[i].skipped = false; //remove skip status at start of round
    }

    for (int card = 0; card < 10; card++) { //deal 10 cards to every player
        for (int i = 0; i < num_players; i++) {
            if (!deck.isEmpty())
                players[i].hand.push_back(deck.drawCard()); //draw one card from the deck
        }
    }

    for (int i = 0; i < num_players; i++)
        sendHand(i); //show each player their new hand
}


void Game::sortHand(int player_id) { //sort hand because it was too dificult to read otherwise
    sort(players[player_id].hand.begin(), players[player_id].hand.end(),
         [](const Card& a, const Card& b) {
             if (a.skip != b.skip) return !a.skip;  // skips to end
             if (a.wild != b.wild) return !a.wild;  // wilds before skips
             return a.num < b.num; //then sort normal cards by number
         });
}


void Game::sendHand(int player_id) { //shows hand (sorts then sends)
    sortHand(player_id); //make the hand easier to read before sending it

    Player& p = players[player_id]; //shortcut to this player
    string msg = "\n=== Your Hand ===\n";

    if (p.hand.empty()) { //prevents blank hand output
        msg += "  (empty)\n";
    } else {
        for (int i = 0; i < (int)p.hand.size(); i++) {
            msg += "  [" + to_string(i + 1) + "] " + cardToString(p.hand[i]) + "\n"; //number cards so the player can choose them
        }
    }

    msg += "Phase " + to_string(p.currentPhase) + ": " +
           phaseDescription(p.currentPhase) + "\n";
    msg += "Score: " + to_string(p.score) + " pts\n";
    msg += "=================\n";

    send_to(player_id, msg);
}

static string formatCards(const vector<Card>& cards) { //formatting cards into one readable string
    string s;
    for (int i = 0; i < (int)cards.size(); i++) {
        if (cards[i].wild) s += "[WILD 25pts]"; //wild cards have special display
        else if (cards[i].skip) s += "[SKIP 15pts]"; //skip cards have special display
        else s += "[" + cards[i].color + " " + to_string(cards[i].num) +
                  " " + to_string(cards[i].points) + "pts]";
        if (i < (int)cards.size() - 1) s += "  ";
    }
    return s;
}


string Game::formatPhaseArea(const vector<Card>& area, int phaseNum) { //format phase area, struggled with this formatting because it was always cluttered
    vector<Card> wilds, nonwilds; //split wilds from normal cards so the output looks cleaner
    for (const Card& c : area) {
        if (c.wild) wilds.push_back(c); //save wilds for their own line
        else        nonwilds.push_back(c); //normal cards are used to make sets/runs
    }

    sort(nonwilds.begin(), nonwilds.end(), [](const Card& a, const Card& b) { //sort normal cards by number
        return a.num < b.num;
    });

    string out;

    auto groupByNum = [&]() -> vector<vector<Card>> { //helper that groups same-number cards together for sets
        vector<vector<Card>> groups;
        for (const Card& c : nonwilds) {
            bool added = false;
            for (auto& g : groups) {
                if (g[0].num == c.num) { g.push_back(c); added = true; break; } //add card to matching set
            }
            if (!added) groups.push_back({c}); //start a new number group
        }
        sort(groups.begin(), groups.end(), [](const vector<Card>& a, const vector<Card>& b) { //biggest sets first
            return a.size() > b.size();
        });
        return groups;
    };

    switch (phaseNum) {
        case 1:
        case 7: { //these phases are only sets 
            auto groups = groupByNum();
            for (auto& g : groups)
                out += "  Set of " + to_string(g.size()) + ": " + formatCards(g) + "\n";
            break;
        }

        case 2:
        case 3: { //these phases have both a set and a run 
            auto groups = groupByNum();


            vector<Card> setCards, runCards; //separate the set part from the run part
            bool setFound = false;
            for (auto& g : groups) {
                if (!setFound && g.size() >= 2) { //first big enough group is treated as the set
                    setCards = g; setFound = true;
                } else {
                    for (const Card& c : g) runCards.push_back(c); //everything else becomes the run display
                }
            }
            sort(runCards.begin(), runCards.end(), [](const Card& a, const Card& b) {
                return a.num < b.num; //then sort normal cards by number
            });
            out += "  Set of " + to_string(setCards.size()) + ": " + formatCards(setCards) + "\n";
            out += "  Run of " + to_string(runCards.size()) + ": " + formatCards(runCards) + "\n";
            break;
        }

        case 4: 
        case 5:  
        case 6: { //these phases are just runs 
            out += "  Run of " + to_string(nonwilds.size()) + ": " + formatCards(nonwilds) + "\n";
            break;
        }

        case 8: { //phase 8 is grouped by color
            out += "  Color group: " + formatCards(nonwilds) + "\n";
            break;
        }

        case 9: 
        case 10: { //these phases are also sets
            auto groups = groupByNum();
            for (auto& g : groups)
                out += "  Set of " + to_string(g.size()) + ": " + formatCards(g) + "\n";
            break;
        }

        default:
            out += "  " + formatCards(nonwilds) + "\n";
    }

    if (!wilds.empty()) //show wilds separately so they don't mess up the formatting
        out += "  Wilds: " + formatCards(wilds) + "\n";

    return out;
}

string Game::phaseAreasString() { //builds the message that shows everyone's laid down phases
    string out = "\n=== Phase Areas ===\n";
    bool any = false; //tracks if anyone has laid down cards yet

    for (int j = 0; j < num_players; j++) {
        if (players[j].phaseArea.empty()) continue; //skip players with no phase area
        any = true; //at least one phase area exists

        out += "Player " + to_string(j + 1) +
               "  [Phase " + to_string(players[j].currentPhase) + ": " +
               phaseDescription(players[j].currentPhase) + "]\n";

        out += formatPhaseArea(players[j].phaseArea, players[j].currentPhase);
    }

    if (!any) out += "  (no phase areas yet)\n";
    out += "===================\n";
    return out;
}

bool Game::cardFitsArea(const Card& card, const vector<Card>& area, int phaseNum) { //check if card can be hit on a phase area, 
                                                                                    //this was very difficult to implement because of all the 
                                                                                    //different phase types and rules about where cards can be hit
    if (area.empty()) return false; //can't hit on an empty phase area
    if (card.skip)    return false; //skip cards cannot be hit

    bool runPhases[]   = {false, false, false, false, true, true, true, false, false, false, false}; //marks phases that are runs
    bool colorPhases[] = {false, false, false, false, false, false, false, false, true, false, false}; //marks color phase

    if (card.wild) return true; //wild can fit basically anywhere except empty/skip cases above

    if (colorPhases[phaseNum]) { //phase 8 lets players hit cards of the same color
        string targetColor;
        for (const Card& c : area) if (!c.wild) { targetColor = c.color; break; }
        return targetColor.empty() || card.color == targetColor;
    }

    if (runPhases[phaseNum]) { //run phases can only extend the front or back of the run
        vector<int> nums;
        for (const Card& c : area) if (!c.wild) nums.push_back(c.num);
        if (nums.empty()) return true;
        int mn = *min_element(nums.begin(), nums.end());
        int mx = *max_element(nums.begin(), nums.end());
        return card.num == mn - 1 || card.num == mx + 1;
    }

    if (phaseNum == 2 || phaseNum == 3) { //mixed set/run phases need extra checking
        map<int, int> counts;
        for (const Card& c : area) if (!c.wild) counts[c.num]++;

        int setNum = -1, bestCount = 0; //find the number that most likely belongs to the set
        for (auto& [num, cnt] : counts)
            if (cnt > bestCount) { bestCount = cnt; setNum = num; }

        vector<int> runNums;
        for (auto& [num, cnt] : counts)
            if (num != setNum) runNums.push_back(num);
        sort(runNums.begin(), runNums.end());

        if (card.num == setNum) return true; //same number can be added to the set

        if (!runNums.empty()) {
            int mn = runNums.front();
            int mx = runNums.back();
            // make sure the number isn't already in the run
            bool alreadyInRun = find(runNums.begin(), runNums.end(), card.num) != runNums.end();
            if (!alreadyInRun && (card.num == mn - 1 || card.num == mx + 1))
                return true;
        }

        return false;
    }


    for (const Card& c : area)
        if (!c.wild && card.num == c.num) return true;
    return false;
}

void Game::hitPhase(int player_id) { //auto hit any cards the player can place on completed phases
    bool anyPlaced = true; //keeps looping while cards are still being placed
    bool firstMsg  = true; //only prints the auto-hit header once

    while (anyPlaced) {
        anyPlaced = false; //will turn true again if a card gets placed

        for (int cardIdx = 0; cardIdx < (int)players[player_id].hand.size(); ) {
            Card& c = players[player_id].hand[cardIdx];
            bool placed = false; //tracks if this card was hit somewhere

            for (int j = 0; j < num_players && !placed; j++) {
                if (players[j].phaseArea.empty()) continue; //cannot hit on someone without a phase down //skip players with no phase area

                if (cardFitsArea(c, players[j].phaseArea, players[j].currentPhase)) {
                    if (firstMsg) { //first time a card can be hit, show phase areas and header
                        send_to(player_id, phaseAreasString()); //show current phase areas before drawing
                        broadcast("\n--- Auto-hitting cards ---\n");
                        pause(1000);
                        firstMsg = false;
                    }

                    broadcast("Player " + to_string(player_id + 1) +
                              " hits on Player " + to_string(j + 1) +
                              "'s phase with " + cardToString(c) + "\n");
                    pause(800);

                    players[j].phaseArea.push_back(c); //add card to the target phase area
                    players[player_id].hand.erase( //remove card from current player's hand
                        players[player_id].hand.begin() + cardIdx);

                    anyPlaced = true;
                    placed    = true;
                }
            }

            if (!placed) cardIdx++; //only move forward if the card stayed in the hand
        }
    }

    if (!firstMsg) {
        send_to(player_id, phaseAreasString()); //show current phase areas before drawing
        sendHand(player_id);
    }
}

void Game::drawPhase(int player_id) { //handles drawing from the deck or discard pile
    send_to(player_id, phaseAreasString()); //show current phase areas before drawing

    string info = "--- Draw ---\n"; //message with deck/discard options
    info += "Deck (" + to_string(deck.size()) + " cards): top card is " +
            (deck.isEmpty() ? "(empty)" : cardToString(deck.peekCard())) + "\n";
    if (!discardPile.empty())
        info += "Discard pile top: " + cardToString(discardPile.back()) + "\n";
    else
        info += "Discard pile: (empty)\n";

    info += "Draw from [d]eck";
    if (!discardPile.empty()) info += " or discard [p]ile";
    info += ":\n";
    send_to(player_id, info);

    while (true) {
        string resp = recv_from(player_id); //read player choice

        if (resp == "d" || resp == "D") { //draw from deck
            if (deck.isEmpty()) { //backup if deck runs out
                send_to(player_id, "Deck is empty! Drawing from discard pile.\n");
                players[player_id].hand.push_back(discardPile.back());
                discardPile.pop_back();
            } else {
                players[player_id].hand.push_back(deck.drawCard());
            }
            send_to(player_id, "Card drawn!\n");
            break;

        } else if ((resp == "p" || resp == "P") && !discardPile.empty()) { //pick up discard pile top
            players[player_id].hand.push_back(discardPile.back());
            discardPile.pop_back();
            send_to(player_id, "Picked up from discard pile!\n");
            break;

        } else {
            send_to(player_id, "Enter 'd' for deck or 'p' for discard pile:\n");
        }
    }
}

void Game::layDownPhase(int player_id) { //lets a player choose cards and try to complete their phase
    Player& p      = players[player_id]; //shortcut to current player
    int phaseNum   = p.currentPhase; //phase the player is currently on
    int required[] = {0, 6, 7, 8, 7, 8, 9, 8, 7, 7, 8}; //how many cards each phase needs
    int need       = required[phaseNum]; //cards needed for this player's phase

    send_to(player_id, "\nYou need: " + phaseDescription(phaseNum) + "\n");
    sendHand(player_id);
    send_to(player_id, "Enter " + to_string(need) +
                       " card numbers (1-based, space-separated), or Enter to cancel:\n");

    while (true) {
        string input = recv_from(player_id); //read card number to discard
        if (input.empty()) { //blank input cancels laying down
            send_to(player_id, "Cancelled.\n");
            return;
        }

        vector<int> indices = parseIndices(input, (int)p.hand.size()); //convert typed numbers to hand indexes

        if ((int)indices.size() != need) { //must select exactly the number of cards required
            send_to(player_id, "Need exactly " + to_string(need) +
                               " cards. Try again (or Enter to cancel):\n");
            continue;
        }

        vector<int> sorted_idx = indices; //copy used for duplicate checking and erasing
        sort(sorted_idx.begin(), sorted_idx.end());
        if (unique(sorted_idx.begin(), sorted_idx.end()) != sorted_idx.end()) { //make sure player did not choose same card twice
            send_to(player_id, "Duplicate numbers. Try again:\n");
            continue;
        }

        vector<Card> selected; //actual cards chosen from the hand
        for (int idx : indices) selected.push_back(p.hand[idx]);

        if (!Phase::checkPhase(phaseNum, selected)) { //validate the selected cards against phase rules
            send_to(player_id, "That doesn't satisfy Phase " +
                               to_string(phaseNum) + ". Try again (or Enter to cancel):\n");
            continue;
        }

        sort(sorted_idx.begin(), sorted_idx.end(), greater<int>()); //erase from back so indexes do not shift
        for (int idx : sorted_idx)
            p.hand.erase(p.hand.begin() + idx);

        p.phaseArea      = selected; //store the completed phase on the table
        p.completedPhase = true; //mark this player as completed for this round

        send_to(player_id, "Phase laid down!\n");
        broadcast("Player " + to_string(player_id + 1) +
                  " laid down Phase " + to_string(phaseNum) + "!\n");
        broadcast(phaseAreasString());

        sendHand(player_id);
        return;
    }
}

void Game::discardCard(int player_id) { //lets the player discard one card at the end of their turn
    send_to(player_id, "Choose a card to discard (number 1-" +
                       to_string(players[player_id].hand.size()) + "):\n");

    while (true) {
        string input = recv_from(player_id); //read card number to discard
        try {
            int idx = stoi(input) - 1; //convert from 1-based player input to 0-based vector index 
            if (idx < 0 || idx >= (int)players[player_id].hand.size()) {
                send_to(player_id, "Invalid number. Try again:\n");
                continue;
            }
            Card discarded = players[player_id].hand[idx]; //save card so it can be shown after removing it
            discardPile.push_back(discarded); //put discarded card on top of discard pile
            players[player_id].hand.erase(players[player_id].hand.begin() + idx); //remove discarded card from hand

            broadcast("Player " + to_string(player_id + 1) +
                      " discarded: " + cardToString(discarded) + "\n");

            if (discarded.skip) { //skip card skips the next player
                int target = (player_id + 1) % num_players; //next player loops back to 0 at the end
                players[target].skipped = true;
                broadcast("Player " + to_string(target + 1) +
                          " will be skipped next turn!\n");
            }

            return;
        } catch (...) {
            send_to(player_id, "Enter a number:\n");
        }
    }
}

void Game::tallyScores() { //adds penalty points from leftover cards after someone goes out
    broadcast("\n=== Tallying Scores ===\n");
    for (int i = 0; i < num_players; i++) {
        int penalty      = players[i].handScore(); //points still left in this player's hand
        players[i].score += penalty; //lower score is better, so these are penalty points
        broadcast("Player " + to_string(i + 1) + ": +" + to_string(penalty) +
                  " pts  (total: " + to_string(players[i].score) + " pts)\n");
    }
}


bool Game::checkGameWon() { //checks who won after someone finishes phase 10
    vector<int> winners; //players who made it past phase 10
    for (int i = 0; i < num_players; i++)
        if (players[i].currentPhase > 10) winners.push_back(i); //possible winner
    if (winners.empty()) return false;

    int best = winners[0]; //start by assuming first winner has best score
    for (int w : winners)
        if (players[w].score < players[best].score) best = w; //tie breaker is lowest score

    broadcast("\n=============================\n");
    broadcast(" Player " + to_string(best + 1) + " WINS THE GAME!\n");
    broadcast(" Final score: " + to_string(players[best].score) + " pts\n");
    broadcast("=============================\n");
    return true;
}



void Game::broadcast(const string& msg) { //send the same message to every connected player
    for (socket_t s : socks)
        rio_writen(s, msg.c_str(), msg.size()); //write message to this socket
}

void Game::send_to(int player_id, const string& msg) { //send a private message to one player
    rio_writen(socks[player_id], msg.c_str(), msg.size());
}

void Game::pause(int ms) { //small pause so messages are easier to read and do not spam instantly
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

string Game::recv_from(int player_id) { //reads one line from a player socket
    char buf[256] = {}; //buffer for one message from the client
    rio_readlineb(&rios[player_id], buf, sizeof(buf)); //read until newline
    for (int i = 0; buf[i]; i++)
        if (buf[i] == '\r' || buf[i] == '\n') { buf[i] = '\0'; break; } //remove newline characters
    return string(buf);
}


string Game::cardToString(const Card& c) { //turns one card into readable text
    if (c.wild) return "[WILD 25pts]";
    if (c.skip) return "[SKIP 15pts]";
    return "[" + c.color + " " + to_string(c.num) + " " + to_string(c.points) + "pts]";
}

string Game::phaseDescription(int phaseNum) { //returns the rule text for each phase number
    switch (phaseNum) {
        case 1:  return "2 sets of 3";
        case 2:  return "1 set of 3 + 1 run of 4";
        case 3:  return "1 set of 4 + 1 run of 4";
        case 4:  return "1 run of 7";
        case 5:  return "1 run of 8";
        case 6:  return "1 run of 9";
        case 7:  return "2 sets of 4";
        case 8:  return "7 of one color";
        case 9:  return "1 set of 5 + 1 set of 2";
        case 10: return "1 set of 5 + 1 set of 3";
        default: return "???";
    }
}


vector<int> Game::parseIndices(const string& input, int handSize) { //converts player input into valid hand indexes
    vector<int> indices;
    istringstream ss(input); //lets us read numbers from a string like cin
    int idx;
    while (ss >> idx) {
        int zero = idx - 1; //player sees cards as 1-based, vector uses 0-based
        if (zero >= 0 && zero < handSize) //ignore anything outside the hand size
            indices.push_back(zero);
    }
    return indices;
}