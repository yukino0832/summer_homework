#include <opencv2/opencv.hpp>

#include <vector>

int main(int argc, char* argv[])
{
    cv::Mat src = cv::imread(argv[1]);  // 读取图像
    cv::Mat hsv;    // 存储 HSV 图像
    std::vector<cv::Mat> dst;   // 存储分离后的通道

    cv::cvtColor(src, hsv, cv::COLOR_BGR2HSV);    // RGB 转 HSV
    cv::split(hsv, dst);    // 分离通道

    cv::namedWindow("src", cv::WINDOW_NORMAL);  // 创建窗口，WINDOW_NORMAL 表示窗口大小可以调整
    cv::namedWindow("H", cv::WINDOW_NORMAL);
    cv::namedWindow("S", cv::WINDOW_NORMAL);
    cv::namedWindow("V", cv::WINDOW_NORMAL);
    cv::imshow("src", src); // 显示图像
    cv::imshow("H", dst[0]);
    cv::imshow("S", dst[1]);
    cv::imshow("V", dst[2]);
    
    cv::waitKey(0); // 等待窗口关闭 (0表示一直等待)
    return 0;
}