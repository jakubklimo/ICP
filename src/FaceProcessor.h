#pragma once
#include <opencv2/opencv.hpp>
#include "CrossDrawer.h"
#include "fps_meter.h"  // p�id�no pro FPS

class FaceProcessor {
public:
    FaceProcessor(const std::string& cascadePath, const std::string& lockscreenPath);

    // hlavn� metoda: spust� kameru a hled� obli�ej
    int run_from_camera(FPSMeter* fps = nullptr);
    // metoda pro cv3 (lockscreen + detekce objektu + v�ce obli�ej� + hrnek)
    int run_from_camera_plus(FPSMeter* fps = nullptr);

private:
    cv::CascadeClassifier face_cascade;
    std::string lockscreenPath;
    cv::Point2f detect_face(const cv::Mat& frame);
};
