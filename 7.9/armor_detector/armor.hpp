#pragma once

#include <opencv2/opencv.hpp>

class armor
{
public:
    cv::Point2f Armor[4];                     // 存放装甲板四个角点
    cv::Mat image;                            // PnP解算后的图像
    cv::Mat number_image;
    cv::Mat src;                              // 原始图像
    std::string type;
    double classification_confidence;
    std::string number;
    std::string classification_result;

    armor(std::string path, std::string armor_type)
    {

        this->type = armor_type;

        cv::Mat img = cv::imread(path);
        this->src = img;

        cv::Mat gray, binary;
        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;

        cv::Point2f mid_Rect1[2];
        cv::Point2f mid_Rect2[2];

        int time = 0;

        // 转换为灰度图
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

        // 二值化
        cv::threshold(gray, binary, 150, 255, cv::THRESH_BINARY);

        // cv::imshow("binary", binary);

        // 检测轮廓
        cv::findContours(binary, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        for (std::size_t i = 0; i < contours.size(); i++)
        {
            // 忽略小轮廓
            if (cv::contourArea(contours[i]) < 50)
            {
                continue;
            }

            // 计算轮廓的最小外接矩形
            cv::RotatedRect Rect = cv::minAreaRect(contours[i]);

            // 获取最小外接矩形的四个顶点
            cv::Point2f rect_points[4];
            Rect.points(rect_points);

            float ratio = std::max(Rect.size.width, Rect.size.height) / std::min(Rect.size.width, Rect.size.height);
            if (ratio > 1.0 && ratio < 7) // 假设灯条的长宽比在1到10之间
            {
                // 绘制矩形框
                for (int j = 0; j < 4; j++)
                {
                    cv::line(img, rect_points[j], rect_points[(j + 1) % 4], cv::Scalar(0, 255, 0), 2);
                }
            }

            if (std::max(Rect.size.width, Rect.size.height) == Rect.size.height)
            {
                mid_Rect1[time] = (rect_points[0] + rect_points[3]) * 0.5;
                mid_Rect2[time] = (rect_points[1] + rect_points[2]) * 0.5;
            }

            if (Rect.size.width == std::max(Rect.size.width, Rect.size.height))
            {
                mid_Rect1[time] = (rect_points[0] + rect_points[1]) * 0.5;
                mid_Rect2[time] = (rect_points[2] + rect_points[3]) * 0.5;
            }

            cv::circle(img, mid_Rect1[time], 3, cv::Scalar(0, 0, 255), -1);
            cv::circle(img, mid_Rect2[time], 3, cv::Scalar(0, 0, 255), -1);
            time++;
        }
        this->Armor[0] = mid_Rect1[0];
        this->Armor[1] = mid_Rect1[1];
        this->Armor[2] = mid_Rect2[0];
        this->Armor[3] = mid_Rect2[1];



        cv::Point2f swap;
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 3 - i; j++)
            {
                if (this->Armor[j].x > this->Armor[j + 1].x)
                {
                    swap = this->Armor[j];
                    this->Armor[j] = this->Armor[j + 1];
                    this->Armor[j + 1] = swap;
                }
            }
        }

        if (this->Armor[0].y < this->Armor[1].y)
        {
            swap = this->Armor[0];
            this->Armor[0] = this->Armor[1];
            this->Armor[1] = swap;
        }

        if (this->Armor[2].y > this->Armor[3].y)
        {
            swap = this->Armor[2];
            this->Armor[2] = this->Armor[3];
            this->Armor[3] = swap;
        }

        cv::putText(img, "0", Armor[0], cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(0, 0, 255), 1);
        cv::putText(img, "1", Armor[1], cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(0, 0, 255), 1);
        cv::putText(img, "2", Armor[2], cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(0, 0, 255), 1);
        this->image = img;
    }



    void imgshow()
    {

        cv::imshow("image", this->image);
        cv::waitKey(0);
    }

    void PnPcal()
    {
        std::vector<cv::Point2f> image_points;
        image_points.push_back(this->Armor[0]);
        image_points.push_back(this->Armor[1]);
        image_points.push_back(this->Armor[2]);
        image_points.push_back(this->Armor[3]);

        const float SMALL_ARMOR_WIDTH = 0.135;
        const float SMALL_ARMOR_HEIGHT = 0.055;
        const float LARGE_ARMOR_WIDTH = 0.225;
        const float LARGE_ARMOR_HEIGHT = 0.055;

        // x 垂直装甲板向内，从左下角开始顺时针
        const std::vector<cv::Point3f> model_points = {
            {0, +SMALL_ARMOR_WIDTH / 2, -SMALL_ARMOR_HEIGHT / 2},
            {0, +SMALL_ARMOR_WIDTH / 2, +SMALL_ARMOR_HEIGHT / 2},
            {0, -SMALL_ARMOR_WIDTH / 2, +SMALL_ARMOR_HEIGHT / 2},
            {0, -SMALL_ARMOR_WIDTH / 2, -SMALL_ARMOR_HEIGHT / 2}};

        cv::Mat camera_matrix = (cv::Mat_<double>(3, 3) << 2102.080562187802,
                            0,
                            689.2057889332623,
                            0,
                            2094.0179120166754,
                            496.6622802275393,
                            0,
                            0,
                            1);

        cv::Mat dist_coeffs = (cv::Mat_<double>(5, 1) << -0.06478109387525666,
                        0.39036067923005396,
                        -0.0042514793151166306,
                        0.008306749648029776,
                        -1.6613800909405605);

        // 旋转向量
        cv::Mat rotation_vector;
        // 平移向量
        cv::Mat translation_vector;

        cv::solvePnP(model_points, image_points, camera_matrix, dist_coeffs,
                rotation_vector, translation_vector, 0, cv::SOLVEPNP_ITERATIVE);

        cv::Mat Rvec;
        cv::Mat_<float> Tvec;
        rotation_vector.convertTo(Rvec, CV_32F);  // 旋转向量转换格式
        translation_vector.convertTo(Tvec, CV_32F); // 平移向量转换格式 

        std::cout << "Rvec:" << std::endl
                  << Rvec << std::endl;

        std::cout << "Tvec:" << std::endl
            << Tvec << std::endl;

    }

};