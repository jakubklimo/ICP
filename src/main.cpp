#include "ImageProcessor.h"

#include "FaceProcessor.h"

#include "fps_meter.h"

#include "Renderer3D.h"

#include "gl_err_callback.h"

#include <iostream>

#include <chrono>

#include <opencv2/opencv.hpp>

#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include "ShaderLoaderApp.h"



// Globální FPS měřič pro celý program

FPSMeter gFPS;



class App {

public:

    App();

    bool init(void);

    int run(void);

    int zarovka(void);

    int hrnekimg(void);

    int hrnekvid(void);

    int face_video(void);

    int face_video_plus(void);

    int face_video_plus_FPS(void);

    void init_gl_debug();


    ~App();

};



App::App() {}


bool init_glfw() {
    if (!glfwInit()) {
        std::cerr << "GLFW init failed!\n";
        return false;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    return true;
}

bool init_glew() {
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "GLEW init failed: " << glewGetErrorString(err) << '\n';
        return false;
    }
    return true;
}

bool App::init() {
    std::cout << "Inicializace GLFW..." << std::endl;
    if (!init_glfw()) {
        std::cerr << "GLFW initialization failed!\n";
        return false;
    }


    init_gl_debug();

    gFPS.reset();
    return true;
}






int App::run(void) {

    std::cout << "===== MENU =====\n";

    std::cout << "1. Najdi stred zarovky\n";

    std::cout << "2. Separace cerveneho hrnku z obrazku\n";

    std::cout << "3. Separace cerveneho hrnku z videa\n";

    std::cout << "4. Oblicej z videa\n";

    std::cout << "5. Kamera PLUS (oblicej + hrnek)\n";

    std::cout << "6. FPS Boost App\n";

    std::cout << "7. OpenGL 3D Renderer (triangle)\n";

    std::cout << "8. Shader Loader Demo\n";

    std::cout << "Zadej volbu: ";



    int choice;
    std::cin >> choice;

    switch (choice) {
    case 1: return zarovka();
    case 2: return hrnekimg();
    case 3: return hrnekvid();
    case 4: return face_video();
    case 5: return face_video_plus();
    case 6: return face_video_plus_FPS();
    case 7: {
        Renderer3D renderer;
        if (renderer.init()) renderer.run();
        else std::cerr << "Renderer3D initialization failed!\n";
        return EXIT_SUCCESS;
    }
    case 8: {
        ShaderLoaderApp shaderApp;
        if (shaderApp.init()) shaderApp.run();
        else std::cerr << "ShaderLoaderApp initialization failed!\n";
        return EXIT_SUCCESS;
    }

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



        cv::Point center_absolute(center_normalized.x * frame.cols,

            center_normalized.y * frame.rows);



        ImageProcessor::draw_cross(frame, center_absolute.x, center_absolute.y, 25);

        ImageProcessor::draw_cross_normalized(frame2, center_normalized, 25);



        cv::imshow("frame", frame);

        cv::imshow("frame2", frame2);



        while (true) {

            gFPS.update();

            if (gFPS.is_updated())

                std::cout << "FPS: " << gFPS.get() << std::endl;



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



int App::face_video(void) {

    try {

        FaceProcessor faceProc(

            "./resources/haarcascade_frontalface_default.xml",

            "./resources/lockscreen.jpg",

            "./resources/warning.jpg"

        );



        return faceProc.run_from_camera(&gFPS);

    }

    catch (std::exception const& e) {

        std::cerr << "Face video failed: " << e.what() << std::endl;

        return EXIT_FAILURE;

    }

}



int App::face_video_plus(void) {
    try {
        FaceProcessor faceProc(
            "./resources/haarcascade_frontalface_default.xml",
            "./resources/lockscreen.jpg",
            "./resources/warning.jpg"
        );

        return faceProc.run_from_camera_plus(&gFPS);
    }
    catch (std::exception const& e) {
        std::cerr << "Face video plus failed: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}

int App::face_video_plus_FPS(void) {
    try {
        FaceProcessor faceProc(
            "./resources/haarcascade_frontalface_default.xml",
            "./resources/lockscreen.jpg",
            "./resources/warning.jpg"
        );

        return faceProc.run_from_camera_plus_FPS(&gFPS);
    }
    catch (std::exception const& e) {
        std::cerr << "Face video plus failed: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
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



    while (true) {

        gFPS.update();

        if (gFPS.is_updated())

            std::cout << "FPS: " << gFPS.get() << std::endl;



        int key = cv::waitKey(30);

        if (key == 27) break;

    }



    return EXIT_SUCCESS;

}



int App::hrnekvid(void) {

    ImageProcessor::detect_red_object_video(

        "./resources/video.mkv",

        cv::Scalar(175, 115, 115), cv::Scalar(180, 255, 255), true,

        &gFPS

    );

    return EXIT_SUCCESS;

}

void App::init_gl_debug()
{
    if (GLEW_ARB_debug_output)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(MessageCallback, nullptr);

        GLuint unusedIds = 0;
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE,
            GL_DEBUG_SEVERITY_NOTIFICATION, 0, &unusedIds, GL_FALSE);

        std::cout << "OpenGL debug output enabled.\n";
    }
    else
    {
        std::cout << "OpenGL debug output NOT supported.\n";
    }
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