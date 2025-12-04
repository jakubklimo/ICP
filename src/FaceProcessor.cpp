#include "FaceProcessor.h"
#include "ImageProcessor.h"
#include <iostream>
#include <thread>
#include <chrono>

// Pomocná funkce definovaná v původním kódu
std::vector<uchar> lossy_quality_limit(cv::Mat& input_img, float target_quality) {
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

FaceProcessor::FaceProcessor(const std::string& cascadePath,
    const std::string& lockscreenPath,
    const std::string& warningPath)
    : lockscreenPath(lockscreenPath), warningPath(warningPath)
{
    if (!face_cascade.load(cascadePath)) {
        throw std::runtime_error("Nepodařilo se načíst Haar cascade: " + cascadePath);
    }
}

// Implementace pomocné metody pro čtení z kamery (běží v capture_thread)
void FaceProcessor::capture_loop() {
    cv::Mat local;
    while (!terminate_requested) {
        if (!cap->isOpened()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        *cap >> local;
        if (local.empty()) break;

        {
            std::scoped_lock lock(frame_mutex);
            shared_frame = local.clone();
            frame_ready = true;
        }
    }
}

// Implementace pomocné metody pro detekci (běží v tracker_thread)
void FaceProcessor::tracker_loop() {
    cv::Mat frame;
    while (!terminate_requested) {
        // Kontrola, zda je připravený nový snímek
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
}


// Implementace nově přidaných metod:

// Inicializuje kameru a spouští detekční vlákna
int FaceProcessor::startBackgroundDetection(FPSMeter* fps) {
    if (cap && cap->isOpened()) {
        std::cerr << "Chyba: Detekce na pozadí je již spuštěna.\n";
        return EXIT_FAILURE;
    }

    cap = std::make_unique<cv::VideoCapture>(0);
    if (!cap->isOpened()) {
        std::cerr << "Nepodařilo se otevřít kameru!\n";
        return EXIT_FAILURE;
    }

    // Resetování stavu
    terminate_requested = false;
    result_x = -1.0f;
    result_y = -1.0f;
    face_count = 0;
    frame_ready = false;

    // Spuštění vláken
    capture_thread = std::thread(&FaceProcessor::capture_loop, this);
    tracker_thread = std::thread(&FaceProcessor::tracker_loop, this);

    // Poznámka: FPSMeter zde nepoužíváme, protože se jedná o neblokující API.
    // FPS se budou měřit v hlavním vlákně aplikace (např. ShaderLoaderApp::drawLoop).

    return EXIT_SUCCESS;
}

// Zastaví vlákna a uvolní kameru
int FaceProcessor::stopBackgroundDetection() {
    terminate_requested = true;

    if (capture_thread.joinable()) {
        capture_thread.join();
    }
    if (tracker_thread.joinable()) {
        tracker_thread.join();
    }

    if (cap) {
        cap->release();
        cap.reset();
    }

    return EXIT_SUCCESS;
}

// Vrací aktuální stav detekce
bool FaceProcessor::isFaceDetected() {
    return face_count.load() == 1;
}

// Zjednodušená verze run_from_camera_plus_FPS, která používá nové metody
int FaceProcessor::run_from_camera_plus_FPS(FPSMeter* fps) {
    if (startBackgroundDetection() != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    cv::Mat lockscreen = cv::imread(lockscreenPath);
    cv::Mat warning = cv::imread(warningPath);
    if (lockscreen.empty() || warning.empty()) {
        std::cerr << "Nepodarilo se nacist lockscreen nebo warning obrazek!\n";
        stopBackgroundDetection();
        return EXIT_FAILURE;
    }

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

    return stopBackgroundDetection();
}

// Původní FaceProcessor::run_from_camera_plus, který je nyní zastaralý, ale ponechán pro kompatibilitu
int FaceProcessor::run_from_camera_plus(FPSMeter* fps) {
    // Vzhledem k tomu, že původní run_from_camera_plus a run_from_camera_plus_FPS byly podobné,
    // přesměrujeme run_from_camera_plus na novou metodu, ale zachováme původní kód, pokud ho potřebujete.
    // Zde ponecháme původní kód s refaktoringem:

    // Původní implementace run_from_camera_plus je složitá, protože duplikuje logiku run_from_camera_plus_FPS.
    // Pro zjednodušení a odstranění duplikace, a protože se jedná o neblokující detekci,
    // necháme implementaci stejnou jako run_from_camera_plus_FPS (což bylo chování i v původním kódu):
    return run_from_camera_plus_FPS(fps);
}

// Původní FaceProcessor::run_from_camera
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