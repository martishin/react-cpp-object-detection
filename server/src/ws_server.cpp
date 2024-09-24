#include "ws_server.h"
#include <iostream>
#include <opencv2/opencv.hpp>

WebSocketServer::WebSocketServer(int port, Detector &detector) : port(port), detector(detector)
{
    // No need to start the processing thread
}

WebSocketServer::~WebSocketServer()
{
    // No need to join any threads
}

void WebSocketServer::run()
{
    uWS::App()
        .ws<PerSocketData>("/*",
                           {.compression = uWS::SHARED_COMPRESSOR,
                            .maxPayloadLength = 50 * 1024 * 1024, // 50 MB
                            .idleTimeout = 10,
                            .open = [](auto *ws) { std::cout << "Client connected" << std::endl; },
                            .message =
                                [this](auto *ws, std::string_view message, uWS::OpCode opCode)
                            {
                                std::cout << "Received a message with opcode: " << static_cast<int>(opCode)
                                          << ", message size: " << message.size() << std::endl;

                                // Expecting binary data (image)
                                if (opCode == uWS::OpCode::BINARY)
                                {
                                    std::cout << "Processing binary message" << std::endl;

                                    try
                                    {
                                        // Decode the image
                                        std::vector<uchar> data(message.begin(), message.end());
                                        cv::Mat img = cv::imdecode(data, cv::IMREAD_COLOR);
                                        if (img.empty())
                                        {
                                            std::cerr << "Failed to decode image" << std::endl;
                                            return;
                                        }

                                        std::cout << "Image decoded successfully" << std::endl;

                                        // Process the frame using the detector
                                        detector.detect(img);

                                        std::cout << "Image processed by detector" << std::endl;

                                        // Encode the processed image to JPEG
                                        std::vector<uchar> buf;
                                        cv::imencode(".jpg", img, buf);
                                        std::string encodedImage(buf.begin(), buf.end());

                                        std::cout << "Image encoded successfully, sending back to client" << std::endl;

                                        // Send back the processed image
                                        ws->send(encodedImage, uWS::OpCode::BINARY);
                                    }
                                    catch (const std::exception &e)
                                    {
                                        std::cerr << "Exception during message processing: " << e.what() << std::endl;
                                    }
                                }
                                else
                                {
                                    std::cerr << "Received non-binary message with opcode: " << static_cast<int>(opCode)
                                              << std::endl;
                                }
                            },
                            .close = [](auto *ws, int, std::string_view)
                            { std::cout << "Client disconnected" << std::endl; }})
        .listen(port,
                [this](auto *listenSocket)
                {
                    if (listenSocket)
                    {
                        std::cout << "Server listening on port " << port << std::endl;
                    }
                    else
                    {
                        std::cerr << "Failed to listen on port " << port << std::endl;
                    }
                })
        .run();
}

// void WebSocketServer::processImages()
// {
//     while (isRunning)
//     {
//         std::unique_lock<std::mutex> lock(queueMutex);
//         queueCondVar.wait(lock, [this]() -> bool { return !messageQueue.empty() || !isRunning; });
//
//         if (!isRunning && messageQueue.empty())
//             break;
//
//         auto item = messageQueue.front();
//         messageQueue.pop();
//         lock.unlock();
//
//         cv::Mat frame = item.first;
//         uWS::WebSocket<false, true, PerSocketData> *ws = item.second;
//
//         // Process the frame using the detector
//         detector.detect(frame);
//
//         // Encode the processed image to JPEG
//         std::vector<uchar> buf;
//         cv::imencode(".jpg", frame, buf);
//         std::string encodedImage(buf.begin(), buf.end());
//
//         // Send back the processed image
//         ws->send(encodedImage, uWS::OpCode::BINARY);
//     }
// }
