#include <set>
#include "student_markers.h"
using namespace std;

int foo;
class Graph {
public:
    unordered_map<string, set<string>> adj;
    void build();

};
#include <iostream>


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
    STUDENT_START __marker_1_start__;
    int n;
    cin >> n;
    fib(n);
    STUDENT_END __marker_1_end__;

}