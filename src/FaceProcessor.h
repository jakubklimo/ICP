#pragma once
#include <opencv2/opencv.hpp>
#include "CrossDrawer.h"
#include "fps_meter.h"

class FaceProcessor {
public:
    FaceProcessor(const std::string& cascadePath,
        const std::string& lockscreenPath,
        const std::string& warningPath);

    int run_from_camera(FPSMeter* fps = nullptr);
    int run_from_camera_plus(FPSMeter* fps);
    int run_from_camera_plus_FPS(FPSMeter* fps);



    cv::Point2f detect_face(const cv::Mat& frame);

private:
    cv::CascadeClassifier face_cascade;
    std::string lockscreenPath;
    std::string warningPath;
};
