#pragma once
#include <opencv2/opencv.hpp>
#include <atomic>
#include <thread>
#include <mutex>
#include "CrossDrawer.h"
#include "fps_meter.h"

class FaceProcessor {
public:
    FaceProcessor(const std::string& cascadePath,
        const std::string& lockscreenPath,
        const std::string& warningPath);
    ~FaceProcessor();

    int run_from_camera(FPSMeter* fps = nullptr);
    int run_from_camera_plus(FPSMeter* fps);
    int run_from_camera_plus_FPS(FPSMeter* fps);

    cv::Point2f detect_face(const cv::Mat& frame);

    void startBackgroundDetection();
    void stopBackgroundDetection();
    bool isFaceDetected() const;

private:
    cv::CascadeClassifier face_cascade;
    std::string lockscreenPath;
    std::string warningPath;

    std::atomic<bool> _faceDetected{ false };
    std::atomic<bool> _running{ false };
    std::thread _workerThread;

    void backgroundLoop();
};