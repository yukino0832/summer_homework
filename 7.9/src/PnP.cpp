#include <iostream>
#include <opencv2/opencv.hpp>
#include "../armor_detector/armor.hpp"
#include "../armor_detector/classify_number.hpp"

int main()
{

    std::string path = "/home/zcy/ZCY/hdu/rm/7.9/0709work/armor_imgs/14.jpg";
    std::string model_path = "/home/zcy/ZCY/hdu/rm/7.9/0709work/model/mlp.onnx";
    std::string label_path = "/home/zcy/ZCY/hdu/rm/7.9/0709work/model/label.txt";
    std::vector<std::string> ignore_classes = {"outpost", "base", "guard", "negative"};
    armor armor_small(path, "small");
    armor_small.PnPcal();
    std::vector<armor> armors;
    armors.push_back(armor_small);
    NumberClassifier classify_number(model_path, label_path, 0.65, ignore_classes);
    classify_number.ExtractNumbers(armor_small.src, armors);
    classify_number.Classify(armors);
    
    std::cout << "result:" << armors[0].classification_result << std::endl;
    std::cout << "confidence:" << armors[0].classification_confidence << std::endl;
    std::cout << "number" << armors[0].number << std::endl;

    armor_small.imgshow();

    return 0;
}