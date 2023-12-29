#include "tqdm/tqdm.hpp"
#include <iostream>
#include <vector>

int main()
{
    std::vector<bool> v(10000000);
    for (auto _ : tqdm(v, "test", std::cerr, 10, 20)) {}
}
