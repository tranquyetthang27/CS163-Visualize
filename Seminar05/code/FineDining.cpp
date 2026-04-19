#include "bits/stdc++.h"
#define fast ios_base::sync_with_stdio(0) , cin.tie(0) , cout.tie(0)
#define endl '\n'
#define int long long
#define f first
#define mp make_pair
#define s second
using namespace std;

int n, m, k;
unordered_map<int, unordered_map<int,int> > price;
int dist1[50005], dist2[50005]; 
vector <int> g[50005];
priority_queue<pair<int,int>, vector<pair<int,int> >, greater<pair<int,int> > > pq;

signed main()
{
    freopen("dining.in", "r", stdin); freopen("dining.out", "w", stdout);
    fast;
    cin >> n >> m >> k;
    for(int i = 1; i <= n; i++) dist1[i] = dist2[i] = INT_MAX;
    for(int i = 0; i < m; i++){
        int a, b, t;
        cin >> a >> b >> t;
        g[a].push_back(b);
        g[b].push_back(a);
        price[a][b] = price[b][a] = t;
    }
    dist1[n] = 0;
    pq.push(mp(0, n));
    while(!pq.empty()){
        int time = pq.top().f;
        int in = pq.top().s;
        pq.pop();
        if(dist1[in] != time) continue;
        for(int a : g[in]){
            if(dist1[in] + price[in][a] < dist1[a]){
                dist1[a] = dist1[in] + price[in][a];
                pq.push(mp(dist1[a], a));
            }
        }
    }
    for(int i = 0; i < k; i++){
        int a, yum;
        cin >> a >> yum;
        dist2[a] = dist1[a] - yum;
        pq.push(mp(dist2[a], a));
    }
    while(!pq.empty()){
        int time = pq.top().f;
        int in = pq.top().s;
        pq.pop();
        if(dist2[in] != time) continue;
        for(int a : g[in]){
            if(dist2[in] + price[in][a] < dist2[a]){
                dist2[a] = dist2[in] + price[in][a];
                pq.push(mp(dist2[a], a));
            }
        }
    }
    for(int i = 1; i < n; i++) cout << (dist2[i] <= dist1[i]) << endl;
}