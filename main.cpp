//
// Created by dferraro on 06/12/23.
//

#include <vector>
#include <functional>
#include <map>
#include <thread>
#include <mutex>
#include <optional>
#include <iostream>

#include "Farm.hpp"


#define INT_ARG(default_val, index) (argc <= index ? default_val : atoi(argv[index]))

void print(double arg) {
    std::cout << "got: " << arg << std::endl;
}

int main(int argc, char *argv[]) {
    int pardegree = INT_ARG(16, 1);
    //Farm farm(pardegree, [](){ return 1704; }, [](int val) { return val + 1; }, [](int val){ });
    std::vector<int> stream(30);
    for (int i = 1; i <= stream.size(); ++i) {
        stream[i-1] = i * i;
    }

    auto fun = [](int value) {
        return value / 2.0;
    };
    Farm<int, double> farm(pardegree, fun, &print);
    farm.run();
    for (int i = 0; i < stream.size(); ++i) {
        farm.send(stream[i]);
    }
    farm.notify_eos();
    farm.wait();

    return(0);
}