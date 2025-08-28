#include<iostream>
using namespace std;
int main() {
    long long n, a, b, c;
    int d,e,f;
    cin >> n;
    int sum[n];
    for(int i = 0; i < n; i++)
        cin >> sum[i];

    long long sum1 = 0;
    for(long long i = 0; i < n; i++)
        sum1 += sum[i];

    cout << sum1 << endl;
}