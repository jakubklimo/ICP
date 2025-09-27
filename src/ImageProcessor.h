#pragma once
#include <opencv2/opencv.hpp>
#include <iostream>
#include <algorithm>

class ImageProcessor {
public:
    ImageProcessor() = default;

    // najde st�ed pomoc� Y kan�lu (YCbCr), vr�t� normalizovan� sou�adnice
    static cv::Point2f find_object_luma(const cv::Mat& input, cv::Mat& output, int threshold = 230);

    // nakresl� k�� v absolutn�ch sou�adnic�ch
    static void draw_cross(cv::Mat& img, int x, int y, int size);

    // nakresl� k�� v normalizovan�ch sou�adnic�ch (0..1)
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
