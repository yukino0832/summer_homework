#include <iostream>
#include <opencv2/opencv.hpp>
#include <Eigen/Dense>
#include "/home/zcy/ZCY/hdu/rm/7.8/include/Pose.hpp"
#include "armor.hpp"
#include "classify_number.hpp"
#include "kalman.hpp"

typedef struct
{
    unsigned short Start = 0x0D00; // 0~1
    unsigned short MessageType;    // 2~3
    unsigned int DataID;           // 4~7
    unsigned int DataTotalLenth;   // 8~11
    unsigned int Offset;           // 12~15
    unsigned int DataLenth;        // 16~19
    unsigned char Data[10218];     // 20~10237
    unsigned short End = 0x0721;   // 10238~10239
} MessageBuffer;

enum MessageType
{
    STRING_MSG = 0x0000,
    IMAGE_MSG = 0x1145,
    CAMERA_INFO = 0x1419,
    TRANSFORM = 0x1981,
    TRANSFORM_REQUEST = 0x1982
};

// type == CAMERA_INFO
typedef struct
{
    double CameraMatrix[9];
    double DistortionCoefficients[5];
} CameraInfoData;

// type == TRANSFORM
typedef struct
{
    double Translation[3];
    double Rotation[4];
} TransformData;

// type == TRANSFORM_REQUEST
typedef struct
{
    char From[10218 / 2];
    char To[10218 / 2];
} TransformRequestData;

void image_data_decode(std::vector<unsigned char> data, FrameHandle &framehandle, int sockfd)
{
    cv::Mat frame = cv::imdecode(data, cv::IMREAD_COLOR);
    framehandle.calculate(frame);
    framehandle.send_pose(sockfd);
}

void armor_detection(const cv::Mat &img, std::vector<cv::Mat> &tvec, std::vector<cv::Mat> &rvec, cv::Mat cameraMatrix, cv::Mat distCoeffs, std::vector<std::string> &result)
{
    std::vector<armor> armors;
    cv::Mat t_tvec, t_rvec;

    std::string model_path = "/home/zcy/ZCY/hdu/rm/7.9/0709work/model/mlp.onnx";
    std::string label_path = "/home/zcy/ZCY/hdu/rm/7.9/0709work/model/label.txt";
    std::vector<std::string> ignore_classes = {"outpost", "base", "guard", "negative"};

    find_points(armors, img);

    for (std::size_t i = 0; i < armors.size(); i++)
    {
        armors[i].PnPcal(cameraMatrix, distCoeffs, t_tvec, t_rvec);
        tvec.push_back(t_tvec);
        rvec.push_back(t_rvec);
        NumberClassifier classify_number(model_path, label_path, 0.65, ignore_classes);
        classify_number.ExtractNumbers(armors[i].src, armors);
        classify_number.Classify(armors);
        // std::cout << "result:" << armors[i].classification_result << std::endl;
        // std::cout << "confidence:" << armors[i].classification_confidence << std::endl;
        // std::cout << "number" << armors[i].number << std::endl;
        result.push_back(armors[i].classification_result);
    }
}


class FrameHandle
{
public:
    Pose transform_pose;
    Eigen::VectorXd X;
    Eigen::MatrixXd P, R, Q;   // 卡尔曼滤波参数
    cv::Mat cameraMatrix;
    cv::Mat distCoeffs;
    cv::Mat image;             // 保存处理后的图像

    std::vector<std::vector<double>> result_pose[2];  // [0]为装甲板中心点，[1]为车中心
    Pose last_pose;            // 上一帧的位置信息
    bool is_first;             // 是否为第一帧
    int last_frame;            // 保存上一帧
    int now_frame;             // 迭代器，记录当前帧

    FrameHandle() : transform_pose({-0.2, 0, 0},Quaternion(M_PI / 2, M_PI / 2, 0)), 
                  is_first(true),
                  last_frame(0),
                  now_frame(0)
    {
        // 状态向量 (位置和速度)
        X = Eigen::VectorXd(6);
        X << 0, 0, 0, 0, 0, 0;

        // 状态协方差矩阵
        P = Eigen::MatrixXd(6, 6);
        P.setIdentity();
        P = 100 * P;

        // 测量噪声协方差矩阵
        R = Eigen::MatrixXd(3, 3);
        R.setIdentity();
        R = 100 * R;

        // 过程噪声协方差矩阵
        Q = Eigen::MatrixXd(6, 6);
        Q.setIdentity();
        Q = 100 * Q;
    }

    void get_camera_info(cv::Mat camera_matrix, cv::Mat dist_coeffs)
    {
        this->cameraMatrix = camera_matrix;
        this->distCoeffs = dist_coeffs;
    }

    void calculate(cv::Mat &frame)
    {
        std::vector<Pose> pose;
        std::vector<cv::Mat> tvec, rvec;
        std::vector<std::string> result;
        std::vector<double> center(3);
        std::vector<Eigen::VectorXd> observed_value; // 测量值
        std::vector<double> v;                    // 线速度
        std::vector<double> w;                    // 角速度
        std::vector<double> save_pose = {0, 0, 0, 0, 0, 0};
        std::vector<double> save_last_pose = {0, 0, 0, 0, 0, 0};
        double rot_r; // 旋转半径

        armor_detection(frame, tvec, rvec, this->cameraMatrix, this->distCoeffs, result);

        for (int i = 0; i < rvec.size(); i++)
        {
            Pose t_pose;
            cv::Mat tvec1 = tvec[i];
            cv::Mat rvec1 = rvec[i];
            double x = tvec1.at<double>(0, 0);
            double y = tvec1.at<double>(0, 1);
            double z = tvec1.at<double>(0, 2);
            Quaternion q(rvec1.at<double>(0, 2), rvec1.at<double>(0, 1), rvec1.at<double>(0, 0));
            t_pose.position[0] = x;
            t_pose.position[1] = y;
            t_pose.position[2] = z;
            t_pose.rotation = q;
            pose.push_back(t_pose);
        }
        int i = 0;
        if (is_first) // 是否为第一帧
        {
            last_pose = pose[0];
            is_first = false;
        }
        bool flag = false;
        double x_now_last, x_max = 0;
        int max = 0;

        for (int k = 0; k < pose.size(); k++)
        {
            if (result[i][0] == 'n')
            {
                i++;
                continue;
            }
            x_now_last = pow(pose[k].position[0] - last_pose.position[0], 2) + pow(pose[k].position[1] - last_pose.position[1], 2) + pow(pose[k].position[2] - last_pose.position[2], 2);
            if (!flag)
            {
                flag = true;
                x_max = x_now_last;
                max = 0;
                i++;
                continue;
            }
            if (x_now_last > x_max)
            {
                x_max = x_now_last;
                max = i;
                i++;
            }
        }

        if (result[max][0] != 'n')
        {
            result_pose[0].push_back(std::vector<double>(6, 0.0));
            result_pose[1].push_back(std::vector<double>(3, 0.0));
        }

        else
        {
            Pose p = pose[max];

            Quaternion rot = p.rotation;
            Pose pose_gimbal = p + transform_pose;
            save_pose[0] = pose_gimbal.position[0];
            save_pose[1] = pose_gimbal.position[1];
            save_pose[2] = pose_gimbal.position[2];
            double yaw, pitch, roll;
            pose_gimbal.rotation.toEularAngles(roll, yaw, pitch);
            save_pose[3] = yaw;
            save_pose[4] = pitch;
            save_pose[5] = roll;
            result_pose[0].push_back(save_pose);
            save_last_pose[0] = last_pose.position[0];
            save_last_pose[1] = last_pose.position[1];
            save_last_pose[2] = last_pose.position[2];
            last_pose.rotation.toEularAngles(roll, yaw, pitch);
            save_last_pose[3] = yaw;
            save_last_pose[4] = pitch;
            save_last_pose[5] = roll;

            cv::Mat_<double> rvec_now = {save_pose[3], save_pose[4], save_pose[5]};
            cv::Mat_<double> rvec_last = {save_last_pose[3], save_last_pose[4], save_last_pose[5]};

            for (int i = 0; i < 3; i++)
            {
                v.push_back(std::abs((save_last_pose[i] - save_pose[i])) / (now_frame - last_frame));
                w.push_back(std::abs((save_last_pose[i + 3] - save_pose[i + 3])) / (now_frame - last_frame));
            }
            double v_total = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
            double w_total = std::sqrt(w[0] * w[0] + w[1] * w[1] + w[2] * w[2]);

            rot_r = v_total / w_total;

            center[0] = save_pose[0] + v[0] / v_total * rot_r;
            center[1] = save_pose[1] + v[1] / v_total * rot_r;
            center[2] = save_pose[2];

            observed_value.push_back({center[0], center[1], center[2]});
            result_pose[1].push_back(center);
            update(X, P, R, observed_value[0]);
            last_pose = p;
            last_frame = now_frame;
        }
        now_frame += 1;
        predict(X, P, Q);
    }

    void send_pose(int sockfd)
    {
        std::string message = "x:" + std::to_string(result_pose[1][now_frame - 1][0]) + "y:" + std::to_string(result_pose[1][now_frame - 1][1]) + "z:" + std::to_string(result_pose[1][now_frame - 1][2]);
        send(sockfd, message.c_str(), message.size(), 0);
    }
};
