#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include "BuddyAlloc.h"

using TAllocPair = std::pair<size_t, char*>;

void Test(Allocator& bAlloc, unsigned int cycles)
{
    const int blockSizes[] = {32, 64, 128, 256, 512};
    int maxSize = 1024, blockSize;
    TAllocPair pair;
    std::vector<TAllocPair> vec;

    auto timeBegin = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < cycles; ++i)
    {
        maxSize = bAlloc.MaxAlloc();
        if (maxSize == 0)
        {
            auto it = vec.begin() + std::rand() % vec.size();
            bAlloc.deallocate(it->second, it->first);
            maxSize += it->first;
                // std::cout << "Deallocate " << it->first << std::endl;
            vec.erase(it);
        }
        else if (maxSize == 1024)
        {
            blockSize = blockSizes[std::rand() % 5];
            vec.push_back(std::pair<size_t, char*> (blockSize, (char*)bAlloc.allocate(blockSize)));
            maxSize -= blockSize;
                // std::cout << "Allocate " << blockSize << std::endl;
        }
        else
        {
            if (std::rand() % 2)
            {
                     if (maxSize >= 512)  { blockSize = blockSizes[std::rand() % 5]; }
                else if (maxSize >= 256)  { blockSize = blockSizes[std::rand() % 4]; }
                else if (maxSize >= 128)  { blockSize = blockSizes[std::rand() % 3]; }
                else if (maxSize >= 64 )  { blockSize = blockSizes[std::rand() % 2]; }
                else if (maxSize >= 32 )  { blockSize = blockSizes[0]; }

                vec.push_back(std::pair<size_t, char*> (blockSize, (char*)bAlloc.allocate(blockSize)));
                maxSize -= blockSize;
                // std::cout << "Allocate " << blockSize << std::endl;
            }
            else
            {
                auto it = vec.begin() + std::rand() % vec.size();
                bAlloc.deallocate(it->second, it->first);
                maxSize += it->first;
                // std::cout << "Deallocate " << it->first << std::endl;
                vec.erase(it);
            }
            
        }

        // bAlloc.dump();
        // std::cout << std::endl;
    }

    auto timeEnd = std::chrono::high_resolution_clock::now();
    std::cout << cycles << "\t" << std::chrono::duration<double>(timeEnd - timeBegin).count() << std::endl;
}

int main(int argc, char* argv[])
{
    std::srand(std::time(nullptr));
    if (argc < 2)
    {
        std::cerr << "NOT_ENOUGH_ARGUMENTS" << std::endl;
        return -1;
    }

    Allocator bAlloc(32, 1024);

    Test(bAlloc, std::atoi(argv[1]));
}