#include <opencv2/opencv.hpp>

int main(int argc, char* argv[]) {
    cv::Mat src = cv::imread(argv[1]);
    cv::Mat dst;
    cv::Mat gray;

    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY); // 转换为灰度图
    cv::Canny(gray, dst, 50, 150); // Canny 边缘检测

    cv::namedWindow("src", cv::WINDOW_NORMAL);
    cv::namedWindow("dst", cv::WINDOW_NORMAL);
    cv::imshow("src", src);
    cv::imshow("dst", dst);

    cv::waitKey(0);
    return 0;
}