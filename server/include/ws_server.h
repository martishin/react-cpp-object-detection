#ifndef WS_SERVER_H
#define WS_SERVER_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <opencv2/opencv.hpp>
#include <queue>
#include <thread>
#include <uWebSockets/App.h>
#include <unordered_map>
#include "detector.h"

struct PerSocketData {
    uint64_t clientId;
};

class WebSocketServer {
public:
    WebSocketServer(int port, std::string modelConfiguration, std::string modelWeights, std::string classesFile,
                    float confThreshold = 0.5f, float nmsThreshold = 0.4f);
    ~WebSocketServer();

    void run();

private:
    void workerFunction(uWS::Loop *loop);

    int port;
    std::string modelConfiguration;
    std::string modelWeights;
    std::string classesFile;
    float confThreshold;
    float nmsThreshold;

    std::atomic<bool> isRunning;
    std::thread processingThread;
    std::vector<std::thread> workerThreads;
    int numThreads;

    std::atomic<uint64_t> clientIdCounter;

    std::queue<std::pair<cv::Mat, uint64_t>> frameQueue;
    std::mutex queueMutex;
    std::condition_variable queueCondVar;

    std::unordered_map<uint64_t, uWS::WebSocket<false, true, PerSocketData> *> clients;
    std::mutex clientsMutex;

    std::unordered_map<uint64_t, std::map<uint64_t, std::shared_ptr<std::string>>> frameBuffer;
    std::unordered_map<uint64_t, uint64_t> lastSentFrame;
    std::unordered_map<uint64_t, uint64_t> nextFrameNumber;
};

#endif // WS_SERVER_H
