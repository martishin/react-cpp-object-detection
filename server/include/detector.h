#ifndef DETECTOR_H
#define DETECTOR_H

#include <opencv2/dnn.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

class Detector {
public:
    Detector(const std::string &modelConfiguration, const std::string &modelWeights, const std::string &classesFile,
             float confThreshold = 0.5f, float nmsThreshold = 0.4f);

    void detect(cv::Mat &frame);
    bool isValid() const;

private:
    cv::dnn::Net net;
    std::vector<std::string> classes;
    float confThreshold;
    float nmsThreshold;
    bool valid; // Add this member variable
};

#endif // DETECTOR_H
