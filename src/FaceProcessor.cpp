#include "FaceProcessor.h"
#include "ImageProcessor.h"
#include <iostream>
#include <chrono>

FaceProcessor::FaceProcessor(const std::string& cascadePath,
    const std::string& lockscreenPath,
    const std::string& warningPath)
    : lockscreenPath(lockscreenPath), warningPath(warningPath)
{
    if (!face_cascade.load(cascadePath)) {
        throw std::runtime_error("Failed to load Haar cascade: " + cascadePath);
    }
}

FaceProcessor::~FaceProcessor() {
    stopBackgroundDetection();
}

void FaceProcessor::startBackgroundDetection() {
    if (_running) return;
    _running = true;
    _workerThread = std::thread(&FaceProcessor::backgroundLoop, this);
}

void FaceProcessor::stopBackgroundDetection() {
    _running = false;
    if (_workerThread.joinable()) {
        _workerThread.join();
    }
}

bool FaceProcessor::isFaceDetected() const {
    return _faceDetected.load();
}

void FaceProcessor::backgroundLoop() {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        _running = false;
        return;
    }

    cap.set(cv::CAP_PROP_FRAME_WIDTH, 320);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 240);

    cv::Mat frame, gray;
    std::vector<cv::Rect> faces;

    while (_running) {
        cap >> frame;
        if (frame.empty()) continue;

        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(gray, gray);

        face_cascade.detectMultiScale(gray, faces, 1.3, 3, 0, cv::Size(30, 30));

        if (!faces.empty()) {
            _faceDetected.store(true);
        }
        else {
            _faceDetected.store(false);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    cap.release();
}

int FaceProcessor::run_from_camera_plus_FPS(FPSMeter* fps) {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) return EXIT_FAILURE;

    cv::Mat lockscreen = cv::imread(lockscreenPath);
    cv::Mat warning = cv::imread(warningPath);
    if (lockscreen.empty() || warning.empty()) return EXIT_FAILURE;

    std::atomic<bool> terminate_requested{ false };
    std::atomic<float> result_x{ -1.0f };
    std::atomic<float> result_y{ -1.0f };
    std::atomic<int> face_count{ 0 };

    cv::Mat shared_frame;
    std::mutex frame_mutex;
    std::atomic<bool> frame_ready{ false };

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

            cv::Mat gray;
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
            cv::equalizeHist(gray, gray);

            std::vector<cv::Rect> faces;
            face_cascade.detectMultiScale(gray, faces, 1.2, 3, 0, cv::Size(40, 40));
            face_count = static_cast<int>(faces.size());

            if (faces.size() == 1) {
                cv::Mat mask;
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

        cv::imshow("Face+Cup Detection", scene);

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

std::vector<uchar> lossy_quality_limit_fp(cv::Mat& input_img, float target_quality) {
    std::vector<uchar> buf;
    int low = 1, high = 100, best_quality = 100;

    while (low <= high) {
        int q = (low + high) / 2;
        std::vector<int> params = { cv::IMWRITE_JPEG_QUALITY, q };
        cv::imencode(".jpg", input_img, buf, params);

        cv::Mat decoded = cv::imdecode(buf, cv::IMREAD_COLOR);
        double metric = cv::PSNR(input_img, decoded);

        if (metric >= target_quality) {
            best_quality = q;
            high = q - 1;
        }
        else {
            low = q + 1;
        }
    }

    std::vector<int> params = { cv::IMWRITE_JPEG_QUALITY, best_quality };
    cv::imencode(".jpg", input_img, buf, params);
    return buf;
}

int FaceProcessor::run_from_camera_plus(FPSMeter* fps) {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) return EXIT_FAILURE;

    cap.set(cv::CAP_PROP_FPS, 60);
    cap.set(cv::CAP_PROP_BUFFERSIZE, 1);

    cv::Mat lockscreen = cv::imread(this->lockscreenPath);
    cv::Mat warning = cv::imread(this->warningPath);

    std::atomic<bool> terminate{ false };
    std::atomic<int> face_count{ 0 };
    std::atomic<float> result_x{ -1 }, result_y{ -1 };

    cv::Mat shared_raw_frame;
    cv::Mat shared_display_frame;
    std::mutex raw_mutex;
    std::mutex display_mutex;
    std::condition_variable frame_ready;

    std::thread grabber([&]() {
        cv::Mat frame;
        while (!terminate) {
            cap >> frame;
            if (frame.empty()) continue;

            {
                std::scoped_lock lock(raw_mutex);
                frame.copyTo(shared_raw_frame);
            }
            frame_ready.notify_all();
        }
        });

    std::thread face_detector([&]() {
        cv::Mat local_frame;
        while (!terminate) {
            {
                std::unique_lock<std::mutex> lock(raw_mutex);
                frame_ready.wait(lock, [&] { return !shared_raw_frame.empty() || terminate.load(); });
                shared_raw_frame.copyTo(local_frame);
            }

            cv::Mat gray;
            cv::cvtColor(local_frame, gray, cv::COLOR_BGR2GRAY);
            cv::equalizeHist(gray, gray);

            std::vector<cv::Rect> faces;
            face_cascade.detectMultiScale(gray, faces, 1.2, 2, 0, cv::Size(50, 50));
            face_count = faces.size();
        }
        });

    std::thread cup_detector([&]() {
        cv::Mat local_frame;
        while (!terminate) {
            if (face_count.load() != 1) {
                result_x = result_y = -1;
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                continue;
            }

            {
                std::unique_lock<std::mutex> lock(raw_mutex);
                frame_ready.wait(lock, [&] { return !shared_raw_frame.empty() || terminate.load(); });
                shared_raw_frame.copyTo(local_frame);
            }

            cv::Mat mask;
            cv::Point2f cupCenter = ImageProcessor::detect_red_object(
                local_frame, mask,
                cv::Scalar(175, 115, 115),
                cv::Scalar(180, 255, 255)
            );
            result_x = cupCenter.x;
            result_y = cupCenter.y;
        }
        });

    std::thread compressor([&]() {
        cv::Mat local_frame;
        while (!terminate) {
            {
                std::unique_lock<std::mutex> lock(raw_mutex);
                frame_ready.wait(lock, [&] { return !shared_raw_frame.empty() || terminate.load(); });
                shared_raw_frame.copyTo(local_frame);
            }

            std::vector<uchar> compressed = lossy_quality_limit_fp(local_frame, 50.0f);
            cv::Mat decoded = cv::imdecode(compressed, cv::IMREAD_COLOR);

            {
                std::scoped_lock lock(display_mutex);
                decoded.copyTo(shared_display_frame);
            }
        }
        });

    if (fps) fps->reset();
    while (!terminate) {
        cv::Mat scene;
        {
            std::unique_lock<std::mutex> lock(display_mutex, std::try_to_lock);
            if (lock.owns_lock() && !shared_display_frame.empty())
                shared_display_frame.copyTo(scene);
            else
                lockscreen.copyTo(scene);
        }

        int count = face_count.load();
        float x = result_x.load(), y = result_y.load();

        if (count == 0) {
            scene = lockscreen.clone();
        }
        else if (count == 1 && x >= 0 && y >= 0) {
            CrossDrawer::draw_cross_normalized(scene, cv::Point2f(x, y), 30);
        }
        else if (count > 1) {
            scene = warning.clone();
        }

        cv::imshow("Face+Cup Compressed", scene);

        if (fps) {
            fps->update();
            if (fps->is_updated())
                std::cout << "FPS: " << fps->get() << std::endl;
        }

        if (cv::waitKey(16) == 27) terminate = true;
    }

    terminate = true;
    frame_ready.notify_all();
    if (grabber.joinable()) grabber.join();
    if (face_detector.joinable()) face_detector.join();
    if (cup_detector.joinable()) cup_detector.join();
    if (compressor.joinable()) compressor.join();
    cap.release();
    cv::destroyAllWindows();

    return EXIT_SUCCESS;
}

int FaceProcessor::run_from_camera(FPSMeter* fps) {
    return run_from_camera_plus(fps);
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