#pragma once

#include "armor.hpp"
#include <fstream>
#include <opencv2/opencv.hpp>
#include <iostream>

class NumberClassifier
{
public:
    NumberClassifier(
        const std::string &model_path, const std::string &label_path,
        const float confidence_threshold,
        const std::vector<std::string> &ignore_classes)
        : confidence_threshold_(confidence_threshold), ignore_classes_(ignore_classes)
    {
        net_ = cv::dnn::readNetFromONNX(model_path);
        std::ifstream label_file(label_path);
        std::string line;
        while (std::getline(label_file, line))
        {
            class_names_.push_back(line);
        }
    }

    /**
     * @brief 提取数字的图像存入 armor.number_img
     *
     * @param src 原始图像
     * @param armors 包含所有装甲板的容器
     */

    void ExtractNumbers(const cv::Mat &src, std::vector<armor> &armors)
    {
        const int light_length = 12;
        const int warp_height = 28;
        const int small_armor_width = 32;
        const int large_armor_width = 54;
        const cv::Size roi_size(20, 28);

        for (auto &armor : armors)
        {
            // 透视变换
            cv::Point2f lights_vertices[4] = {armor.Armor[0],
                                              armor.Armor[1],
                                              armor.Armor[2],
                                              armor.Armor[3]};

            const int top_light_y = (warp_height - light_length) / 2 - 1;
            const int bottom_light_y = top_light_y + light_length;
            const int warp_width = armor.type == "small" ? small_armor_width : large_armor_width;
            cv::Point2f target_vertices[4] = {
                cv::Point(0, bottom_light_y),
                cv::Point(0, top_light_y),
                cv::Point(warp_width - 1, top_light_y),
                cv::Point(warp_width - 1, bottom_light_y),
            };
            cv::Mat number_image;
            auto rotation_matrix =
                cv::getPerspectiveTransform(lights_vertices, target_vertices);
            cv::warpPerspective(src, number_image, rotation_matrix,
                                cv::Size(warp_width, warp_height));

            // 获取数字 ROI
            number_image = number_image(cv::Rect(
                cv::Point((warp_width - roi_size.width) / 2, 0), roi_size));

            cv::imshow("roi", number_image);

            // 二值化
            cv::cvtColor(number_image, number_image, cv::COLOR_RGB2GRAY);
            cv::threshold(number_image, number_image, 0, 255,
                          cv::THRESH_BINARY | cv::THRESH_OTSU);

            cv::imshow("binary", number_image);

            armor.number_image = number_image;
        }
    }

    /**
     * @brief 对装甲板进行分类，结果存入 armor.classfication_result
     *
     * @param armors 装甲板的容器
     */

    void Classify(std::vector<armor> &armors)
    {
        for (auto &armor : armors)
        {
            cv::Mat image = armor.number_image.clone();

            // Normalize
            image = image / 255.0;

            // Create blob from image
            cv::Mat blob;
            cv::dnn::blobFromImage(image, blob);

            // Set the input blob for the neural network
            net_.setInput(blob);
            // Forward pass the image blob through the model
            cv::Mat outputs = net_.forward();

            // Do softmax
            float max_prob =
                *std::max_element(outputs.begin<float>(), outputs.end<float>());
            cv::Mat softmax_prob;
            cv::exp(outputs - max_prob, softmax_prob);
            float sum = static_cast<float>(cv::sum(softmax_prob)[0]);
            softmax_prob /= sum;

            double confidence;
            cv::Point class_id_point;
            minMaxLoc(softmax_prob.reshape(1, 1), nullptr, &confidence, nullptr,
                      &class_id_point);
            int label_id = class_id_point.x;

            armor.classification_confidence = confidence;
            armor.number = class_names_[label_id];

            std::stringstream result_ss;
            result_ss << armor.number << ": " << std::fixed << std::setprecision(1)
                      << armor.classification_confidence * 100.0 << "%";
            armor.classification_result = result_ss.str();
        }

        // erase 删除从 first 到 last 之间
        // first: remove_if 返回的要 remove 的首个元素的 iterator
        // last: 装甲板结尾
        armors.erase(
            // 返回值真的元素移至末尾，返回首个要 remove 的首个元素的 iterator
            std::remove_if(
                armors.begin(), armors.end(),
                [this](const armor &armor)
                {
                    // 装甲板置信度过低
                    if (armor.classification_confidence < confidence_threshold_)
                    {
                        return true;
                    }

                    // 忽略的装甲板类型
                    for (const auto &ignore_class : ignore_classes_)
                    {
                        if (armor.number == ignore_class)
                        {
                            return true;
                        }
                    }

                    // 错误匹配的类型
                    bool mismatch_armor_type = false;
                    if (armor.type == "large")
                    {
                        mismatch_armor_type = armor.number == "outpost" ||
                                              armor.number == "2" ||
                                              armor.number == "guard";
                    }
                    else if (armor.type == "small")
                    {
                        mismatch_armor_type = armor.number == "1" ||
                                              armor.number == "base";
                    }
                    return mismatch_armor_type;
                }),
            armors.end());
    }

    /**
     * @brief 更新忽略的类别
     *
     * @param ignore_classes 忽略的类别
     */

    void UpdateIgnoreClasses(const std::vector<std::string> &ignore_classes)
    {
        ignore_classes_ = ignore_classes;
    }

private:
    float confidence_threshold_;              // 数字分类置信度阈值
    cv::dnn::Net net_;                        // 数字分类网络
    std::vector<std::string> class_names_;    // 类别名字
    std::vector<std::string> ignore_classes_; // 忽略的类别
};
