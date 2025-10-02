#include "FaceProcessor.h"

FaceProcessor::FaceProcessor(const std::string& cascadePath, const std::string& lockscreenPath)
    : lockscreenPath(lockscreenPath) {
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

int FaceProcessor::run_from_camera() {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Nepodarilo se otevrit kameru!\n";
        return EXIT_FAILURE;
    }

    cv::Mat frame;
    cv::Mat lockscreen = cv::imread(lockscreenPath);
    if (lockscreen.empty()) {
        std::cerr << "Nepodarilo se nacist lockscreen obrazek!\n";
        return EXIT_FAILURE;
    }

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        cv::Point2f center = detect_face(frame);

        if (center.x >= 0.0f && center.y >= 0.0f) {
            // oblicej nalezen → zobrazíme kameru
            cv::Mat scene_cross;
            frame.copyTo(scene_cross);
            CrossDrawer::draw_cross_normalized(scene_cross, center, 30);
            cv::imshow("Face Detection", scene_cross);
        }
        else {
            // zadny oblicej → zobrazime lockscreen
            cv::imshow("Face Detection", lockscreen);
        }

        int key = cv::waitKey(30);
        if (key == 27) break; // ESC -> konec
    }

    cap.release();
    return EXIT_SUCCESS;
}
