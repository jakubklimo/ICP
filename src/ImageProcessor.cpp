#include "ImageProcessor.h"

cv::Point2f ImageProcessor::find_object_luma(const cv::Mat& input, cv::Mat& output, int threshold) {
    input.copyTo(output);

    double sumX = 0, sumY = 0;
    int count_white = 0;

    for (int y = 0; y < input.rows; y++) {
        for (int x = 0; x < input.cols; x++) {
            cv::Vec3b pixel = input.at<cv::Vec3b>(y, x);

            unsigned char R = pixel[2];
            unsigned char G = pixel[1];
            unsigned char B = pixel[0];

            // pøevod na Y podle JPEG (8bit)
            unsigned char Y = static_cast<unsigned char>(
                0.299 * R
                + 0.587 * G
                + 0.114 * B);

            if (Y >= threshold) {
                output.at<cv::Vec3b>(y, x) = cv::Vec3b(255, 255, 255);
                sumX += x;
                sumY += y;
                count_white++;
            }
            else {
                output.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, 0);
            }
        }
    }

    cv::Point2f center_normalized(0.5f, 0.5f); // defaultnì støed obrazu
    if (count_white > 0) {
        float cx = static_cast<float>(sumX / count_white);
        float cy = static_cast<float>(sumY / count_white);

        center_normalized.x = cx / input.cols;
        center_normalized.y = cy / input.rows;
    }
    return center_normalized;
}

void ImageProcessor::draw_cross(cv::Mat& img, int x, int y, int size) {
    cv::Point p1(x - size / 2, y);
    cv::Point p2(x + size / 2, y);
    cv::Point p3(x, y - size / 2);
    cv::Point p4(x, y + size / 2);

    cv::line(img, p1, p2, CV_RGB(255, 0, 0), 3);
    cv::line(img, p3, p4, CV_RGB(255, 0, 0), 3);
}

void ImageProcessor::draw_cross_normalized(cv::Mat& img, cv::Point2f center_normalized, int size) {
    center_normalized.x = std::clamp(center_normalized.x, 0.0f, 1.0f);
    center_normalized.y = std::clamp(center_normalized.y, 0.0f, 1.0f);
    size = std::clamp(size, 1, std::min(img.cols, img.rows));

    cv::Point2f center_absolute(center_normalized.x * img.cols, center_normalized.y * img.rows);

    cv::Point2f p1(center_absolute.x - size / 2, center_absolute.y);
    cv::Point2f p2(center_absolute.x + size / 2, center_absolute.y);
    cv::Point2f p3(center_absolute.x, center_absolute.y - size / 2);
    cv::Point2f p4(center_absolute.x, center_absolute.y + size / 2);

    cv::line(img, p1, p2, CV_RGB(255, 0, 0), 3);
    cv::line(img, p3, p4, CV_RGB(255, 0, 0), 3);
}
