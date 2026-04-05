#pragma once

#include <netinet/in.h>
#include <string>
#include <mqueue.h>


class Server
{
    mqd_t messageQueue;
    pthread_t threadId;
    bool threadRunning;

    int createPassiveSocket(in_port_t port);
    static void* broadcastAgent(void *args);

public:
    static constexpr auto DEFAULT_NAME = "reference-server";
    static constexpr unsigned int DEFAULT_PORT = 8111;
    static constexpr unsigned int MAX_CONNECTION_COUNT = 20;

    static constexpr auto MESSAGE_QUEUE_CLOSE_COMMAND = "EXIT_QUEUE";
    static constexpr unsigned int MAXIMUM_QUEUE_SIZE = 10;

    static Server instance;

    int listenSock_fd;
    in_port_t port = DEFAULT_PORT;
    std::string serverName = DEFAULT_NAME;
    std::string msgQueueName;

    Server() = default;

    int connectionHandler();
    int broadcastAgentInit();
    void broadcastAgentCleanup();
    void cleanup();
};