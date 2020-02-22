#pragma once

#include <iostream>
#include <exception>

using RawByte = char*;

class BlockAlloc {
public:
    BlockAlloc()
    {
        // 4 blocks of 32,  4 blocks of 64,  4 blocks of 128
        // 4 blocks of 256, 2 blocks of 512, 1 blocks of 1024
        allMemory = static_cast<RawByte>(::operator new(
            4 * 32 + 4 * 64 + 4 * 128 + 4 * 256 + 2 * 512 + 1 * 1024 
        ));

            // Binding blocks of 32
        BindBlocks(32, 4, 0);
        list[0] = allMemory;
            // Binding blocks of 64
        BindBlocks(64, 4, 128);
        list[1] = allMemory + 128;
            // Binding blocks of 128
        BindBlocks(128, 4, 384);
        list[2] = allMemory + 384;
            // Binding blocks of 256
        BindBlocks(256, 4, 896);
        list[3] = allMemory + 896;
            // Binding blocks of 512
        BindBlocks(512, 2, 1920);
        list[4] = allMemory + 1920;
            // Binding blocks of 1024
        BindBlocks(1024, 1, 2944);
        list[5] = allMemory + 2944;
    }

    ~BlockAlloc()
    {
        ::operator delete(allMemory);
    }

    void *allocate(size_t size)
    {
        size = Rounder(size);

        int i;
        switch (size)
        {
            case 32   : i = 0; break;
            case 64   : i = 1; break;
            case 128  : i = 2; break;
            case 256  : i = 3; break;
            case 512  : i = 4; break;
            case 1024 : i = 5; break;
            default   : throw std::bad_alloc();
        }

        for ( ; i < 6; ++i)
        {
            if (list[i] != nullptr)
            {
                RawByte** result = (RawByte**) list[i];
                list[i] = *((RawByte*) list[i]);
                *result = &list[i];
                // RawByte res = (RawByte)result + sizeof(RawByte**);
                return (RawByte) ((RawByte)result + sizeof(RawByte**));
            }
        }

        throw std::bad_alloc();
    }

    void deallocate(void *ptr)
    {
        // -How many pointers do you need?
        // -Yes.

        RawByte* node = (RawByte*) ((RawByte) ptr - sizeof(RawByte));
        // Node head --> List --> Next element in list, so double pointer
        RawByte **pointerToList = *((RawByte***)node);
        *node = (RawByte)*pointerToList;
        *pointerToList = node;
    }

private:

    size_t Rounder(const size_t size)
    {
        if (size <= minBlockSize - sizeof(RawByte)) return minBlockSize;

        size_t roundUp = minBlockSize;
        while (roundUp - sizeof(RawByte) < size) { roundUp <<= 1; }
        return roundUp;
    }

    void BindBlocks(size_t size, size_t amount, unsigned int offset)
    {
        RawByte *constructor = (RawByte*) (allMemory + offset);
        for (size_t i = 1; i < amount; ++i)
        {
            *constructor = allMemory + offset + i * size;
             constructor = (RawByte*) (allMemory + offset + i * size);
        }
        *constructor = nullptr;
    }

    RawByte list[6];
    RawByte allMemory;
    size_t  minBlockSize = 32;
};