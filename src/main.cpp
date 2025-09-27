#include "ImageProcessor.h"
#include <iostream>
#include <chrono>
#include <opencv2/opencv.hpp>

class App {
public:
    App();
    bool init(void);
    int run(void);
    int zarovka(void);
    int hrnekimg(void);
    int hrnekvid(void);
    ~App();
};

App::App() {}

bool App::init() {
    return true;
}

int App::run(void) {
    std::cout << "===== MENU =====\n";
    std::cout << "1. Najdi stred zarovky\n";
    std::cout << "2. Separace cerveneho hrnku z obrazku\n";
    std::cout << "3. Separace cerveneho hrnku z videa\n";
    std::cout << "Zadej volbu: ";

    int choice;
    std::cin >> choice;

    switch (choice) {
    case 1:
        return zarovka();
    case 2:
        return hrnekimg();
    case 3:
        return hrnekvid();
    default:
        std::cerr << "Neplatná volba!\n";
        return EXIT_FAILURE;
    }
}

int App::zarovka(void) {
    try {
        cv::Mat frame = cv::imread("./resources/lightbulb.jpg");
        if (frame.empty())
            throw std::runtime_error("Empty file? Wrong path?");

        auto start = std::chrono::steady_clock::now();

        cv::Mat frame2;
        cv::Point2f center_normalized = ImageProcessor::find_object_luma(frame, frame2, 230);

        std::cout << "Center normalized: " << center_normalized << '\n';

        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time: " << elapsed_seconds.count() << " sec" << std::endl;

        // převod zpět na absolutní souřadnice pro kreslení
        cv::Point center_absolute(center_normalized.x * frame.cols,
            center_normalized.y * frame.rows);

        ImageProcessor::draw_cross(frame, center_absolute.x, center_absolute.y, 25);
        ImageProcessor::draw_cross_normalized(frame2, center_normalized, 25);

        cv::imshow("frame", frame);
        cv::imshow("frame2", frame2);

        while (true) {
            int key = cv::pollKey();
            if (key == 27) break;
        }
    }
    catch (std::exception const& e) {
        std::cerr << "App failed : " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int App::hrnekimg(void) {
    cv::Mat img = cv::imread("./resources/red_cup.jpg");
    if (img.empty()) return EXIT_FAILURE;

    cv::Mat mask;
    cv::Point2f centroid = ImageProcessor::detect_red_object(
        img, mask,
        cv::Scalar(175, 115, 115), cv::Scalar(180, 255, 255)
    );

    cv::imshow("Original", img);
    cv::imshow("Maska", mask);
    cv::waitKey(0);
    return EXIT_SUCCESS;
}

int App::hrnekvid(void) {
    ImageProcessor::detect_red_object_video(
        "./resources/video.mkv",
        cv::Scalar(175, 115, 115), cv::Scalar(180, 255, 255)
    );
    return EXIT_SUCCESS;
}



App::~App() {
    cv::destroyAllWindows();
    std::cout << "Bye...\n";
}

App app;

int main() {
    if (app.init())
        return app.run();
}
