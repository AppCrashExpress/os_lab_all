#pragma once

#include <string>
#include <zmq.hpp>

    //----------//
    // Settings //
    //----------//

const int TIMEOUT_MS = 5000;
const int EXEC_TIMEOUT_MS = 200;
const size_t MAX_STRING_LENGTH = 256;
const char WORKER_NAME[] = "worker";

    //----------------------//
    // Types and structures //
    //----------------------//

using TNodeID = int;

enum CommType { EXEC, PING, ADD, DEL };

enum Signal { START, SUCC, FAIL, ERR }; 

struct TLetter {
    // Addressing <id>
    TNodeID id;
    CommType commType;
    Signal sig;
};

    //-----------//
    // Functions //
    //-----------//

const size_t PORT_SIZE = 10;
std::string ParsePort(zmq::socket_t& socket) {
    char address[1024];
    char port[10] = {}; 
    size_t size = sizeof(address);
    socket.getsockopt(ZMQ_LAST_ENDPOINT, &address, &size);

    int i = 0, j = 0;
    while (address[i - 1] != ':') ++i;
    ++i;
    while (address[i - 1] != ':') ++i;

    for (; address[i] != '\0' && i < 1024 && j < PORT_SIZE; ++j, ++i) {
        port[j] = address[i];
    }

    return std::string (port);
}

void CommandRoute(zmq::socket_t& socket, const int id, const TLetter command) {
    std::string idStr (std::to_string(id));

    // Found out that you can't write directly to message
    // Total hours wasted here: ~20
    // That one was my fault. We get it.
    socket.send(idStr.c_str(), idStr.size(), ZMQ_SNDMORE);
    socket.send((void*)&command, sizeof(TLetter), 0);
}

// In file search.o

std::string FindPos(const std::string pattern, const std::string text);

// Result - indexes of match ups, divided by ";"
// If none found, empty string