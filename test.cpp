#include "tqdm/tqdm.hpp"
#include <iostream>
#include <thread>
#include <vector>

int main()
{
    std::vector<bool> v(1000);
    for (auto _ : tqdm(v, "test", std::cerr, 10)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
