#include <cmath>
#include <iostream>
#include <set>
#include <climits>
#include <cstdio>
#include <algorithm>
#include <cassert>
#include <string>
#include <vector>
#include <iomanip>
#include <unordered_map>
#include <type_traits>
#include <string>
#include <queue>
#define ll long long
#include <map>

using namespace std;
const ll delta = 2e5;
int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    int N, M, K;
    cin >> N >> M >> K;
    vector<vector<pair<ll,ll>>> adj(N);
    for (int i = 0; i < M; i++) {
        ll a, b, c;
        cin >> a >> b >> c;
        a--, b--;
        adj[a].push_back({c, b});
    }
    vector<ll> dist[N];
    priority_queue<pair<ll,ll>> pq; //-dist, val
    pq.push({0, 0});
    while (!pq.empty()) {
        ll u = pq.top().second;
        ll d = -pq.top().first;
        pq.pop();
        if (dist[u].size() >= K) {
            continue;
        }
        dist[u].push_back(d);
        for (auto p: adj[u]) {
            pq.push({-p.first-d, p.second});
        }
    }
    for (ll i: dist[N - 1]) {
        cout << i << ' ';
    }
}