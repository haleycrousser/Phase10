#include "game.h"
#include "phase.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <stdexcept>

using namespace std;

// ── Constructor ────────────────────────────────────────────────────────────
Game::Game(int n, vector<socket_t>& sockets)
    : num_players(n), socks(sockets), game_over(false), current_player(0)
{
    players.resize(n);
    rios.resize(n);
    for (int i = 0; i < n; i++) {
        players[i].player_id = i;
        rio_readinitb(&rios[i], socks[i]);
    }
}

// ── player_loop ────────────────────────────────────────────────────────────
void Game::player_loop(int player_id, socket_t fd) {
    if (player_id == 0) {
        run();
    } else {
        unique_lock<mutex> lock(turn_mutex);
        turn_cv.wait(lock, [this] { return game_over.load(); });
    }
}

// ── run ────────────────────────────────────────────────────────────────────
void Game::run() {
    broadcast("\n=== Dealing cards... ===\n");
    dealCards();

    while (!game_over) {
        for (int i = 0; i < num_players && !game_over; i++) {
            current_player = i;

            broadcast("\n----------------------------------------\n");
            broadcast("Player " + to_string(i + 1) + "'s turn!\n");
            broadcast("----------------------------------------\n");

            // Tell waiting players to hang on
            for (int j = 0; j < num_players; j++) {
                if (j != i)
                    send_to(j, "Waiting for Player " + to_string(i + 1) + "...\n");
            }

            // 1. Draw
            drawPhase(i);
            sendHand(i);

            // 2. Lay down phase (if not already done)
            if (!players[i].completedPhase) {
                send_to(i, "\nLay down your phase? (y/n): ");
                string resp = recv_from(i);
                if (resp == "y" || resp == "Y") {
                    layDownPhase(i);
                }
            }

            // 3. Hit on existing phase areas (only if own phase is down)
            if (players[i].completedPhase) {
                // Check if anyone has a phase area to hit on
                bool anyPhaseArea = false;
                for (int j = 0; j < num_players; j++) {
                    if (!players[j].phaseArea.empty()) { anyPhaseArea = true; break; }
                }
                if (anyPhaseArea) {
                    send_to(i, "\nHit on a phase area? (y/n): ");
                    string resp = recv_from(i);
                    if (resp == "y" || resp == "Y") {
                        hitPhase(i);
                    }
                }
            }

            // 4. Must discard
            discardCard(i);

            // 5. Check if this player has gone out (empty hand)
            if (players[i].hand.empty()) {
                broadcast("\n*** Player " + to_string(i + 1) + " has gone out! ***\n");

                tallyScores();

                // Advance phases for players who completed theirs
                bool gameWon = false;
                for (int j = 0; j < num_players; j++) {
                    if (players[j].completedPhase) {
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
                    checkGameWon(); // prints winner
                    game_over = true;
                    break;
                }

                // Start new round — fresh deck, re-deal
                broadcast("\n=== Starting new round! ===\n");
                deck = Deck();
                discardPile.clear();
                dealCards();
            }
        }
    }

    turn_cv.notify_all();
}

// ── dealCards ──────────────────────────────────────────────────────────────
// Clears hands, deals 10 cards to each player round-robin, flips top card
// to start the discard pile, then sends each player their hand.
void Game::dealCards() {
    for (int i = 0; i < num_players; i++) {
        players[i].hand.clear();
        players[i].phaseArea.clear();
        players[i].completedPhase = false;
    }

    for (int card = 0; card < 10; card++) {
        for (int i = 0; i < num_players; i++) {
            if (!deck.isEmpty())
                players[i].hand.push_back(deck.drawCard());
        }
    }

    // Flip one card to start discard pile
    if (!deck.isEmpty())
        discardPile.push_back(deck.drawCard());

    for (int i = 0; i < num_players; i++)
        sendHand(i);
}

// ── sendHand ───────────────────────────────────────────────────────────────
// Formats and sends a player's hand only to that player's socket.
void Game::sendHand(int player_id) {
    const Player& p = players[player_id];
    string msg = "\n=== Your Hand ===\n";

    if (p.hand.empty()) {
        msg += "  (empty)\n";
    } else {
        for (int i = 0; i < (int)p.hand.size(); i++) {
            msg += "  [" + to_string(i) + "] " + cardToString(p.hand[i]) + "\n";
        }
    }

    msg += "Phase " + to_string(p.currentPhase) + ": " +
           phaseDescription(p.currentPhase) + "\n";
    msg += "Score: " + to_string(p.score) + " pts\n";
    msg += "=================\n";

    send_to(player_id, msg);
}

// ── drawPhase ──────────────────────────────────────────────────────────────
// Lets the active player draw from the deck or the top of the discard pile.
void Game::drawPhase(int player_id) {
    string msg = "\nDraw from [d]eck";
    if (!discardPile.empty())
        msg += " or discard [p]ile (top: " + cardToString(discardPile.back()) + ")";
    msg += "? ";
    send_to(player_id, msg);

    while (true) {
        string resp = recv_from(player_id);

        if (resp == "d" || resp == "D") {
            if (deck.isEmpty()) {
                send_to(player_id, "Deck is empty! Drawing from discard pile.\n");
                players[player_id].hand.push_back(discardPile.back());
                discardPile.pop_back();
            } else {
                players[player_id].hand.push_back(deck.drawCard());
            }
            send_to(player_id, "Card drawn!\n");
            break;

        } else if ((resp == "p" || resp == "P") && !discardPile.empty()) {
            players[player_id].hand.push_back(discardPile.back());
            discardPile.pop_back();
            send_to(player_id, "Picked up from discard pile!\n");
            break;

        } else {
            send_to(player_id, "Enter 'd' for deck or 'p' for discard pile: ");
        }
    }
}

// ── layDownPhase ───────────────────────────────────────────────────────────
// Player selects card indices from their hand to form their phase.
// Validates with Phase::checkPhase, then moves cards to phaseArea.
void Game::layDownPhase(int player_id) {
    Player& p      = players[player_id];
    int phaseNum   = p.currentPhase;
    int required[] = {0, 6, 7, 8, 7, 8, 9, 8, 7, 7, 8};
    int need       = required[phaseNum];

    send_to(player_id, "\nYou need: " + phaseDescription(phaseNum) + "\n");
    send_to(player_id, "Enter " + to_string(need) +
                       " card indices (space-separated), or Enter to cancel: ");

    while (true) {
        string input = recv_from(player_id);
        if (input.empty()) {
            send_to(player_id, "Cancelled.\n");
            return;
        }

        vector<int> indices = parseIndices(input, (int)p.hand.size());

        // Must pick exactly the right number
        if ((int)indices.size() != need) {
            send_to(player_id, "Need exactly " + to_string(need) +
                               " cards. Try again (or Enter to cancel): ");
            continue;
        }

        // No duplicate indices
        vector<int> sorted_idx = indices;
        sort(sorted_idx.begin(), sorted_idx.end());
        if (unique(sorted_idx.begin(), sorted_idx.end()) != sorted_idx.end()) {
            send_to(player_id, "Duplicate indices. Try again: ");
            continue;
        }

        // Build the selected card set
        vector<Card> selected;
        for (int idx : indices) selected.push_back(p.hand[idx]);

        // Validate against phase rules
        if (!Phase::checkPhase(phaseNum, selected)) {
            send_to(player_id, "That doesn't satisfy Phase " +
                               to_string(phaseNum) + ". Try again (or Enter to cancel): ");
            continue;
        }

        // Valid — remove from hand (highest index first to avoid shifting)
        sort(sorted_idx.begin(), sorted_idx.end(), greater<int>());
        for (int idx : sorted_idx)
            p.hand.erase(p.hand.begin() + idx);

        p.phaseArea      = selected;
        p.completedPhase = true;

        send_to(player_id, "Phase laid down!\n");
        broadcast("Player " + to_string(player_id + 1) +
                  " laid down Phase " + to_string(phaseNum) + "!\n");

        // Show updated hand
        sendHand(player_id);
        return;
    }
}

// ── hitPhase ───────────────────────────────────────────────────────────────
// Player picks one card from their hand and adds it to another player's
// phaseArea, provided it fits (matching set number or extending a run).
void Game::hitPhase(int player_id) {
    // Show all phase areas
    string areas = "\n=== Phase Areas ===\n";
    for (int j = 0; j < num_players; j++) {
        if (players[j].phaseArea.empty()) continue;
        areas += "Player " + to_string(j + 1) + " (Phase " +
                 to_string(players[j].currentPhase) + "): ";
        for (const Card& c : players[j].phaseArea)
            areas += cardToString(c) + " ";
        areas += "\n";
    }
    send_to(player_id, areas);

    send_to(player_id, "Which player's area to hit? (1-" +
                       to_string(num_players) + ", or 0 to cancel): ");
    string resp = recv_from(player_id);
    int target;
    try { target = stoi(resp) - 1; } catch (...) { return; }
    if (target < 0 || target >= num_players || players[target].phaseArea.empty()) {
        send_to(player_id, "Invalid. Hit cancelled.\n");
        return;
    }

    sendHand(player_id);
    send_to(player_id, "Which card to play? (index): ");
    resp = recv_from(player_id);
    int cardIdx;
    try { cardIdx = stoi(resp); } catch (...) { return; }
    if (cardIdx < 0 || cardIdx >= (int)players[player_id].hand.size()) {
        send_to(player_id, "Invalid index. Hit cancelled.\n");
        return;
    }

    Card& played = players[player_id].hand[cardIdx];

    // Validate: card must fit into the target's phase area.
    // Build a test group with the new card added and re-check.
    // We use a simple rule: if it's a set phase, the card must match the set
    // number (or be wild). If it's a run phase, it must extend at either end.
    Player& tgt     = players[target];
    int tgtPhase    = tgt.currentPhase;
    bool runPhases[] = {false, false, false, false, true, true, true, false, false, false, false};
    // phases 4,5,6 are pure runs; others are sets (simplified hit check)

    bool valid = false;
    if (played.wild) {
        valid = true; // wilds always hit
    } else if (runPhases[tgtPhase]) {
        // For runs: card must extend the min or max of existing run
        vector<int> nums;
        for (const Card& c : tgt.phaseArea) if (!c.wild) nums.push_back(c.num);
        if (!nums.empty()) {
            int mn = *min_element(nums.begin(), nums.end());
            int mx = *max_element(nums.begin(), nums.end());
            valid = (played.num == mn - 1 || played.num == mx + 1);
        }
    } else {
        // For sets: card must match the set number
        int setNum = -1;
        for (const Card& c : tgt.phaseArea) if (!c.wild) { setNum = c.num; break; }
        valid = (setNum == -1 || played.num == setNum);
    }

    if (!valid) {
        send_to(player_id, "That card doesn't fit. Hit cancelled.\n");
        return;
    }

    tgt.phaseArea.push_back(played);
    players[player_id].hand.erase(players[player_id].hand.begin() + cardIdx);

    broadcast("Player " + to_string(player_id + 1) + " hit on Player " +
              to_string(target + 1) + "'s phase with " + cardToString(played) + "!\n");
    sendHand(player_id);
}

// ── discardCard ────────────────────────────────────────────────────────────
// Active player must discard exactly one card at the end of their turn.
void Game::discardCard(int player_id) {
    sendHand(player_id);
    send_to(player_id, "Choose a card to discard (index): ");

    while (true) {
        string input = recv_from(player_id);
        try {
            int idx = stoi(input);
            if (idx < 0 || idx >= (int)players[player_id].hand.size()) {
                send_to(player_id, "Invalid index. Try again: ");
                continue;
            }
            Card discarded = players[player_id].hand[idx];
            discardPile.push_back(discarded);
            players[player_id].hand.erase(players[player_id].hand.begin() + idx);
            broadcast("Player " + to_string(player_id + 1) +
                      " discarded: " + cardToString(discarded) + "\n");
            return;
        } catch (...) {
            send_to(player_id, "Enter a number: ");
        }
    }
}

// ── tallyScores ────────────────────────────────────────────────────────────
// Adds the point value of each player's remaining hand to their score.
void Game::tallyScores() {
    broadcast("\n=== Tallying Scores ===\n");
    for (int i = 0; i < num_players; i++) {
        int penalty     = players[i].handScore();
        players[i].score += penalty;
        broadcast("Player " + to_string(i + 1) + ": +" + to_string(penalty) +
                  " pts  (total: " + to_string(players[i].score) + " pts)\n");
    }
}

// ── checkGameWon ───────────────────────────────────────────────────────────
// Returns true if any player has advanced past phase 10.
// If multiple players are tied, the one with the lowest score wins.
bool Game::checkGameWon() {
    vector<int> winners;
    for (int i = 0; i < num_players; i++) {
        if (players[i].currentPhase > 10)
            winners.push_back(i);
    }
    if (winners.empty()) return false;

    // Lowest score wins among tied winners
    int best = winners[0];
    for (int w : winners)
        if (players[w].score < players[best].score) best = w;

    broadcast("\n=============================\n");
    broadcast(" Player " + to_string(best + 1) + " WINS THE GAME!\n");
    broadcast(" Final score: " + to_string(players[best].score) + " pts\n");
    broadcast("=============================\n");
    return true;
}

// ── Network helpers ────────────────────────────────────────────────────────

void Game::broadcast(const string& msg) {
    for (socket_t s : socks)
        rio_writen(s, msg.c_str(), msg.size());
}

void Game::send_to(int player_id, const string& msg) {
    rio_writen(socks[player_id], msg.c_str(), msg.size());
}

string Game::recv_from(int player_id) {
    char buf[256] = {};
    rio_readlineb(&rios[player_id], buf, sizeof(buf));
    for (int i = 0; buf[i]; i++)
        if (buf[i] == '\r' || buf[i] == '\n') { buf[i] = '\0'; break; }
    return string(buf);
}

// ── Formatting helpers ─────────────────────────────────────────────────────

string Game::cardToString(const Card& c) {
    if (c.wild) return "[WILD 25pts]";
    if (c.skip) return "[SKIP 15pts]";
    return "[" + c.color + " " + to_string(c.num) + " " + to_string(c.points) + "pts]";
}

string Game::phaseDescription(int phaseNum) {
    switch (phaseNum) {
        case 1:  return "2 sets of 3 (6 cards)";
        case 2:  return "1 set of 3 + 1 run of 4 (7 cards)";
        case 3:  return "1 set of 4 + 1 run of 4 (8 cards)";
        case 4:  return "1 run of 7 (7 cards)";
        case 5:  return "1 run of 8 (8 cards)";
        case 6:  return "1 run of 9 (9 cards)";
        case 7:  return "2 sets of 4 (8 cards)";
        case 8:  return "7 of one color (7 cards)";
        case 9:  return "1 set of 5 + 1 set of 2 (7 cards)";
        case 10: return "1 set of 5 + 1 set of 3 (8 cards)";
        default: return "???";
    }
}

// Parses "0 2 5 3" into {0,2,5,3}, ignoring anything out of range.
vector<int> Game::parseIndices(const string& input, int handSize) {
    vector<int> indices;
    istringstream ss(input);
    int idx;
    while (ss >> idx)
        if (idx >= 0 && idx < handSize)
            indices.push_back(idx);
    return indices;
}