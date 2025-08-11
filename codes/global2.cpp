#include <iostream>
using namespace std;
int a = 0, b = 1, c = 1;
int n;
int main() {
    cin >> n;
    for (int i = 2; i <= n; i++) {
        c = a + b;
        a = b;
        b = c;
    }
    cout << c << endl;
}