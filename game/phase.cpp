#include "phase.h"
#include <algorithm>
#include <numeric>

using namespace std;

// ── isSet ──────────────────────────────────────────────────────────────────
// A valid set: all non-wild cards share the same number.
// Wilds count as any number. Skip cards are never valid in a set.
bool Phase::isSet(vector<Card> cards, int size) {
    if ((int)cards.size() != size) return false;

    int setNum = -1;
    for (const Card& c : cards) {
        if (c.wild) continue;
        if (c.skip) return false;
        if (setNum == -1) setNum = c.num;
        else if (c.num != setNum) return false;
    }
    return true;
}

// ── isRun ──────────────────────────────────────────────────────────────────
// A valid run: non-wild cards are all different and consecutive when wilds
// fill the gaps. Skip cards are never valid in a run.
//
// Algorithm:
//   1. Collect non-wild numbers, count wilds.
//   2. No duplicate numbers allowed.
//   3. gaps  = holes between the min and max numbers.
//   4. Valid if gaps <= wilds AND the total span fits within `size`.
bool Phase::isRun(vector<Card> cards, int size) {
    if ((int)cards.size() != size) return false;

    vector<int> nums;
    int wilds = 0;

    for (const Card& c : cards) {
        if (c.wild)  { wilds++; continue; }
        if (c.skip)  return false;
        nums.push_back(c.num);
    }

    if (nums.empty()) return true; // all wilds — trivially a valid run

    sort(nums.begin(), nums.end());

    // Duplicate numbers break a run even with wilds
    for (int i = 1; i < (int)nums.size(); i++) {
        if (nums[i] == nums[i - 1]) return false;
    }

    int range = nums.back() - nums.front() + 1; // span of the sequence
    int gaps  = range - (int)nums.size();        // holes to fill with wilds

    return gaps <= wilds && range <= size;
}

// ── isColor ────────────────────────────────────────────────────────────────
// A valid color group: all non-wild cards share the same color.
// Wilds count as any color. Skip cards are invalid.
bool Phase::isColor(vector<Card> cards, int size) {
    if ((int)cards.size() != size) return false;

    string targetColor;
    for (const Card& c : cards) {
        if (c.wild) continue;
        if (c.skip) return false;
        if (targetColor.empty()) targetColor = c.color;
        else if (c.color != targetColor) return false;
    }
    return true;
}

// ── tryTwoComponents ───────────────────────────────────────────────────────
// Tries every way to split `cards` into a group of size1 and a group of
// size2. Returns true as soon as any split satisfies both checkers.
//
// Uses a boolean selector array + prev_permutation to iterate combinations.
// C(9,4) = 126 at most — fast enough for card-sized hands.
bool Phase::tryTwoComponents(
    vector<Card> cards,
    int size1, int size2,
    function<bool(vector<Card>)> check1,
    function<bool(vector<Card>)> check2)
{
    int n = (int)cards.size();
    if (n != size1 + size2) return false;

    // selector: size1 trues followed by size2 falses
    // prev_permutation walks every unique ordering → every combination
    vector<bool> selector(n, false);
    fill(selector.begin(), selector.begin() + size1, true);
    sort(selector.begin(), selector.end(), greater<bool>());

    do {
        vector<Card> group1, group2;
        for (int i = 0; i < n; i++) {
            if (selector[i]) group1.push_back(cards[i]);
            else             group2.push_back(cards[i]);
        }
        if (check1(group1) && check2(group2)) return true;
    } while (prev_permutation(selector.begin(), selector.end()));

    return false;
}

// ── Phase checkers ─────────────────────────────────────────────────────────

// Phase 1 — 2 sets of 3  (6 cards)
bool Phase::phaseOne(const vector<Card>& c) {
    if ((int)c.size() != 6) return false;
    return tryTwoComponents(c, 3, 3,
        [](vector<Card> g) { return Phase::isSet(g, 3); },
        [](vector<Card> g) { return Phase::isSet(g, 3); });
}

// Phase 2 — set of 3 + run of 4  (7 cards)
bool Phase::phaseTwo(const vector<Card>& c) {
    if ((int)c.size() != 7) return false;
    return tryTwoComponents(c, 3, 4,
        [](vector<Card> g) { return Phase::isSet(g, 3); },
        [](vector<Card> g) { return Phase::isRun(g, 4); })
        ||
        tryTwoComponents(c, 4, 3,
        [](vector<Card> g) { return Phase::isRun(g, 4); },
        [](vector<Card> g) { return Phase::isSet(g, 3); });
}

// Phase 3 — set of 4 + run of 4  (8 cards)
bool Phase::phaseThree(const vector<Card>& c) {
    if ((int)c.size() != 8) return false;
    return tryTwoComponents(c, 4, 4,
        [](vector<Card> g) { return Phase::isSet(g, 4); },
        [](vector<Card> g) { return Phase::isRun(g, 4); })
        ||
        tryTwoComponents(c, 4, 4,
        [](vector<Card> g) { return Phase::isRun(g, 4); },
        [](vector<Card> g) { return Phase::isSet(g, 4); });
}

// Phase 4 — run of 7  (7 cards)
bool Phase::phaseFour(const vector<Card>& c) {
    if ((int)c.size() != 7) return false;
    return isRun(c, 7);
}

// Phase 5 — run of 8  (8 cards)
bool Phase::phaseFive(const vector<Card>& c) {
    if ((int)c.size() != 8) return false;
    return isRun(c, 8);
}

// Phase 6 — run of 9  (9 cards)
bool Phase::phaseSix(const vector<Card>& c) {
    if ((int)c.size() != 9) return false;
    return isRun(c, 9);
}

// Phase 7 — 2 sets of 4  (8 cards)
bool Phase::phaseSeven(const vector<Card>& c) {
    if ((int)c.size() != 8) return false;
    return tryTwoComponents(c, 4, 4,
        [](vector<Card> g) { return Phase::isSet(g, 4); },
        [](vector<Card> g) { return Phase::isSet(g, 4); });
}

// Phase 8 — 7 of one color  (7 cards)
bool Phase::phaseEight(const vector<Card>& c) {
    if ((int)c.size() != 7) return false;
    return isColor(c, 7);
}

// Phase 9 — set of 5 + set of 2  (7 cards)
bool Phase::phaseNine(const vector<Card>& c) {
    if ((int)c.size() != 7) return false;
    return tryTwoComponents(c, 5, 2,
        [](vector<Card> g) { return Phase::isSet(g, 5); },
        [](vector<Card> g) { return Phase::isSet(g, 2); })
        ||
        tryTwoComponents(c, 2, 5,
        [](vector<Card> g) { return Phase::isSet(g, 2); },
        [](vector<Card> g) { return Phase::isSet(g, 5); });
}

// Phase 10 — set of 5 + set of 3  (8 cards)
bool Phase::phaseTen(const vector<Card>& c) {
    if ((int)c.size() != 8) return false;
    return tryTwoComponents(c, 5, 3,
        [](vector<Card> g) { return Phase::isSet(g, 5); },
        [](vector<Card> g) { return Phase::isSet(g, 3); })
        ||
        tryTwoComponents(c, 3, 5,
        [](vector<Card> g) { return Phase::isSet(g, 3); },
        [](vector<Card> g) { return Phase::isSet(g, 5); });
}

// ── checkPhase ─────────────────────────────────────────────────────────────
// Main entry point called by Game.
bool Phase::checkPhase(int phaseNum, const vector<Card>& cards) {
    switch (phaseNum) {
        case 1:  return phaseOne  (cards);
        case 2:  return phaseTwo  (cards);
        case 3:  return phaseThree(cards);
        case 4:  return phaseFour (cards);
        case 5:  return phaseFive (cards);
        case 6:  return phaseSix  (cards);
        case 7:  return phaseSeven(cards);
        case 8:  return phaseEight(cards);
        case 9:  return phaseNine (cards);
        case 10: return phaseTen  (cards);
        default: return false;
    }
}