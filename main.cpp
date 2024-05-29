#include <iostream>
#include <thread>
#include <chrono>
#include "monitors/presenter.h"
int main(int argc, char* argv[]) {
    Presenter presenter("C");
    int index = 0;
    while (index++ < 10) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        presenter.collect();
        for (auto item : presenter.reportMeans()) {
            std::cout << "\n===\n" << item << "\n===" << std::endl;
        }
    }
    return 0;
}