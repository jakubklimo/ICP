#include "ImageProcessor.h"
#include <iostream>
#include <chrono>

class App {
public:
    App();
    bool init(void);
    int run(void);
    int zarovka(void);
    int runOption2(void);
    int runOption3(void);
    ~App();
};

App::App() {}

bool App::init() {
    return true;
}

int App::run(void) {
    std::cout << "===== MENU =====\n";
    std::cout << "1. Najdi stred zarovky\n";
    std::cout << "2. zadani\n";
    std::cout << "3. zadani\n";
    std::cout << "Zadej volbu: ";

    int choice;
    std::cin >> choice;

    switch (choice) {
    case 1:
        return zarovka();
    case 2:
        return runOption2();
    case 3:
        return runOption3();
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


int App::runOption2(void) {
    std::cout << "Moznost 2\n";
    return EXIT_SUCCESS;
}

int App::runOption3(void) {
    std::cout << "Moznost 3\n";
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
