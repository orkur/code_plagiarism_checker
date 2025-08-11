#include <iostream>

int main() {
    int a;
    std::cin >> a;
    int sum = 0;
    int i = 0;
    while (i < a) {
        int el;
        std::cin >> el;
        sum += el;
    }
    std::cout << sum;
}
