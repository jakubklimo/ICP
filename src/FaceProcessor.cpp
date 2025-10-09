#include "FaceProcessor.h"
#include "ImageProcessor.h"
#include <iostream>

FaceProcessor::FaceProcessor(const std::string& cascadePath,
    const std::string& lockscreenPath,
    const std::string& warningPath)
    : lockscreenPath(lockscreenPath), warningPath(warningPath)
{
    if (!face_cascade.load(cascadePath)) {
        throw std::runtime_error("Nepodařilo se načíst Haar cascade: " + cascadePath);
    }
}

cv::Point2f FaceProcessor::detect_face(const cv::Mat& frame) {
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(gray, gray);

    std::vector<cv::Rect> faces;
    face_cascade.detectMultiScale(gray, faces, 1.1, 3, 0, cv::Size(30, 30));

    cv::Point2f center(-1.0f, -1.0f);
    if (!faces.empty()) {
        cv::Rect face = faces[0];
        center.x = static_cast<float>(face.x + face.width / 2) / frame.cols;
        center.y = static_cast<float>(face.y + face.height / 2) / frame.rows;
    }
    return center;
}

int FaceProcessor::run_from_camera_plus(FPSMeter* fps) {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Nepodarilo se otevrit kameru!\n";
        return EXIT_FAILURE;
    }

    cv::Mat lockscreen = cv::imread(lockscreenPath);
    cv::Mat warning = cv::imread(warningPath);
    if (lockscreen.empty() || warning.empty()) {
        std::cerr << "Nepodarilo se nacist lockscreen nebo warning obrazek!\n";
        return EXIT_FAILURE;
    }

    std::atomic<bool> terminate_requested{ false };
    std::atomic<float> result_x{ -1.0f };
    std::atomic<float> result_y{ -1.0f };
    std::atomic<int> face_count{ 0 };

    cv::Mat shared_frame;
    std::mutex frame_mutex;
    std::atomic<bool> frame_ready{ false };

    // Vlákno pro čtení kamery
    std::thread capture_thread([&]() {
        cv::Mat local;
        while (!terminate_requested) {
            cap >> local;
            if (local.empty()) break;
            {
                std::scoped_lock lock(frame_mutex);
                shared_frame = local.clone();
                frame_ready = true;
            }
        }
        });

    // Vlákno pro detekci
    std::thread tracker_thread([&]() {
        cv::Mat frame;
        while (!terminate_requested) {
            if (!frame_ready) {
                std::this_thread::yield();
                continue;
            }

            {
                std::scoped_lock lock(frame_mutex);
                if (shared_frame.empty()) continue;
                frame = shared_frame.clone();
                frame_ready = false;
            }

            // Detekce obličeje
            cv::Mat gray;
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
            cv::equalizeHist(gray, gray);

            std::vector<cv::Rect> faces;
            face_cascade.detectMultiScale(gray, faces, 1.2, 3, 0, cv::Size(40, 40));
            face_count = static_cast<int>(faces.size());

            if (faces.size() == 1) {
                cv::Mat mask; // stále musíme předat mask
                cv::Point2f cupCenter = ImageProcessor::detect_red_object(
                    frame, mask,
                    cv::Scalar(175, 115, 115), cv::Scalar(180, 255, 255)
                );
                result_x = cupCenter.x;
                result_y = cupCenter.y;
            }
            else {
                result_x = result_y = -1.0f;
            }
        }
        });

    // Hlavní smyčka zobrazování
    while (!terminate_requested) {
        cv::Mat frame_copy;
        {
            std::scoped_lock lock(frame_mutex);
            if (!shared_frame.empty())
                frame_copy = shared_frame.clone();
        }

        int count = face_count.load();
        float x = result_x.load();
        float y = result_y.load();

        cv::Mat scene;
        if (count == 0) {
            scene = lockscreen.clone();
        }
        else if (count == 1) {
            scene = frame_copy;
            if (x >= 0.0f && y >= 0.0f)
                CrossDrawer::draw_cross_normalized(scene, cv::Point2f(x, y), 30);
        }
        else {
            scene = warning.clone();
        }

        cv::imshow("Face+Cup Detection (Non-blocking)", scene);

        if (fps) {
            fps->update();
            if (fps->is_updated())
                std::cout << "FPS: " << fps->get() << std::endl;
        }

        int key = cv::waitKey(1);
        if (key == 27) terminate_requested = true;
    }

    if (capture_thread.joinable()) capture_thread.join();
    if (tracker_thread.joinable()) tracker_thread.join();

    cap.release();
    return EXIT_SUCCESS;
}


int FaceProcessor::run_from_camera(FPSMeter* fps) {
    return run_from_camera_plus(fps);
}
