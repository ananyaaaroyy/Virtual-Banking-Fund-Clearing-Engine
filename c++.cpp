#include <iostream>
#include <unordered_map>
#include <stack>
#include <queue>
#include <vector>
#include <algorithm>
#include <climits>
using namespace std;

// ── (a) BANK LEDGER — Hash Map ──────────────────────────────
unordered_map<string, long long> ledger;

void addBank(string id, long long bal) {
    ledger[id] = bal;
    cout << "Bank " << id << " added | Balance: " << bal << "\n";
}

void getBalance(string id) {
    cout << "Balance of " << id << ": " << ledger[id] << "\n";
}

// ── (b) TRANSFER UNDO — Stack ───────────────────────────────
struct Transfer { string from, to; long long amount; };
stack<Transfer> history;

void transfer(string from, string to, long long amt) {
    if (ledger[from] < amt) { cout << "Transfer failed: low balance\n"; return; }
    ledger[from] -= amt;
    ledger[to]   += amt;
    history.push({from, to, amt});
    cout << "Transferred " << amt << " from " << from << " to " << to << "\n";
}

void undoTransfer() {
    if (history.empty()) { cout << "Nothing to undo\n"; return; }
    Transfer t = history.top(); history.pop();
    ledger[t.to]   -= t.amount;
    ledger[t.from] += t.amount;
    cout << "Undone: " << t.amount << " returned to " << t.from << "\n";
}

// ── (c) SETTLEMENT LINE — Queue ─────────────────────────────
struct Pending { string from, to; long long amount; };
queue<Pending> settleLine;

void enqueueTransfer(string from, string to, long long amt) {
    settleLine.push({from, to, amt});
    cout << "Queued: " << from << " -> " << to << " | " << amt << "\n";
}

void processAll() {
    cout << "Processing settlement queue...\n";
    while (!settleLine.empty()) {
        Pending p = settleLine.front(); settleLine.pop();
        if (ledger[p.from] >= p.amount) {
            ledger[p.from] -= p.amount;
            ledger[p.to]   += p.amount;
            cout << "  Settled: " << p.from << " -> " << p.to << " | " << p.amount << "\n";
        } else {
            cout << "  Failed: " << p.from << " insufficient funds\n";
        }
    }
}

// ── (d) ROUTING CHECK — Binary Search ───────────────────────
vector<string> routingCodes;

void addRoutingCode(string code) {
    auto pos = lower_bound(routingCodes.begin(), routingCodes.end(), code);
    routingCodes.insert(pos, code);
}

void verifyRouting(string code) {
    bool found = binary_search(routingCodes.begin(), routingCodes.end(), code);
    cout << "Routing " << code << ": " << (found ? "AUTHORIZED" : "NOT AUTHORIZED") << "\n";
}

// ── (e) TRANSFER SORTER — std::sort ─────────────────────────
struct Txn { string id, currency; int urgency; long long amount; };
vector<Txn> transactions;

void addTxn(string id, string cur, int urg, long long amt) {
    transactions.push_back({id, cur, urg, amt});
}

void sortByCurrency() {
    sort(transactions.begin(), transactions.end(),
         [](Txn& a, Txn& b){ return a.currency < b.currency; });
    cout << "Sorted by currency:\n";
    for (auto& t : transactions)
        cout << "  " << t.id << " | " << t.currency << " | " << t.amount << "\n";
}

void sortByUrgency() {
    sort(transactions.begin(), transactions.end(),
         [](Txn& a, Txn& b){ return a.urgency > b.urgency; });
    cout << "Sorted by urgency:\n";
    for (auto& t : transactions)
        cout << "  " << t.id << " | Urgency:" << t.urgency << " | " << t.amount << "\n";
}

// ── (f) GLOBAL BANK MAP — Graph ──────────────────────────────
unordered_map<string, vector<pair<string,int>>> graph;

void addLink(string a, string b, int fee) {
    graph[a].push_back({b, fee});
    graph[b].push_back({a, fee});
    cout << a << " <-> " << b << " | Fee: " << fee << " bps\n";
}

void showMap() {
    for (auto& [bank, conns] : graph) {
        cout << "  " << bank << " -> ";
        for (auto& [nb, fee] : conns) cout << nb << "(" << fee << ") ";
        cout << "\n";
    }
}

// ── (g) CHEAPEST TRANSFER — Dijkstra ────────────────────────
void cheapestRoute(string src, string dst) {
    unordered_map<string,int> dist;
    unordered_map<string,string> prev;
    for (auto& [b,_] : graph) dist[b] = INT_MAX;
    dist[src] = 0;

    priority_queue<pair<int,string>, vector<pair<int,string>>, greater<>> pq;
    pq.push({0, src});

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;
        for (auto& [v, w] : graph[u])
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                prev[v] = u;
                pq.push({dist[v], v});
            }
    }

    if (dist[dst] == INT_MAX) { cout << "No route found\n"; return; }

    vector<string> path;
    for (string cur = dst; cur != src; cur = prev[cur]) path.push_back(cur);
    path.push_back(src);
    reverse(path.begin(), path.end());

    cout << "Cheapest route " << src << " to " << dst << ": ";
    for (int i = 0; i < (int)path.size(); i++) {
        cout << path[i];
        if (i+1 < (int)path.size()) cout << " -> ";
    }
    cout << " | Total fee: " << dist[dst] << " bps\n";
}

// ── (h) RESERVE MANAGER — Min-Heap ──────────────────────────
struct Reserve {
    string id; long long amount;
    bool operator>(const Reserve& o) const { return amount > o.amount; }
};
priority_queue<Reserve, vector<Reserve>, greater<Reserve>> reserveHeap;

void addReserve(string id, long long amt) {
    reserveHeap.push({id, amt});
    cout << "Reserve: " << id << " | " << amt << "\n";
}

void checkReserves(long long minRequired) {
    auto tmp = reserveHeap;
    cout << "Checking reserves (min required: " << minRequired << "):\n";
    while (!tmp.empty()) {
        auto r = tmp.top(); tmp.pop();
        cout << "  " << r.id << ": " << r.amount;
        if (r.amount < minRequired) cout << " << BELOW MINIMUM - ALLOCATING FUNDS";
        cout << "\n";
    }
}

// ── MAIN ─────────────────────────────────────────────────────
int main() {
    cout << "===== CLEARBANK SETTLEMENT SYSTEM =====\n\n";

    // (a) Ledger
    cout << "-- (a) BANK LEDGER --\n";
    addBank("SBI",   50000000);
    addBank("HDFC",  30000000);
    addBank("ICICI", 25000000);
    addBank("AXIS",  15000000);

    // (b) Undo
    cout << "\n-- (b) TRANSFER UNDO --\n";
    transfer("SBI", "HDFC", 5000000);
    transfer("HDFC", "ICICI", 2000000);
    undoTransfer();
    getBalance("HDFC");

    // (c) Settlement
    cout << "\n-- (c) SETTLEMENT LINE --\n";
    enqueueTransfer("SBI",   "AXIS",  1000000);
    enqueueTransfer("HDFC",  "ICICI",  500000);
    enqueueTransfer("ICICI", "SBI",    750000);
    processAll();

    // (d) Routing
    cout << "\n-- (d) ROUTING CHECK --\n";
    addRoutingCode("HDFC0005678");
    addRoutingCode("SBIN0001234");
    addRoutingCode("ICIC0009012");
    verifyRouting("SBIN0001234");
    verifyRouting("FAKE0000000");

    // (e) Sorter
    cout << "\n-- (e) TRANSFER SORTER --\n";
    addTxn("T1", "USD", 2, 500000);
    addTxn("T2", "INR", 3, 120000);
    addTxn("T3", "EUR", 1, 850000);
    addTxn("T4", "GBP", 3, 200000);
    sortByCurrency();
    sortByUrgency();

    // (f) Bank Map
    cout << "\n-- (f) GLOBAL BANK MAP --\n";
    addLink("SBI",   "HDFC",  15);
    addLink("SBI",   "ICICI", 20);
    addLink("HDFC",  "AXIS",  10);
    addLink("ICICI", "CITI",  12);
    addLink("AXIS",  "CITI",   8);
    showMap();

    // (g) Cheapest Route
    cout << "\n-- (g) CHEAPEST TRANSFER --\n";
    cheapestRoute("SBI",  "CITI");
    cheapestRoute("SBI",  "AXIS");

    // (h) Reserves
    cout << "\n-- (h) RESERVE MANAGER --\n";
    addReserve("SBI",   ledger["SBI"]);
    addReserve("HDFC",  ledger["HDFC"]);
    addReserve("ICICI", ledger["ICICI"]);
    addReserve("AXIS",  ledger["AXIS"]);
    checkReserves(5000000);

    cout << "\n===== SIMULATION COMPLETE =====\n";
    return 0;
}
