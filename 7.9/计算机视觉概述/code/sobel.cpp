#include <opencv2/opencv.hpp>

int main(int argc, char* argv[]) {
    cv::Mat src = cv::imread(argv[1]);
    cv::Mat dst;
    cv::Mat gray;
    cv::Mat grad_x, grad_y;
    cv::Mat abs_grad_x, abs_grad_y;

    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY); // 转换为灰度图
    cv::Sobel(gray, grad_x, CV_16S, 1, 0, 3); // x 方向梯度
    cv::Sobel(gray, grad_y, CV_16S, 0, 1, 3); // y 方向梯度
    cv::convertScaleAbs(grad_x, abs_grad_x); // 转换为8位图
    cv::convertScaleAbs(grad_y, abs_grad_y); // 转换为8位图
    cv::addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, dst); // 合并梯度

    cv::namedWindow("src", cv::WINDOW_NORMAL);
    cv::namedWindow("dst", cv::WINDOW_NORMAL);
    cv::imshow("src", src);
    cv::imshow("dst", dst);

    cv::waitKey(0);
    return 0;
}