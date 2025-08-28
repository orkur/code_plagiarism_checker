#include<iostream>
using namespace std;
int main() {
    int n;
    int sum[n];
    int sum1 = 0;
    cin >> n;
    for(int i = 0; i < n; i++) {
        cin >> sum[i];
    }
    for(int i = 0; i < n; i++) {
        sum1 += sum[i];
    }
    cout << sum1 << endl;
}