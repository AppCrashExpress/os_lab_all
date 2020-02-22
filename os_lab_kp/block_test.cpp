#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <vector>
#include "BlockAlloc.h"

void Fill(size_t size, size_t amount, std::vector<size_t>& free)
{
    for (int i = 0; i < amount; ++i)
    {
        free.push_back(size);
    }
}

void test(unsigned int cycles, BlockAlloc& blAlloc)
{
    std::vector<size_t> free;
    std::vector< std::pair<void*, size_t> > allocated;

        // Fill free
    Fill(24,   4, free);
    Fill(56,   4, free);
    Fill(120,  4, free);
    Fill(248,  4, free);
    Fill(504,  2, free);
    Fill(1016, 1, free);

    auto timeBegin = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < cycles; ++i)
    {
        if (free.empty())
        {
            auto it = allocated.begin() + std::rand() % allocated.size();
            free.push_back(it->second);
            blAlloc.deallocate(it->first);
            allocated.erase(it);
        }
        else if (allocated.empty())
        {
            auto it = free.begin() + std::rand() % free.size(); 
            void *ptr = blAlloc.allocate(*it);
            if (ptr == NULL) throw std::bad_alloc();
            allocated.push_back(std::pair<void*, size_t> (ptr, *it));
            free.erase(it);
        }
        else
        {
            if (std::rand() % 2)
            {
                auto it = free.begin() + std::rand() % free.size(); 
                allocated.push_back(std::pair<void*, size_t> (blAlloc.allocate(*it), *it));
                free.erase(it);
            }
            else
            {
                auto it = allocated.begin() + std::rand() % allocated.size();
                free.push_back(it->second);
                blAlloc.deallocate(it->first);
                allocated.erase(it);
            }
        }
    }

    auto timeEnd = std::chrono::high_resolution_clock::now();
    std::cout << cycles << "\t" << std::chrono::duration<double>(timeEnd - timeBegin).count() << std::endl;
    // std::cout << "TEST_OUT0" << std::endl;
}

int main(int argc, char* argv[])
{
    std::srand(std::time(nullptr));
    if (argc < 2)
    {
        std::cerr << "NOT_ENOUGH_ARGUMENTS" << std::endl;
        return -1;
    }
    BlockAlloc blAlloc;

    test(std::atoi(argv[1]), blAlloc);
}