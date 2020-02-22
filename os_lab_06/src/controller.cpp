#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <unistd.h>
#include <sstream>
#include "lexicon.hpp"

// #define DEBUG

void CheckMailbox(zmq::socket_t& execRecieve) {
    zmq::message_t msg;
    while (execRecieve.recv(&msg)) {
        TLetter inLetter = *( static_cast<TLetter*> (msg.data()) );
        if (inLetter.commType == EXEC){
            if (inLetter.sig == SUCC) {
                execRecieve.recv(&msg);
                std::string report((char*)msg.data(), msg.size());
                std::cout << "MAILBOX: ID " << inLetter.id << " reports: " << report << std::endl;
            } else {
                std::cout << "ID " << inLetter.id << " reports: " << (inLetter.sig == FAIL ? "failure" : "error") << std::endl;
            }
        } else {
            std::cout << "MAILBOX: Broken IO message" << std::endl;
        }
    }
}

int main() {
    zmq::context_t context(1);

        //---------------//
        // Port creation //
        //---------------//

    zmq::socket_t treeSend   (context, ZMQ_ROUTER);
    treeSend.setsockopt(ZMQ_ROUTER_HANDOVER, 1);
    zmq::socket_t treeRecieve(context, ZMQ_PULL);
    treeRecieve.setsockopt(ZMQ_RCVTIMEO, TIMEOUT_MS);
    
    zmq::socket_t execRecieve(context, ZMQ_PULL);
    execRecieve.setsockopt(ZMQ_RCVTIMEO, EXEC_TIMEOUT_MS);

    std::string outPort, inPort, execPort;
    try {
    #ifdef DEBUG
        treeSend.bind("tcp://127.0.0.1:44050");
        treeRecieve.bind("tcp://127.0.0.1:44051");
        execRecieve.bind("tcp://127.0.0.1:44052");
    #else
        treeSend.bind("tcp://127.0.0.1:*");
        treeRecieve.bind("tcp://127.0.0.1:*");
        execRecieve.bind("tcp://127.0.0.1:*");
    #endif
    } catch (zmq::error_t& err) {
        std::cerr << "FATAL ERROR: Could not create one or multiple sockets" << std::endl;
        return err.num();
    }
    outPort  = ParsePort(treeSend);
    inPort   = ParsePort(treeRecieve);
    execPort = ParsePort(execRecieve);

        //----------------//
        // Main loop vars //
        //----------------//
    int id, parentID;
    std::string command;
    std::map<TNodeID, TNodeID> subtreeID;

    #ifdef DEBUG
        std::cout << "OUT: " << outPort << " | IN: " << inPort << " | EXEC: " << execPort << std::endl;
    #endif
 
    bool work = true;
    while (work) {
        std::cout << "► ";
        std::cin >> command;

        if (command == "create") {
            std::cin >> id >> parentID;
            if (id < 0) {
                std::cout << "New ID must be positive" << std::endl;
                continue;
            } else if (parentID < -1) {
                std::cout << "ERROR: Invalid parent ID" << std::endl;
                continue;
            } else {
                auto node = subtreeID.find(id);
                if (node != subtreeID.end()) {
                    std::cout << "ERROR: ID exists" << std::endl; 
                    continue;
                }
            }

            if (parentID == -1) {
            #ifdef DEBUG
                pid_t pid = 1;
            #else 
                pid_t pid = fork();
            #endif
                if (pid < 0) {
                    std::cerr << "ERROR: Failed to fork" << std::endl;
                    continue;
                } else if (pid == 0) {
                    std::string idStr( std::to_string(id) );
                    // WHICH FUCKING DUMBCUNT DECIDED "oh let's not make the name implicit like fucking shell" FUCK
                    // Total hours wasted here: 7
                    if ( execl(WORKER_NAME, WORKER_NAME, idStr.c_str(), outPort.c_str(), inPort.c_str(), execPort.c_str(), NULL) < 0 ) {
                        std::cerr << "ERROR: Failed to create new process" << std::endl;
                        continue;
                    }
                } else if (pid == 1) {
                    std::cout << "CONTROLLER: Manual server creation mode enabled" << std::endl;
                }
            } else {
                // Auto type : std::map<TNodeID, TNodeID>::iterator
                auto node = subtreeID.find(parentID);
                if (node == subtreeID.end()) {
                    std::cout << "ERROR: ID not found" << std::endl; 
                    continue;
                }

                    // Tell parent to work
                TLetter outLetter = {parentID, ADD, START};
                zmq::message_t msg(&outLetter, sizeof(TLetter));
                
                std::string childIdStr(std::to_string(node->second)); // Child of controller that has the parentID
                treeSend.send(childIdStr.c_str(), childIdStr.size(), ZMQ_SNDMORE);
                treeSend.send(msg, ZMQ_SNDMORE);

                    // Add new child ID
                childIdStr = std::to_string(id);
                treeSend.send(childIdStr.c_str(), childIdStr.size(), 0);
            }

            #ifdef DEBUG
                std::string waitInput;
                std::cout << "ARBITRARY_STRING_INPUT_WAIT: ";
                std::cin >> waitInput;
            #endif

            zmq::message_t msg;
            if ( treeRecieve.recv(&msg, 0) ) {
                TLetter res = *(static_cast<TLetter*> (msg.data()));
                if (parentID != -1) {
                    // Expect another ID message from node
                    treeRecieve.recv(&msg, 0);
                }
                if ( res.id == id && res.sig == SUCC) {
                    std::cout << "ID " << res.id << " reports: success" << std::endl;

                    if (parentID == -1) {
                        subtreeID.insert(std::pair<TNodeID, TNodeID>(id, id));
                    } else {
                        auto node = subtreeID.find(parentID);
                        subtreeID.insert(std::pair<TNodeID, TNodeID>(id, node->second));
                    }
                    
                } else if (res.sig == FAIL || res.sig == ERR) {
                    std::cout << "ID " << res.id << " reports: " << (res.sig == FAIL ? "failure" : "error") << std::endl;

                } else {
                    std::cout << "CONTROLLER: Broken IO message" << std::endl;
                }                    

            } else {
                std::cout << "Node is not responding" << std::endl;
            }

        } else if (command == "remove") {
            std::cin >> id;
            auto node = subtreeID.find(id);
            if (node == subtreeID.end()) {
                std::cout << "ERROR: ID not found" << std::endl; 
                continue;
            }

            TLetter ioLetter = {id, DEL, START};
            zmq::message_t msg ((void*)&ioLetter, sizeof(TLetter));
            std::string childID(std::to_string(node->second));

            treeSend.send(childID.c_str(), childID.size(), ZMQ_SNDMORE);
            treeSend.send(msg, 0);

            if ( treeRecieve.recv(&msg, 0) ) {
                ioLetter = *(static_cast<TLetter*> (msg.data()));
                if ( ioLetter.id == id && ioLetter.sig == SUCC) {
                    std::cout << "ID " << ioLetter.id << " reports: success" << std::endl;

                    zmq::message_t idStringMsg;
                    treeRecieve.recv(&idStringMsg);

                    std::string strOfID((char*)idStringMsg.data(), idStringMsg.size());
                    std::stringstream IDstream(strOfID);

                    TNodeID ID;
                    while (IDstream >> ID) {
                        subtreeID.erase(ID);
                    }
                    std::cout << "Subtree deleted" << std::endl;

                } else if (ioLetter.sig == FAIL || ioLetter.sig == ERR) {
                    std::cout << "ID " << ioLetter.id << " reports: " << (ioLetter.sig == FAIL ? "failure" : "error") << std::endl;

                } else {
                    std::cout << "CONTROLLER: Broken IO message" << std::endl;
                }                    

            } else {
                std::cout << "Node is not responding" << std::endl;
            }

        } else if (command == "exec") {
            std::cin >> id;
            if (id < 0) {
                std::cout << "ERROR: Incorrect ID" << std::endl; 
                continue;
            }
            auto node = subtreeID.find(id);
            if (node == subtreeID.end()) {
                std::cout << "ERROR: ID not found" << std::endl; 
                continue;
            }
            
            TLetter ioLetter = {id, EXEC, START};
            zmq::message_t msg ((void*)&ioLetter, sizeof(TLetter));
            std::string childID(std::to_string(node->second));

            treeSend.send(childID.c_str(), childID.size(), ZMQ_SNDMORE);
            treeSend.send(msg, ZMQ_SNDMORE);

            std::string pattern, text;
            std::cin >> text >> pattern;
            treeSend.send(std::string(pattern + " " + text).c_str(), pattern.size() + 1 + text.size());

            CheckMailbox(execRecieve);

        } else if (command == "ping") {
            std::cin >> id;
            if (id < 0) {
                std::cout << "ERROR: Incorrect ID" << std::endl; 
                continue;
            }
            auto node = subtreeID.find(id);
            if (node == subtreeID.end()) {
                std::cout << "ERROR: ID not found" << std::endl; 
                continue;
            }

            TLetter ioLetter = {id, PING, START};
            CommandRoute(treeSend, node->second, ioLetter);

            zmq::message_t msg;
            if (treeRecieve.recv(&msg, 0)) {
                ioLetter = *(static_cast<TLetter*> (msg.data()));
                if (ioLetter.id == id && ioLetter.sig == SUCC) {
                    std::cout << "ID " << ioLetter.id << " reports: success" << std::endl;
                    
                } else if (ioLetter.sig == FAIL || ioLetter.sig == ERR) {
                    std::cout << "ID " << ioLetter.id << " reports: " << (ioLetter.sig == FAIL ? "failure" : "error") << std::endl;

                } else {
                    std::cout << "CONTROLLER: Broken IO message" << std::endl;
                }                    
            } else {
                std::cout << "CONTROLLER: No response from node\n"
                          << "Delete node from the map? (y/n): "<< std::endl;
                char ch;
                do {
                    std::cin >> ch;
                    if (ch == 'y') { 
                        subtreeID.erase(id);
                    }
                    else if (ch == 'n') { ; }
                    else
                    {
                        std::cout << "(y/n): ";
                    }
                } while (ch != 'y' && ch != 'n');
            }
        } else if (command == "check") {
            // Do nothing, check the exec mailbox

        } else if (command == "exit") {
            for (std::pair<const TNodeID, TNodeID>& e : subtreeID) {
                if (e.first == e.second) {
                    std::string idStr(std::to_string(e.second));
                    TLetter end {e.second, DEL, START};
                    zmq::message_t endNote((void*)&end, sizeof(TLetter), 0);
                    treeSend.send(idStr.c_str(), idStr.size(), ZMQ_SNDMORE);
                    treeSend.send(endNote, 0);
                }
            }

            std::cout << "CONTROLLER: Ending service (EXEC mailbox will be checked once more)" << std::endl;
            work = 0;

        } else {
            std::cout << "CONTROLLER: Not a command" << std::endl;
            std::cin.ignore(256, '\n');
        }

        //--------------------//
        // EXEC mailbox check //
        //--------------------//

        CheckMailbox(execRecieve);
    }    
}