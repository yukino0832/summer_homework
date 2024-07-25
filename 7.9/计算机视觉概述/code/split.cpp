#include <opencv2/opencv.hpp>

#include <vector>

int main(int argc, char* argv[])
{
    cv::Mat src = cv::imread(argv[1]);
    std::vector<cv::Mat> dst;

    cv::split(src, dst);

    cv::namedWindow("src", cv::WINDOW_NORMAL);
    cv::namedWindow("B", cv::WINDOW_NORMAL);
    cv::namedWindow("G", cv::WINDOW_NORMAL);
    cv::namedWindow("R", cv::WINDOW_NORMAL);
    cv::imshow("src", src);
    cv::imshow("B", dst[0]);
    cv::imshow("G", dst[1]);
    cv::imshow("R", dst[2]);
    
    cv::waitKey(0);
    return 0;
}