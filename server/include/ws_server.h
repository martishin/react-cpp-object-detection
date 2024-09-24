#ifndef WS_SERVER_H
#define WS_SERVER_H

#include <opencv2/opencv.hpp>
#include <uwebsockets/App.h>
#include "detector.h"

struct PerSocketData
{
    // You can add per-socket data here if needed
};

class WebSocketServer
{
public:
    WebSocketServer(int port, Detector &detector);
    ~WebSocketServer();

    void run();

private:
    int port;
    Detector &detector;
};

#endif // WS_SERVER_H
