#include <optional>
#include <iostream>

int main() {
    std::optional<int> opt = 42;
    if (opt) {
        std::cout << "Optional value: " << *opt << std::endl;
    } else {
        std::cout << "Optional is empty" << std::endl;
    }
    return 0;
}