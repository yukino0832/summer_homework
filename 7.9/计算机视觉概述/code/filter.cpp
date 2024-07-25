#include <opencv2/opencv.hpp>

int main(int argc, char* argv[]) {
    cv::Mat src = cv::imread(argv[1]);
    cv::Mat dst, dst_x, dst_y;
    cv::Mat kernel_x, kernel_y;
    cv::Mat gray;

    kernel_x = (cv::Mat_<float>(3, 3) << -3, 0, 3, -10, 0, 10, -3, 0, 3);
    kernel_y = (cv::Mat_<float>(3, 3) << -3, -10, -3, 0, 0, 0, 3, 10, 3);

    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    cv::filter2D(gray, dst_x, -1, kernel_x);
    cv::filter2D(gray, dst_y, -1, kernel_y);

    dst = cv::abs(dst_x) + cv::abs(dst_y);

    cv::namedWindow("src", cv::WINDOW_NORMAL);
    cv::namedWindow("dst", cv::WINDOW_NORMAL);
    cv::imshow("src", src);
    cv::imshow("dst", dst);
    
    cv::waitKey(0);
    return 0;
}