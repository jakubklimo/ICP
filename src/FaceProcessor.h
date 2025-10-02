#pragma once
#include <opencv2/opencv.hpp>
#include "CrossDrawer.h"

class FaceProcessor {
public:
    FaceProcessor(const std::string& cascadePath, const std::string& lockscreenPath);

    // hlavní metoda: spustí kameru a hledá oblièej
    int run_from_camera();

private:
    cv::CascadeClassifier face_cascade;
    std::string lockscreenPath;
    cv::Point2f detect_face(const cv::Mat& frame);
};
