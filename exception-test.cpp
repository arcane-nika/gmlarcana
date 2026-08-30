#include <iostream>
#include <stdexcept>

struct TinyError {};

int main() {
    std::cout << "1: before int throw\n";
    try {
        throw 123;
    } catch (int v) {
        std::cout << "2: caught int: " << v << "\n";
    }

    std::cout << "3: before TinyError throw\n";
    try {
        throw TinyError{};
    } catch (const TinyError&) {
        std::cout << "4: caught TinyError\n";
    }

    std::cout << "5: before runtime_error throw\n";
    try {
        throw std::runtime_error("hello");
    } catch (const std::runtime_error& e) {
        std::cout << "6: caught runtime_error: " << e.what() << "\n";
    }

    std::cout << "7: SUCCESS\n";
    return 0;
}
