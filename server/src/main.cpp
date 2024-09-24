#include "detector.h"
#include "ws_server.h"

int main() {
    // Load YOLO model configuration
    std::string modelConfiguration = "models/yolo-tiny/yolov3-tiny.cfg";
    std::string modelWeights = "models/yolo-tiny/yolov3-tiny.weights";
    std::string classesFile = "models/yolo-tiny/coco.names";

    // Start the WebSocket server
    int port = 8080;
    WebSocketServer server(port, modelConfiguration, modelWeights, classesFile);
    server.run();

    return 0;
}
