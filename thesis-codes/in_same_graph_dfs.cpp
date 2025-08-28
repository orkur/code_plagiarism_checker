#include<iostream>
#include<vector>

#include "student_markers.h"
using namespace std;
vector<int> adj[100];
bool vis[100];
void dfs(int u) {
    vis[u] = true;
    for (auto v :  adj[u]) {
        if (!vis[v]) {
            dfs(v);
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
    dfs(0);
    int k;
    cin >> k;
    for(int i = 0; i < k; i++) {
        int v;
        cin >> v;
        cout << vis[v];
    }
}