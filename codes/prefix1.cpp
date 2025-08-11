#include <iostream>

int main() {
    int a;
    std::cin >> a;
    int tab[a];
    int sum = 0;
    int i = 0;
    while (i < a) {
        std::cin >> tab[i];
    }
    for (int i = 0; i < a; i++) {
        sum += tab[i];
    }
    std::cout << sum;

}
