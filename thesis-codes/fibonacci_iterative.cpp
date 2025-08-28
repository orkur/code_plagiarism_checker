#include <iostream>
#include "student_markers.h"
using namespace std;
int fib(int n) {
    STUDENT_START __marker_1_start__;
    if (n <= 1)
        return n;
    int tab[3] = {0,1,0};
    for (int i = 2; i <= n; i++)
        tab[i%3] = tab[(i-1)%3] + tab[(i-2)%3];
    return tab[n%3];
    STUDENT_END __marker_1_end__;
}

int main() {
    int n;
    cin >> n;
    cout << fib(n);
}