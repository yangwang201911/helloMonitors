#include <iostream>
#include <thread>
#include <chrono>
#include "monitors/presenter.h"
int main(int argc, char* argv[]) {
    Presenter presenter("C");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    presenter.collect();
    for (auto item : presenter.reportMeans()) {
        std::cout << item << std::endl;
    }
    return 0;
}