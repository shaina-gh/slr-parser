#include <bits/stdc++.h>
using namespace std;

vector<pair<char, string>> productions, sets[20];
vector<char> nonTerminals, terminals, first[10], follow[10], epsilons;
map<char, int> symbolMap, nonTermMap;
int parseTable[20][20], numStates, symbolCount;
vector<int> parseEntries[20][20];
queue<int> stateQueue;
int visited[10];

// Check if character is a non-terminal
bool isNonTerminal(char c) {
    return c >= 'A' && c <= 'Z';
}

// Make closure set
void make_set(pair<char, string> item) {
    queue<pair<char, string>> q;
    if (find(sets[numStates].begin(), sets[numStates].end(), item) == sets[numStates].end()) {
        sets[numStates].push_back(item);
        q.push(item);
    }

    while (!q.empty()) {
        auto x = q.front(); q.pop();
        int pos = x.second.find('.');
        if (pos + 1 >= x.second.size()) continue;

        for (auto &prod : productions) {
            if (prod.first == x.second[pos + 1]) {
                string rhs = "." + prod.second;
                pair<char, string> newItem = {prod.first, rhs};
                if (find(sets[numStates].begin(), sets[numStates].end(), newItem) == sets[numStates].end()) {
                    sets[numStates].push_back(newItem);
                    q.push(newItem);
                }
            }
        }
    }
}

// Check for duplicate set
int check_duplicate() {
    for (int i = 0; i < numStates; ++i)
        if (sets[i] == sets[numStates]) return i;
    return -1;
}

// GOTO function
void goto_function(char symbol, int stateIndex) {
    for (auto &item : sets[stateIndex]) {
        int pos = item.second.find('.');
        if (pos + 1 < item.second.size() && item.second[pos + 1] == symbol) {
            string newRHS = item.second;
            swap(newRHS[pos], newRHS[pos + 1]);
            make_set({item.first, newRHS});
        }
    }

    if (sets[numStates].empty()) return;

    int sameSet = check_duplicate();
    if (sameSet == -1) {
        parseTable[stateIndex][symbolMap[symbol]] = numStates;
        parseEntries[stateIndex][symbolMap[symbol]].push_back(numStates);
        stateQueue.push(numStates++);
    } else {
        parseTable[stateIndex][symbolMap[symbol]] = sameSet;
        parseEntries[stateIndex][symbolMap[symbol]].push_back(sameSet);
        sets[numStates].clear();
    }
}

// Mark epsilon productions
void mark_epsilon() {
    for (auto &prod : productions)
        if (prod.second.empty()) epsilons.push_back(prod.first);
}

// First set utility
void compute_first_util(char lhs, vector<char> &result) {
    if (visited[nonTermMap[lhs]]) return;
    visited[nonTermMap[lhs]] = 1;

    for (auto &prod : productions) {
        if (prod.first == lhs && !prod.second.empty()) {
            int i = 0;
            while (i < prod.second.size()) {
                char c = prod.second[i];
                if (!isNonTerminal(c)) {
                    if (!count(result.begin(), result.end(), c)) result.push_back(c);
                    break;
                } else {
                    compute_first_util(c, result);
                    if (!count(epsilons.begin(), epsilons.end(), c)) break;
                    ++i;
                }
            }
            if (i == prod.second.size()) epsilons.push_back(lhs);
        }
    }
}

// Find FIRST sets
void compute_first() {
    for (auto &nt : nonTerminals) {
        memset(visited, 0, sizeof(visited));
        compute_first_util(nt, first[nonTermMap[nt]]);
    }
}

// Follow set utility
void compute_follow_util(char lhs, vector<char> &result) {
    if (visited[nonTermMap[lhs]]) return;
    visited[nonTermMap[lhs]] = 1;

    if (lhs == productions[0].first && !count(result.begin(), result.end(), '$')) {
        result.push_back('$');
    }

    for (auto &prod : productions) {
        for (int j = 0; j < prod.second.size(); ++j) {
            if (prod.second[j] == lhs) {
                int k = j + 1;
                while (k < prod.second.size()) {
                    char c = prod.second[k];
                    if (!isNonTerminal(c)) {
                        if (!count(result.begin(), result.end(), c)) result.push_back(c);
                        break;
                    } else {
                        for (auto &x : first[nonTermMap[c]])
                            if (!count(result.begin(), result.end(), x)) result.push_back(x);
                        if (!count(epsilons.begin(), epsilons.end(), c)) break;
                        ++k;
                    }
                }
                if (k == prod.second.size()) compute_follow_util(prod.first, result);
            }
        }
    }
}

// Compute FOLLOW sets
void compute_follow() {
    for (auto &nt : nonTerminals) {
        memset(visited, 0, sizeof(visited));
        compute_follow_util(nt, follow[nonTermMap[nt]]);
    }
}

// Find production number
int find_production(pair<char, string> item) {
    for (int i = 0; i < productions.size(); ++i)
        if (productions[i] == item) return i + 1;
    return -1;
}

// Print stack for debugging
void print_stack(stack<char> s, stack<int> states) {
    string symbols;
    vector<int> nums;
    while (!s.empty()) {
        symbols += s.top(); s.pop();
    }
    while (!states.empty()) {
        nums.push_back(states.top()); states.pop();
    }

    for (int i = symbols.size() - 1; i >= 0; --i) cout << nums[i + 1] << symbols[i];
    cout << nums[0];
}

// Parsing input string
void parse_string() {
    string input;
    cout << "\nEnter the string: ";
    cin >> input;

    stack<char> symbolStack;
    stack<int> stateStack;
    stateStack.push(0);
    int ptr = 0;

    while (ptr < input.size()) {
        cout << "$";
        print_stack(symbolStack, stateStack);
        cout << " ";
        for (int i = ptr; i < input.size(); ++i) cout << input[i];
        cout << "$\n";

        int x = stateStack.top();
        int y = symbolMap[input[ptr]];
        if (parseTable[x][y] > 0) {
            symbolStack.push(input[ptr++]);
            stateStack.push(parseTable[x][y]);
        } else if (parseTable[x][y] < 0) {
            int prodIdx = -parseTable[x][y] - 1;
            if (symbolStack.size() < productions[prodIdx].second.size()) {
                cout << "Error\n"; return;
            }
            for (int i = 0; i < productions[prodIdx].second.size(); ++i) {
                symbolStack.pop();
                stateStack.pop();
            }
            symbolStack.push(productions[prodIdx].first);
            stateStack.push(parseTable[stateStack.top()][symbolMap[symbolStack.top()]]);
        } else {
            cout << "Error\n"; return;
        }
    }

    cout << "\nAccepted\n";
}

int main() {
    int n;
    char lhs;
    string rhs;

    cout << "Enter the number of productions: ";
    cin >> n;
    for (int i = 0; i < n; ++i) {
        cout << "LHS: "; cin >> lhs;
        cout << "RHS: "; cin >> rhs;
        if (rhs == "%") rhs = ""; // epsilon
        productions.push_back({lhs, rhs});
    }

    symbolCount = 0;
    for (auto &prod : productions) {
        if (!count(nonTerminals.begin(), nonTerminals.end(), prod.first))
            nonTerminals.push_back(prod.first);

        for (auto c : prod.second) {
            if (!isNonTerminal(c) && !count(terminals.begin(), terminals.end(), c)) {
                terminals.push_back(c);
                symbolMap[c] = symbolCount++;
            }
        }
    }

    terminals.push_back('$');
    symbolMap['$'] = symbolCount++;

    for (int i = 0; i < nonTerminals.size(); ++i) {
        symbolMap[nonTerminals[i]] = symbolCount++;
        nonTermMap[nonTerminals[i]] = i;
    }

    mark_epsilon();
    compute_first();
    compute_follow();

    string startRule = "." + string(1, productions[0].first);
    make_set({'X', startRule});
    ++numStates;
    stateQueue.push(0);

    while (!stateQueue.empty()) {
        int idx = stateQueue.front(); stateQueue.pop();
        for (char c : nonTerminals) goto_function(c, idx);
        for (char c : terminals) goto_function(c, idx);
    }

    for (int i = 0; i < numStates; ++i) {
        for (auto &item : sets[i]) {
            if (item.second.back() == '.') {
                string rhs = item.second.substr(0, item.second.size() - 1);
                int prodNum = find_production({item.first, rhs});
                if (prodNum < 0) continue;
                prodNum = -prodNum;
                for (char f : follow[nonTermMap[item.first]]) {
                    parseTable[i][symbolMap[f]] = prodNum;
                    parseEntries[i][symbolMap[f]].push_back(prodNum);
                }
            }
        }
    }

    parse_string();
    return 0;
}
