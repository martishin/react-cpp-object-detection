#include "detector.h"
#include <fstream>
#include <iostream>

Detector::Detector(const std::string &modelConfiguration, const std::string &modelWeights,
                   const std::string &classesFile, float confThreshold, float nmsThreshold) :
    confThreshold(confThreshold), nmsThreshold(nmsThreshold) {
    // Load names of classes
    std::ifstream ifs(classesFile.c_str());
    if (!ifs.is_open()) {
        std::cerr << "Error opening classes file: " << classesFile << std::endl;
        return;
    }
    std::string line;
    while (std::getline(ifs, line)) {
        classes.push_back(line);
    }

    // Load the network
    net = cv::dnn::readNetFromDarknet(modelConfiguration, modelWeights);
    if (net.empty()) {
        std::cerr << "Error loading network. Check model files." << std::endl;
        return;
    }
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    // You can set the preferable target to DNN_TARGET_CPU or DNN_TARGET_CUDA
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

    if (net.empty()) {
        std::cerr << "Error loading network. Check model files." << std::endl;
        return;
    }

    valid = true; // Set valid to true if network loaded successfully
}

bool Detector::isValid() const { return valid; }

void Detector::detect(cv::Mat &frame) {
    if (frame.empty()) {
        std::cerr << "Empty frame provided for detection." << std::endl;
        return;
    }

    // Create a blob from the frame
    cv::Mat blob;
    cv::dnn::blobFromImage(frame, blob, 1 / 255.0, cv::Size(416, 416), cv::Scalar(0, 0, 0), true, false);

    // Set the input to the network
    net.setInput(blob);

    // Run forward pass
    std::vector<cv::Mat> outs;
    net.forward(outs, net.getUnconnectedOutLayersNames());

    // Remove the bounding boxes with low confidence
    std::vector<int> classIds;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    for (const auto &out : outs) {
        const float *data = reinterpret_cast<const float *>(out.data);
        for (int j = 0; j < out.rows; ++j, data += out.cols) {
            cv::Mat scores = out.row(j).colRange(5, out.cols);
            cv::Point classIdPoint;
            double confidence;

            cv::minMaxLoc(scores, nullptr, &confidence, nullptr, &classIdPoint);
            if (confidence > confThreshold) {
                int centerX = static_cast<int>(data[0] * frame.cols);
                int centerY = static_cast<int>(data[1] * frame.rows);
                int width = static_cast<int>(data[2] * frame.cols);
                int height = static_cast<int>(data[3] * frame.rows);
                int left = centerX - width / 2;
                int top = centerY - height / 2;

                classIds.push_back(classIdPoint.x);
                confidences.push_back(static_cast<float>(confidence));
                boxes.emplace_back(left, top, width, height);
            }
        }
    }

    // Perform non-maximum suppression to eliminate redundant overlapping boxes
    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);

    for (int idx : indices) {
        cv::Rect box = boxes[idx];

        // Ensure the box is within the image boundaries
        box &= cv::Rect(0, 0, frame.cols, frame.rows);

        // Draw the bounding box
        cv::rectangle(frame, box, cv::Scalar(0, 255, 0), 2);

        // Get the label for the class name and its confidence
        std::string label = cv::format("%.2f", confidences[idx]);
        if (!classes.empty() && classIds[idx] < static_cast<int>(classes.size())) {
            label = classes[classIds[idx]] + ": " + label;
        }

        // Display the label at the top of the bounding box
        int baseLine = 0;
        cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
        int topLabel = std::max(box.y, labelSize.height);
        cv::rectangle(frame, cv::Point(box.x, topLabel - labelSize.height),
                      cv::Point(box.x + labelSize.width, topLabel + baseLine), cv::Scalar::all(255), cv::FILLED);
        cv::putText(frame, label, cv::Point(box.x, topLabel), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);
    }
}
