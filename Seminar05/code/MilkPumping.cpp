// Source: https://usaco.guide/general/io

#include <bits/stdc++.h>
using namespace std;

struct Pipe{
	int next; int cost; int flow;
};

int main() {
	freopen("pump.in", "r", stdin);
	freopen("pump.out", "w", stdout);
	int n, m; cin >> n >> m;
	vector<Pipe> adj[n+1];
	for(int i = 0; i < m; i++){
		int a, b, c, d; cin >> a >> b >> c >> d;
		adj[a].push_back({b, c, d});
		adj[b].push_back({a, c, d});
	}
	int INF = 100000000;
	int distances[n+1];
	for(int& i : distances) i = 0;
	priority_queue<tuple<int, int, int>, vector<tuple<int, int, int>>, greater<tuple<int, int, int>>> pq;
	pq.push({0, INF, 1});
	while(!pq.empty()){
		tuple<int, int, int> u = pq.top(); pq.pop();
		int dist = get<0>(u);
		int node = get<2>(u);
		int flow_rate = get<1>(u);
		for(Pipe i : adj[node]){
			int temp_flow = min(flow_rate, i.flow);
			if(((temp_flow)*1000000)/(dist+i.cost) > distances[i.next]){
				distances[i.next] = ((temp_flow)*1000000)/(dist+i.cost);
				pq.push({dist+i.cost, temp_flow, i.next});
			}
		}
	}
	cout << distances[n];
}