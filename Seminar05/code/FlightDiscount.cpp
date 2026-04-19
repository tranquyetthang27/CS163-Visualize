#include <bits/stdc++.h>
using namespace std;
#define int long long
void hocusPocus()
{
    int n, m;
    cin >> n >> m;
    vector<vector<pair<int, int>>> adj(n);
    for (int i = 0; i < m; i++)
    {
        int u, v, w;
        cin >> u >> v >> w;
        --u, --v;
        adj[u].push_back({v, w});
    }
    vector<vector<int>> d(n, vector<int>(2, LLONG_MAX));
    d[0][0] = 0;
    const auto comp = [](auto &a, auto &b)
    {
        return a.dist > b.dist;
    };
    struct data
    {
        int node;
        int dist;
        bool halfey;
    };
    priority_queue<data, vector<data>, decltype(comp)> q(comp);
    q.push({0, 0, 0});
    while (!q.empty())
    {
        auto [u, dist, isused] = q.top();
        q.pop();
        if (dist > d[u][isused])
            continue;
        for (auto [v, w] : adj[u])
        {
            if (d[v][isused] > d[u][isused] + w)
                d[v][isused] = d[u][isused] + w, q.push({v, d[v][isused], isused});
            if (!isused)
                if (d[v][1] > d[u][0] + w / 2)
                    d[v][1] = d[u][0] + w / 2, q.push({v, d[v][1], 1});
        }
    }
    cout << d[n - 1][1] << endl;
}
signed main()
{
    ios::sync_with_stdio(false), cin.tie(NULL);
    hocusPocus();
}