#include <opencv2/opencv.hpp>

int main(int argc, char* argv[]) {
    cv::Mat src = cv::imread(argv[1]);
    cv::Mat dst;
    cv::Mat padded; // 填充后的图像
    cv::Mat complexI; // 复数形式的频谱图

    cv::cvtColor(src, src, cv::COLOR_BGR2GRAY); // 转换为灰度图
    src.convertTo(padded, CV_32F); // 转换为浮点型
    int m = cv::getOptimalDFTSize(padded.rows);
    int n = cv::getOptimalDFTSize(padded.cols);

    cv::copyMakeBorder(padded, padded, 0, m - padded.rows, 0, n - padded.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0)); // 填充图像

    cv::dft(padded, complexI, cv::DFT_COMPLEX_OUTPUT); // 傅里叶变换
    // 分成四个象限
    cv::Mat q0 = complexI(cv::Rect(0, 0, complexI.cols / 2, complexI.rows / 2));
    cv::Mat q1 = complexI(cv::Rect(complexI.cols / 2, 0, complexI.cols / 2, complexI.rows / 2));
    cv::Mat q2 = complexI(cv::Rect(0, complexI.rows / 2, complexI.cols / 2, complexI.rows / 2));
    cv::Mat q3 = complexI(cv::Rect(complexI.cols / 2, complexI.rows / 2, complexI.cols / 2, complexI.rows / 2));
    // 交换象限
    cv::Mat tmp;
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);
    q1.copyTo(tmp);
    q2.copyTo(q1);
    tmp.copyTo(q2);
    
    // 生成低通滤波器
    int radius = std::stoi(argv[2]);
    cv::Mat lowPass = cv::Mat::zeros(complexI.size(), CV_32FC2);
    cv::Point center = cv::Point(lowPass.cols / 2, lowPass.rows / 2);
    cv::circle(lowPass, center, radius, cv::Scalar(1), -1);

    // 滤波
    cv::mulSpectrums(complexI, lowPass, complexI, 0);

    cv::namedWindow("complexI", cv::WINDOW_NORMAL);
    cv::Mat plane[] = { cv::Mat::zeros(complexI.size(), CV_32F),
                        cv::Mat::zeros(complexI.size(), CV_32F) };
    cv::split(complexI, plane);
    cv::magnitude(plane[0], plane[1], plane[0]);    // 求幅值
    cv::log(plane[0] + cv::Scalar::all(1), plane[0]);   // 对数变换，方便显示
    cv::normalize(plane[0], plane[0], 0, 1, cv::NORM_MINMAX);   // 归一化
    cv::imshow("complexI", plane[0]);   // 显示频谱图

    // 交换象限
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);
    q1.copyTo(tmp);
    q2.copyTo(q1);
    tmp.copyTo(q2);

    cv::idft(complexI, dst, cv::DFT_SCALE | cv::DFT_REAL_OUTPUT); // 傅里叶逆变换

    cv::normalize(dst, dst, 0, 255, cv::NORM_MINMAX); // 归一化
    dst.convertTo(dst, CV_8UC1); // 转换为8位图

    cv::namedWindow("src", cv::WINDOW_NORMAL);
    cv::namedWindow("dst", cv::WINDOW_NORMAL);
    cv::imshow("src", src);
    cv::imshow("dst", dst);

    cv::waitKey(0);
    return 0;
}