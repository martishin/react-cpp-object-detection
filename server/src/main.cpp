#include "detector.h"
#include "ws_server.h"

int main() {
    const std::string modelConfiguration = "models/yolo-tiny/yolov3-tiny.cfg";
    const std::string modelWeights = "models/yolo-tiny/yolov3-tiny.weights";
    const std::string classesFile = "models/yolo-tiny/coco.names";

    constexpr int port = 8080;
    WebSocketServer server(port, modelConfiguration, modelWeights, classesFile);
    server.run();

    return 0;
}
