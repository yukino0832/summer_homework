#include <opencv2/opencv.hpp>

int main(int argc, char* argv[]) {
    cv::Mat src = cv::imread(argv[1]);
    cv::Mat dst;
    cv::Mat gray;
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5)); // 结构元素

    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY); // 转换为灰度图
    cv::threshold(gray, gray, 100, 255, cv::THRESH_BINARY); // 二值化
    
    cv::namedWindow("src", cv::WINDOW_NORMAL);
    cv::namedWindow("dilate", cv::WINDOW_NORMAL);
    cv::namedWindow("erode", cv::WINDOW_NORMAL);
    cv::namedWindow("close", cv::WINDOW_NORMAL);
    cv::namedWindow("open", cv::WINDOW_NORMAL);
    cv::imshow("src", src);
    
    cv::morphologyEx(gray, dst, cv::MORPH_DILATE, element); // 膨胀
    cv::imshow("dilate", dst);
    cv::morphologyEx(gray, dst, cv::MORPH_ERODE, element); // 腐蚀
    cv::imshow("erode", dst);
    cv::morphologyEx(gray, dst, cv::MORPH_CLOSE, element); // 闭运算
    cv::imshow("close", dst);
    cv::morphologyEx(gray, dst, cv::MORPH_OPEN, element); // 开运算
    cv::imshow("open", dst);


    cv::waitKey(0);
    return 0;
}