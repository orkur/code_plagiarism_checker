#include <iostream>
#include "student_markers.h"
using namespace std;
int fib(int n) {
    STUDENT_START __marker_1_start__;
    if (n <= 1)
        return n;
    return fib(n - 1) + fib(n - 2);
    STUDENT_END __marker_1_end__;
}

int main() {
    int n;
    cin >> n;
    cout << fib(n);
}