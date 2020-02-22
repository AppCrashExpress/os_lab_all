#include <zmq.hpp>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <thread>
#include <unistd.h>
#include "lexicon.hpp"

// #define DEBUG

void ConnectToPort(const std::string port, zmq::socket_t& socket) {
    std::string address = "tcp://localhost:" + port;
    socket.connect(address.c_str());
}

zmq::message_t Confirmation(const TNodeID id) {
    TLetter res {id, ADD, SUCC};
    zmq::message_t msg((void*) &res, sizeof(TLetter));
    return msg;
}

void WorkerRoutine(zmq::context_t& context, const std::string& port, const TNodeID id, bool& work) {
    zmq::socket_t execSend(context, ZMQ_PUSH);
    ConnectToPort(port, execSend);
    zmq::socket_t threadRecieve(context, ZMQ_PULL);
    threadRecieve.connect("inproc://silkroad");

    zmq::message_t inMsg;
    while(work) {
        if (threadRecieve.recv(&inMsg, ZMQ_NOBLOCK)) {
            std::string all((char*) inMsg.data(), inMsg.size()),
                        pattern, text;
            std::stringstream allStream (all);
            allStream >> pattern >> text;
            std::string result( FindPos(pattern, text) );
            
            TLetter outLetter = {id, EXEC, SUCC};
            execSend.send((void*)&outLetter, sizeof(TLetter), ZMQ_SNDMORE);
            
            zmq::message_t outMsg;
            if (result.size() == 0) {
                outMsg.rebuild("-1", 2);
            } else {
                outMsg.rebuild((void*)result.c_str(), result.size());
            }
            execSend.send(outMsg, 0);
        }
    }
    
    return;
}

int main(int argc, char* argv[]) {
    //Arguments are: ID, parent "from" port, parent "to" port, controller exec port
    if (argc < 5) {
    #ifdef DEBUG
        std::cerr << "INVALID_ARG_NUM" << std::endl;
    #endif
        return -1;        
    }

    TNodeID id = std::atoi(argv[1]);
    std::string idStr(argv[1]),
                parentRcvPort(argv[2]),
                parentSndPort(argv[3]),
                controllerPort(argv[4]),
                childrenSendPort,
                childrenRecievePort;

        //-------------------//
        // Establish sockets //
        //-------------------//
    zmq::context_t context(1);
        // Upstream: 2 sockets and exec socket
    zmq::socket_t parentRecieve(context, ZMQ_DEALER);
    zmq::socket_t parentSend   (context, ZMQ_PUSH);
    parentRecieve.setsockopt(ZMQ_IDENTITY, idStr.c_str(), idStr.size()); //Discards '\0'
    ConnectToPort(parentRcvPort,  parentRecieve);
    ConnectToPort(parentSndPort,  parentSend);

        // Downstream: 2 sockets    
    zmq::socket_t childrenSend   (context, ZMQ_ROUTER);
    zmq::socket_t childrenRecieve(context, ZMQ_PULL);
    childrenRecieve.setsockopt(ZMQ_RCVTIMEO, TIMEOUT_MS);

    childrenSend.bind("tcp://127.0.0.1:*");
    childrenRecieve.bind("tcp://127.0.0.1:*");
    childrenSendPort    = ParsePort(childrenSend);
    childrenRecievePort = ParsePort(childrenRecieve);

        // In process: thread
    zmq::socket_t threadSend(context, ZMQ_PUSH);
    threadSend.bind("inproc://silkroad");

    bool work = true;
    std::thread execThread(WorkerRoutine, std::ref(context), std::cref(controllerPort), id, std::ref(work));

        //---------------------//
        // Send "Okay message" //
        //---------------------//
    zmq::message_t msg = Confirmation(id);
    parentSend.send(msg);

        //------------------//
        // Setup poll items //
        //------------------//

    zmq::pollitem_t items[] = {
        {(void*)parentRecieve,   0, ZMQ_POLLIN, 0},
        {(void*)childrenRecieve, 0, ZMQ_POLLIN, 0}
    };

    #ifdef DEBUG
        std::cout << "OUT: " << childrenSendPort << " | IN: " << childrenRecievePort << std::endl;
    #endif
        //----------------//
        // Main loop vars //
        //----------------//
    std::map<TNodeID, TNodeID> subtreeID;
    while (work) {
        zmq::message_t message;
        zmq::poll(items, 2, -1);

        if(items[0].revents & ZMQ_POLLIN) {
            parentRecieve.recv(&message, ZMQ_DONTWAIT);
            TLetter ioLetter = *(static_cast<TLetter*> (message.data()));
            
            if (ioLetter.id == id) {
                    // Initialize data to avoid doing that in GOTO statements
                pid_t pid;
                zmq::message_t idMessage,
                               returnMessage;
                std::string childID, strOfID; // childID for one ID, strOfID for string of IDs in delete

                switch (ioLetter.commType) {
                case ADD:
                    parentRecieve.recv(&idMessage, ZMQ_DONTWAIT);
                    childID = std::string((char*)idMessage.data(), idMessage.size());

                    #ifdef DEBUG
                        pid = 1;
                    #else 
                        pid = fork();
                    #endif
                    if (pid < -1) {
                        ioLetter.sig = ERR;
                        parentSend.send(&ioLetter, sizeof(TLetter), 0);
                        break;
                        
                    } else if (pid == 0) {
                        if ( execl(WORKER_NAME,
                                    WORKER_NAME,
                                    childID.c_str(),
                                    childrenSendPort.c_str(),
                                    childrenRecievePort.c_str(),
                                    controllerPort.c_str(),NULL) < 0 ) 
                        {
                            #ifdef DEBUG
                                std::cerr << "ERROR: Failed to create new process" << std::endl;
                            #endif
                            return -1;
                        }
                    } else if (pid == 1) {
                        #ifdef DEBUG
                            std::cout << "MANUAL_SERVER_CREATION_MODE" << std::endl;
                        #endif
                    }

                    if (childrenRecieve.recv(&returnMessage)) {
                        // Send the return of a child, containing its id
                        parentSend.send(returnMessage, ZMQ_SNDMORE);
                        parentSend.send(idStr.c_str(), idStr.size(), 0);

                        // Add id of this node to map in a parent nodes
                        TNodeID childIdInt = std::atoi(childID.c_str());
                        subtreeID.insert(std::pair<TNodeID, TNodeID> (childIdInt, childIdInt));

                    } else {
                        ioLetter.id  = id; // Replace with id of this node
                        ioLetter.sig = FAIL; // Notify of failure
                        returnMessage.rebuild((void*)&ioLetter, sizeof(TLetter));
                        parentSend.send(returnMessage);
                    }
                    break;

                case DEL:
                    // Send delete command to kids to trigger recursion;
                    for (std::pair<const TNodeID, TNodeID>& e : subtreeID) {
                        // Adjacent node are reachable from themselves
                        if (e.first == e.second) {
                            std::string childStr(std::to_string(e.second));
                            TLetter end {e.second, DEL, START};
                            zmq::message_t endNote((void*)&end, sizeof(TLetter), 0);
                            childrenSend.send(childStr.c_str(), childStr.size(), ZMQ_SNDMORE);
                            childrenSend.send(endNote, 0);
                            if (!childrenRecieve.recv(&message, 0)) {
                                // Apparently, messages need to synchnorise as well
                                // Jesus fucking christ
                                TLetter childLetter = *( static_cast<TLetter*> (message.data()) );
                                parentSend.send((void*)&childLetter, sizeof(TLetter), 0);
                                continue;
                            }
                        }
                    }
                    
                    // Build a string containing all IDs of this subtree
                    for (auto e : subtreeID) {
                        strOfID += std::to_string(e.first) + " ";
                    }
                    strOfID += idStr; // ID of this node

                    ioLetter.sig = SUCC;
                    returnMessage.rebuild(strOfID.c_str(), strOfID.size());
                    parentSend.send((void*)&ioLetter, sizeof(TLetter), ZMQ_SNDMORE);
                    parentSend.send(returnMessage, 0);
                    
    
                    parentSend.disconnect(std::string ("tcp://localhost:") + parentSndPort);
                    work = 0;
                    
                    #ifdef DEBUG
                        std::cout << "ENDING_SERVICE" << std::endl;
                    #endif
                    
                    break;

                case EXEC:
                    parentRecieve.recv(&message, 0);
                    threadSend.send(message, 0);
                    break;

                case PING:
                    TLetter res;
                    res = { id, PING, SUCC };
                    message.rebuild(&res, sizeof(TLetter));
                    parentSend.send(message);
                    break;
                }

                //---------------------------//
                // Redirect message to child //
                //---------------------------//

            } else {
                auto node = subtreeID.find(ioLetter.id);
                if (node == subtreeID.end()) {
                    ioLetter.id = id;
                    ioLetter.sig = FAIL;
                    parentSend.send(&ioLetter, sizeof(TLetter), 0);
                    continue;
                }

                std::string childID(std::to_string(node->second));
                zmq::message_t msg(childID.c_str(), childID.size());
                childrenSend.send(msg, ZMQ_SNDMORE);
                msg.rebuild((void*)&ioLetter, sizeof(TLetter));
                if (ioLetter.commType == ADD || ioLetter.commType == EXEC) {
                    // In ADD and EXEC signal we will send additional frame,
                    // containing child id or two char arrays respectivly
                    childrenSend.send(msg, ZMQ_SNDMORE);
                    parentRecieve.recv(&msg);
                    childrenSend.send(msg, 0);
                    
                } else {
                    // Otherwise it's just one frame;
                    childrenSend.send(msg, 0);
                }

            }

            //-----------------------------//
            // Message recieved from child //
            //-----------------------------//
        } else if (items[1].revents & ZMQ_POLLIN) {
            childrenRecieve.recv(&message, ZMQ_DONTWAIT);
            TLetter ioLetter = *(static_cast<TLetter*> (message.data()) );

            if (ioLetter.commType == ADD && ioLetter.sig == SUCC) {
                    // Recieve the id of the child, adjacent to this node
                zmq::message_t idMsg;
                childrenRecieve.recv(&idMsg);
                std::string childIdStr((char*)idMsg.data(), idMsg.size());

                    // Map created id and childs id
                subtreeID.insert( std::pair<TNodeID, TNodeID> ( ioLetter.id, std::atoi(childIdStr.c_str()) ) );
                
                    //Resend signal and id of this node to its parent
                idMsg.rebuild(idStr.c_str(), idStr.size());
                parentSend.send(message, ZMQ_SNDMORE);
                parentSend.send(idMsg, 0);
            
            } else if (ioLetter.commType == DEL && ioLetter.sig == SUCC) {
                zmq::message_t idStringMsg;
                childrenRecieve.recv(&idStringMsg);

                std::string strOfID((char*)idStringMsg.data(), idStringMsg.size());
                std::stringstream IDstream(strOfID);

                // Delete all ids in the map of this node
                TNodeID ID;
                while (IDstream >> ID) {
                    subtreeID.erase(ID);
                }

                // Continue sending
                parentSend.send(message, ZMQ_SNDMORE);
                parentSend.send(idStringMsg, 0);

            } else {
                    // There should never be EXEC signal,
                    // DEL or ADD with FAIL, PING shouldn't countain
                    // additional messages. Simply resend.
                parentSend.send(message, 0);
            }
        }
    }

    execThread.join();
}