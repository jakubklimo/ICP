#pragma once
#include <opencv2/opencv.hpp>
#include "CrossDrawer.h"
#include "fps_meter.h"
#include <atomic>
#include <thread>
#include <mutex>
#include <memory>

class FaceProcessor {
public:
    FaceProcessor(const std::string& cascadePath,
        const std::string& lockscreenPath,
        const std::string& warningPath);

    int run_from_camera(FPSMeter* fps = nullptr);
    int run_from_camera_plus(FPSMeter* fps);
    int run_from_camera_plus_FPS(FPSMeter* fps);

    // Nové metody pro spuštìní/zastavení a kontrolu detekce na pozadí
    int startBackgroundDetection(FPSMeter* fps = nullptr);
    bool isFaceDetected();
    int stopBackgroundDetection();

    cv::Point2f detect_face(const cv::Mat& frame);

private:
    cv::CascadeClassifier face_cascade;
    std::string lockscreenPath;
    std::string warningPath;

    // --- Èlenové pro asynchronní detekci na pozadí ---
    std::unique_ptr<cv::VideoCapture> cap;
    std::atomic<bool> terminate_requested{ false };
    std::atomic<float> result_x{ -1.0f };
    std::atomic<float> result_y{ -1.0f };
    std::atomic<int> face_count{ 0 };

    std::thread capture_thread;
    std::thread tracker_thread;

    cv::Mat shared_frame;
    std::mutex frame_mutex;
    std::atomic<bool> frame_ready{ false };

    // Vnitøní metody pro spouštìní ve vláknech
    void capture_loop();
    void tracker_loop();
};