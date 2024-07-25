#pragma once

#include <opencv2/opencv.hpp>



class armor
{
public:
    cv::Point2f Armor[4]; // 存放装甲板四个角点
    cv::Mat image;        // PnP解算后的图像
    cv::Mat number_image;
    cv::Mat src; // 原始图像
    std::string type;
    double classification_confidence;
    std::string number;
    std::string classification_result;



    armor(cv::Point2f point1, cv::Point2f point2, cv::Point2f point3, cv::Point2f point4)
    {
        Armor[0] = point1;
        Armor[1] = point2;
        Armor[2] = point3;
        Armor[3] = point4;
    }

    void imgshow(int num, std::size_t i)
    {
        // cv::putText(image, "0", Armor[0], cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(0, 0, 255), 1);
        // cv::putText(image, "1", Armor[1], cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(0, 0, 255), 1);
        // cv::putText(image, "2", Armor[2], cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(0, 0, 255), 1);
        if(i == num-1)
        {
            cv::imshow("image", this->image);
            cv::waitKey(0);
        }
    }

    void PnPcal(cv::Mat camera_matrix, cv::Mat dist_coeffs, cv::Mat &tvec, cv::Mat &rvec)
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

        // 旋转向量
        cv::Mat rotation_vector;
        // 平移向量
        cv::Mat translation_vector;

        cv::solvePnP(model_points, image_points, camera_matrix, dist_coeffs,
                     rotation_vector, translation_vector, 0, cv::SOLVEPNP_ITERATIVE);

        cv::Mat Rvec;
        cv::Mat_<float> Tvec;
        rotation_vector.convertTo(Rvec, CV_32F);    // 旋转向量转换格式
        translation_vector.convertTo(Tvec, CV_32F); // 平移向量转换格式

        rvec = Rvec;
        tvec = Tvec;
    }
};

void find_points(std::vector<armor> &armors, cv::Mat img)
{
    cv::Mat original_img = img;
    // cv::imshow("img", img);
    // cv::waitKey(0);

    cv::Mat gray, binary, imgdilation;
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;

    cv::Point2f mid_Rect1[2];
    cv::Point2f mid_Rect2[2];
    std::vector<std::vector<cv::Point2f>> lights;

    int light_num = 0;

    // 转换为灰度图
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

    // cv::Mat kernal = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, 1));
    // cv::dilate(gray, imgdilation, kernal);

    // 二值化
    cv::threshold(gray, binary, 150, 255, cv::THRESH_BINARY);


    // cv::imshow("bin", binary);
    // cv::waitKey(0);

    // 检测轮廓
    cv::findContours(binary, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    for (std::size_t i = 0; i < contours.size(); i++)
    {
        // 忽略小轮廓
        if (cv::contourArea(contours[i]) < 10)
        {
            continue;
        }

        

        // 计算轮廓的最小外接矩形
        cv::RotatedRect Rect = cv::minAreaRect(contours[i]);

        // 获取最小外接矩形的四个顶点
        cv::Point2f rect_points[4];
        Rect.points(rect_points);

        // 去除血量条影响
        std::sort(rect_points, rect_points+4, [](const cv::Point2f& a, const cv::Point2f& b)
        {
            return a.y < b.y;
        });
        if(rect_points[0].x > rect_points[1].x)
            std::swap(rect_points[0], rect_points[1]);
        if(rect_points[2].x > rect_points[3].x)
            std::swap(rect_points[2], rect_points[3]);


        double light_height = sqrt((rect_points[0].x - rect_points[1].x) * (rect_points[0].x - rect_points[1].x) + (rect_points[0].y - rect_points[1].y) * (rect_points[0].y - rect_points[1].y));

        double light_width = sqrt((rect_points[2].x - rect_points[1].x) * (rect_points[2].x - rect_points[1].x) + (rect_points[2].y - rect_points[1].y) * (rect_points[2].y - rect_points[1].y));

        // if (std::abs(rect_points[2].x - rect_points[1].x) < 2 * std::abs(rect_points[0].y - rect_points[1].y))
        //     continue;

        float ratio = light_height / light_width;
        // std::cout << ratio << std::endl;

        if (ratio < 0.5) // 假设有效灯条的长宽比在1.6到4.8之间
        {

            // 绘制矩形框
            // cv::line(img, rect_points[0], rect_points[1], cv::Scalar(0, 255, 0), 2);
            // cv::line(img, rect_points[1], rect_points[3], cv::Scalar(0, 255, 0), 2);
            // cv::line(img, rect_points[2], rect_points[3], cv::Scalar(0, 255, 0), 2);
            // cv::line(img, rect_points[0], rect_points[2], cv::Scalar(0, 255, 0), 2);

            lights.push_back({(rect_points[0] + rect_points[1]) * 0.5, (rect_points[2] + rect_points[3]) * 0.5});

            // cv::circle(img, lights[light_num][0], 3, cv::Scalar(0, 0, 255), -1);
            // cv::circle(img, lights[light_num][1], 3, cv::Scalar(0, 0, 255), -1);
            light_num++;
        }
    }
    // std::cout << light_num << std::endl;
    // cv::imshow("img", img);
    // cv::waitKey(0);

    for (int i = 0; i < light_num; i++)
    {
        if (lights[i][0].y > lights[i][1].y)
            std::swap(lights[i][0], lights[i][1]);
        for (int j = 0; j < light_num - i - 1; j++)
        {
            if (lights[j][0].x > lights[j + 1][0].x)
            {
                std::swap(lights[j], lights[j + 1]);
            }
        }
    }

    int i = 0;
    while (i < light_num - 1)
    {

        double angle1 = std::atan2(std::abs((lights[i][0].y - lights[i][1].y)), std::abs((lights[i][0].x - lights[i][1].x)));
        double angle2 = std::atan2(std::abs((lights[i + 1][0].y - lights[i + 1][1].y)), std::abs((lights[i + 1][0].x - lights[i + 1][1].x)));
        double angle1_2 = std::abs(angle1 - angle2) * 180 / M_PI;

        double width_light = sqrt((lights[i][0].y - lights[i][1].y) * (lights[i][0].y - lights[i][1].y) + (lights[i][0].x - lights[i][1].x) * (lights[i][0].x - lights[i][1].x));
        double height_light = sqrt((lights[i + 1][0].y - lights[i][0].y) * (lights[i + 1][0].y - lights[i][0].y) + (lights[i + 1][0].x - lights[i][0].x) * (lights[i + 1][0].x - lights[i][0].x));

        if (width_light < height_light)
            std::swap(height_light, width_light);

        double armor_ratio = 0.135 / 0.055;
        // std::cout << angle1_2 << std::endl;

        if ((angle1_2 < 6) && std::abs(lights[i][0].y - lights[i+1][0].y) < 10 && (width_light / height_light) > (0.8 * armor_ratio) && (width_light / height_light) < (1.4 * armor_ratio)) // 0.8到1.4
        {
            // std::cout << i << std::endl;
            cv::circle(img, lights[i][0], 3, cv::Scalar(0, 0, 255), -1);
            cv::circle(img, lights[i][1], 3, cv::Scalar(0, 0, 255), -1);
            cv::circle(img, lights[i + 1][0], 3, cv::Scalar(0, 0, 255), -1);
            cv::circle(img, lights[i + 1][1], 3, cv::Scalar(0, 0, 255), -1);

            
            lights[i][0].x -= 20;
            lights[i][1].x -= 20;
            lights[i + 1][0].x += 20;
            lights[i + 1][1].x += 20;
            
            armor single_armor(lights[i][1], lights[i][0], lights[i + 1][0], lights[i + 1][1]);
            // std::cout << lights[i][1] << " " << lights[i][0] << " " << lights[i + 1][0] << " " << lights[i + 1][1] << std::endl;
            single_armor.image = img;
            single_armor.src = original_img;
            armors.push_back(single_armor);
            i += 2;
            continue;
        }

        i++;
    }
}

