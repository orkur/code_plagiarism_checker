#include <iostream>
using namespace std;

void fib(int n) {
    int a = 0, b = 1, c = 1;
    for (int i = 2; i <= n; i++) {
        c = a + b;
        a = b;
        b = c;
    }
    cout << c << endl;
}

int main() {
    int n;
    cin >> n;
    fib(n);
}