#pragma once
#include <opencv2/opencv.hpp>
#include <iostream>
#include <algorithm>

class ImageProcessor {
public:
    ImageProcessor() = default;

    // najde støed pomocí Y kanálu (YCbCr), vrátí normalizované souøadnice
    static cv::Point2f find_object_luma(const cv::Mat& input, cv::Mat& output, int threshold = 230);

    // nakreslí køíž v absolutních souøadnicích
    static void draw_cross(cv::Mat& img, int x, int y, int size);

    // nakreslí køíž v normalizovaných souøadnicích (0..1)
    static void draw_cross_normalized(cv::Mat& img, cv::Point2f center_normalized, int size);

    // detekce cerveneho objektu z obrazku
    static cv::Point2f detect_red_object(const cv::Mat& img, cv::Mat& mask,
        cv::Scalar lower, cv::Scalar upper,
        bool useMorph = true);

    // detekce cerveneho objektu z videa
    static void detect_red_object_video(const std::string& videoPath,
        cv::Scalar lower, cv::Scalar upper,
        bool useMorph = true);
};
