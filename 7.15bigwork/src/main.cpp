#include <iostream>
#include <opencv2/opencv.hpp>
#include "../armor_detector/armor.hpp"
#include "../armor_detector/classify_number.hpp"

int main()
{
    cv::VideoCapture cap("/home/zcy/ZCY/hdu/rm/7.15bigwork/images/7月14日.mp4");
    cv::Mat img;
    std::string path = "/home/zcy/ZCY/hdu/rm/7.15bigwork/images/armor1.jpg";
    std::string model_path = "/home/zcy/ZCY/hdu/rm/7.9/0709work/model/mlp.onnx";
    std::string label_path = "/home/zcy/ZCY/hdu/rm/7.9/0709work/model/label.txt";
    std::vector<std::string> ignore_classes = {"outpost", "base", "guard", "negative"};
    while (1)
    {
        cap >> img;
        std::vector<armor> armors;
        find_points(armors, img);
        // std::cout << armors.size() << std::endl;
        for (std::size_t i = 0; i < armors.size(); i++)
        {
            armors[i].PnPcal();
            NumberClassifier classify_number(model_path, label_path, 0.65, ignore_classes);
            classify_number.ExtractNumbers(armors[i].src, armors);
            classify_number.Classify(armors);
            std::cout << "result:" << armors[i].classification_result << std::endl;
            std::cout << "confidence:" << armors[i].classification_confidence << std::endl;
            std::cout << "number" << armors[i].number << std::endl;

            cv::circle(img, (armors[i].Armor[0] + armors[i].Armor[1] + armors[i].Armor[2] + armors[i].Armor[3]) / 4, 30, cv::Scalar(0, 255, 0), 2);

            armors[i].imgshow(armors.size(), i);
        }
    }

    cap.release();



    // cv::Mat img;
    // img = cv::imread("/home/zcy/ZCY/hdu/rm/7.15bigwork/images/armor4.jpg");
    // std::string model_path = "/home/zcy/ZCY/hdu/rm/7.9/0709work/model/mlp.onnx";
    // std::string label_path = "/home/zcy/ZCY/hdu/rm/7.9/0709work/model/label.txt";
    // std::vector<std::string> ignore_classes = {"outpost", "base", "guard", "negative"};
    // std::vector<armor> armors;
    // find_points(armors, img);
    // // std::cout << armors.size() << std::endl;
    // for (std::size_t i = 0; i < armors.size(); i++)
    // {
    //     armors[i].PnPcal();
        
    //     NumberClassifier classify_number(model_path, label_path, 0.65, ignore_classes);
    //     classify_number.ExtractNumbers(armors[i].src, armors);
    //     classify_number.Classify(armors);
    //     // std::cout << "result:" << armors[i].classification_result << std::endl;
    //     // std::cout << "confidence:" << armors[i].classification_confidence << std::endl;
    //     std::cout << "number" << armors[i].number << std::endl;

    //     cv::circle(img, (armors[i].Armor[0] + armors[i].Armor[1] + armors[i].Armor[2] + armors[i].Armor[3]) / 4, 10, cv::Scalar(0, 255, 0), 2);

    //     // cv::imshow("img", armors[i].image);
    //     // cv::waitKey(0);

    //     armors[i].imgshow(armors.size(), i);
    // }

    return 0;
}
