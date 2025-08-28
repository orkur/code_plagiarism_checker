#include<iostream>
#include<vector>
#include<queue>

#include "../codes/student_markers.h"
using namespace std;
vector<int> adj[100];
bool vis[100];
void bfs(int a) {
    queue<int> q;
    q.push(a);
    vis[a] = true;
    while(!q.empty()) {
        int u = q.front();
        q.pop();
        for(int v : adj[u]) {
            if(!vis[v]) {
                vis[v] = true;
                q.push(v);
            }
        }
    }
}
int main() {
    int n;
    cin >> n;
    for(int i = 0; i < n; i++) {
        int a, b;
        cin >> a >> b;
        adj[a].push_back(b);
        adj[b].push_back(a);
    }
    bfs(0);
    int k;
    cin >> k;
    for(int i = 0; i < k; i++) {
        int v;
        cin >> v;
        cout << vis[v];
    }
}