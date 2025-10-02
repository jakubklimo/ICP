#pragma once
#include <opencv2/opencv.hpp>
#include "CrossDrawer.h"
#include "fps_meter.h"  // pøidáno pro FPS

class FaceProcessor {
public:
    FaceProcessor(const std::string& cascadePath, const std::string& lockscreenPath);

    // hlavní metoda: spustí kameru a hledá oblièej
    int run_from_camera(FPSMeter* fps = nullptr);
    // metoda pro cv3 (lockscreen + detekce objektu + více oblièejù + hrnek)
    int run_from_camera_plus(FPSMeter* fps = nullptr);

private:
    cv::CascadeClassifier face_cascade;
    std::string lockscreenPath;
    cv::Point2f detect_face(const cv::Mat& frame);
};
