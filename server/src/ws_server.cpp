#include "ws_server.h"
#include <iostream>

WebSocketServer::WebSocketServer(const int port, std::string modelConfiguration, std::string modelWeights,
                                 std::string classesFile, const float confThreshold, const float nmsThreshold) :
    port(port), isRunning(true), numThreads(std::thread::hardware_concurrency()),
    modelConfiguration(std::move(modelConfiguration)), modelWeights(std::move(modelWeights)),
    classesFile(std::move(classesFile)), confThreshold(confThreshold), nmsThreshold(nmsThreshold), clientIdCounter(0) {
    if (numThreads == 0) {
        numThreads = 4;
    }
    std::cout << "WebSocketServer initialized with " << numThreads << " worker threads." << std::endl;
}

WebSocketServer::~WebSocketServer() {
    isRunning = false;
    queueCondVar.notify_all();

    for (auto &thread : workerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    std::cout << "WebSocketServer shutting down." << std::endl;
}

void WebSocketServer::run() {
    uWS::Loop *loop = uWS::Loop::get();

    for (int i = 0; i < numThreads; ++i) {
        workerThreads.emplace_back(&WebSocketServer::workerFunction, this, loop);
    }

    uWS::App()
        .ws<PerSocketData>("/*",
                           {.compression = uWS::SHARED_COMPRESSOR,
                            .maxPayloadLength = 50 * 1024 * 1024, // 50 MB
                            .idleTimeout = 10,
                            .open =
                                [this](auto *ws) {
                                    uint64_t clientId = clientIdCounter++;
                                    ws->getUserData()->clientId = clientId;

                                    {
                                        std::lock_guard<std::mutex> lock(clientsMutex);
                                        clients[clientId] = ws;
                                        lastSentFrame[clientId] = 0;
                                        nextFrameNumber[clientId] = 1;
                                    }

                                    std::cout << "Client connected, ID: " << clientId << std::endl;
                                },
                            .message =
                                [this](auto *ws, std::string_view message, uWS::OpCode opCode) {
                                    std::cout << "Received a message with opcode: " << opCode
                                              << ", message size: " << message.size() << std::endl;

                                    if (opCode == uWS::OpCode::BINARY) {
                                        uint64_t clientId = ws->getUserData()->clientId;

                                        try {
                                            std::vector<uchar> data(message.begin(), message.end());
                                            cv::Mat img = cv::imdecode(data, cv::IMREAD_COLOR);
                                            if (img.empty()) {
                                                std::cerr << "Failed to decode image" << std::endl;
                                                return;
                                            }

                                            {
                                                std::lock_guard<std::mutex> lock(queueMutex);
                                                frameQueue.emplace(img, clientId);
                                            }
                                            queueCondVar.notify_one();
                                        } catch (const std::exception &e) {
                                            std::cerr << "Exception during message processing: " << e.what()
                                                      << std::endl;
                                        }
                                    } else {
                                        std::cerr << "Received non-binary message with opcode: " << opCode << std::endl;
                                    }
                                },
                            .close =
                                [this](auto *ws, int /*code*/, std::string_view /*message*/) {
                                    uint64_t clientId = ws->getUserData()->clientId;

                                    {
                                        std::lock_guard<std::mutex> lock(clientsMutex);
                                        clients.erase(clientId);
                                        frameBuffer.erase(clientId);
                                        lastSentFrame.erase(clientId);
                                        nextFrameNumber.erase(clientId);
                                    }

                                    std::cout << "Client disconnected, ID: " << clientId << std::endl;
                                }})
        .listen(port,
                [this](const auto *listenSocket) {
                    if (listenSocket) {
                        std::cout << "Server listening on port " << port << std::endl;
                    } else {
                        std::cerr << "Failed to listen on port " << port << std::endl;
                    }
                })
        .run();

    isRunning = false;
    queueCondVar.notify_all();
}

void WebSocketServer::workerFunction(uWS::Loop *loop) {
    try {
        Detector detector(modelConfiguration, modelWeights, classesFile, confThreshold, nmsThreshold);

        if (!detector.isValid()) {
            std::cerr << "Detector failed to initialize in worker thread." << std::endl;
            return;
        }

        std::cout << "Detector initialized successfully in worker thread." << std::endl;

        while (isRunning) {
            std::pair<cv::Mat, uint64_t> item;

            {
                std::unique_lock lock(queueMutex);
                queueCondVar.wait(lock, [this] { return !frameQueue.empty() || !isRunning; });

                if (!isRunning && frameQueue.empty())
                    break;

                item = frameQueue.front();
                frameQueue.pop();
            }

            cv::Mat frame = item.first;
            uint64_t clientId = item.second;

            uint64_t frameNumber = nextFrameNumber[clientId]++;

            std::cout << "Processing frame for client ID: " << clientId << " with frame number: " << frameNumber
                      << std::endl;

            auto startTime = std::chrono::high_resolution_clock::now();
            detector.detect(frame);
            auto endTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> diff = endTime - startTime;
            std::cout << "Frame processing time: " << diff.count() << " seconds" << std::endl;

            std::cout << "Frame processed for client ID: " << clientId << std::endl;

            std::vector<uchar> buf;
            if (!imencode(".jpg", frame, buf)) {
                std::cerr << "Failed to encode image for client ID: " << clientId << std::endl;
                continue;
            }

            {
                const auto encodedImage = std::make_shared<std::string>(buf.begin(), buf.end());
                std::lock_guard lock(clientsMutex);
                frameBuffer[clientId][frameNumber] = encodedImage;
            }

            loop->defer([this, clientId, frameNumber, loop]() {
                std::lock_guard lock(clientsMutex);

                const auto it = clients.find(clientId);
                if (it == clients.end()) {
                    std::cerr << "Client disconnected, unable to send frame for client ID: " << clientId << std::endl;
                    return;
                }

                auto *ws = it->second;

                // Check and send frames in order
                while (lastSentFrame[clientId] + 1 == frameNumber) {
                    auto &frameQueue = frameBuffer[clientId];
                    const auto it = frameQueue.find(lastSentFrame[clientId] + 1);
                    if (it != frameQueue.end()) {
                        // Send the frame
                        ws->send(*it->second, uWS::OpCode::BINARY);
                        std::cout << "Sent frame " << it->first << " to client ID: " << clientId << std::endl;
                        lastSentFrame[clientId]++;
                        frameQueue.erase(it);
                    } else {
                        break;
                    }
                }
            });
        }
    } catch (const std::exception &e) {
        std::cerr << "Exception in worker thread: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception in worker thread." << std::endl;
    }
}
