#include <opencv2/opencv.hpp>

int main(int argc, char* argv[]) {
    cv::Mat src = cv::imread(argv[1]);
    cv::Mat dst;
    cv::Mat gray;
    cv::Mat dst_norm, dst_norm_scaled;

    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY); // 转换为灰度图
    cv::cornerHarris(gray, dst, 2, 3, 0.04); // Harris 角点检测
    cv::normalize(dst, dst_norm, 0, 255, cv::NORM_MINMAX, CV_32FC1, cv::Mat()); // 归一化
    cv::convertScaleAbs(dst_norm, dst_norm_scaled); // 转换为8位图

    for (int i = 0; i < dst_norm.rows; i++) {
        for (int j = 0; j < dst_norm.cols; j++) {
            if ((int)dst_norm.at<float>(i, j) > 100) {
                cv::circle(src, cv::Point(j, i), 5, cv::Scalar(0, 0, 255), 2, 8, 0);
            }
        }
    }

    cv::namedWindow("src", cv::WINDOW_NORMAL);
    cv::imshow("src", src);

    cv::waitKey(0);
    return 0;
}