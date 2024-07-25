#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <opencv2/opencv.hpp>
#include "../armor_detector/final_cal.hpp"

typedef struct
{
    unsigned short Start = 0x0D00;    // 0~1
    unsigned short MessageType;       // 2~3
    unsigned int DataID;              // 4~7
    unsigned int DataTotalLenth;      // 8~11
    unsigned int Offset;              // 12~15
    unsigned int DataLenth;          // 16~19
    unsigned char Data[10218];        // 20~10237
    unsigned short End = 0x0721;      // 10238~10239
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

int main()
{
    FrameHandle framehandle;

    CameraInfoData camera_info;
    TransformData transform_data;

    cv::Mat camera_matrix;
    cv::Mat dist_coeffs;

    const char *server_ip = "10.2.20.55"; // 修改为服务端的 IP 地址
    const int server_port = 5140;        // 相机内参
    const int server_port = 4399;        // request
    const int server_port = 5140;        // 

    // 创建 socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        std::cerr << "Socket creation failed!" << std::endl;
        return -1;
    }

    // 设置服务器地址
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
    {
        std::cerr << "Invalid address/ Address not supported!" << std::endl;
        close(sockfd);
        return -1;
    }

    // 连接到服务器
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cerr << "Connection failed!" << std::endl;
        close(sockfd);
        return -1;
    }

    std::unordered_map<unsigned int, std::vector<unsigned char>> data_temp;
    std::size_t size = 0;

    while(true)
    {
        MessageBuffer buffer;

        read(sockfd, &(buffer.Start), 2);

        if (buffer.Start != 0x0D00)
        {
            std::cout << "Error start data!" << std::endl;
            exit(EXIT_FAILURE);
        }

        read(sockfd, &(buffer.MessageType), 2);

        if(buffer.MessageType == IMAGE_MSG)
        {
            read(sockfd, &(buffer.DataID), 4);
            read(sockfd, &(buffer.DataTotalLenth), 4);
            read(sockfd, &(buffer.Offset), 4);
            read(sockfd, &(buffer.DataLenth), 4);

            if (data_temp.find(buffer.DataID) == data_temp.end()) // 新的图片
            {
                data_temp[buffer.DataID].resize(buffer.DataTotalLenth);
            }

            read(sockfd, &(buffer.Data), buffer.DataLenth);
            size += buffer.DataLenth;

            memcpy(&data_temp[buffer.DataID] + buffer.Offset, buffer.Data, buffer.DataLenth);

            if(buffer.Offset + buffer.DataLenth >= buffer.DataTotalLenth)
            {
                std::vector<unsigned char> image_data = data_temp[buffer.DataID];
                std::string img_ID = std::to_string(buffer.DataID);
                img_ID.append(".jpg");
                std::cout << "receive " << img_ID << std::endl;
                data_temp.erase(buffer.DataID);
                image_data_decode(image_data, framehandle, sockfd);
                ;
            }
        }

        else if(buffer.MessageType == STRING_MSG)
        {
            read(sockfd, &(buffer.DataID), 4);
            read(sockfd, &(buffer.DataTotalLenth), 4);
            read(sockfd, &(buffer.Offset), 4);
            read(sockfd, &(buffer.DataLenth), 4);
            if (read(sockfd, &(buffer.Data), buffer.DataLenth) >= 0)
            {
                const char *string_message = "string message received!";
                send(sockfd, string_message, strlen(string_message), 0);
            }
            std::cout << buffer.Data << std::endl;
        }

        else if(buffer.MessageType == CAMERA_INFO)
        {
            read(sockfd, &(buffer.DataID), 4);
            read(sockfd, &(buffer.DataTotalLenth), 4);
            read(sockfd, &(buffer.Offset), 4);
            read(sockfd, &(buffer.DataLenth), 4);
            if (read(sockfd, &(buffer.Data), buffer.DataLenth) >= 0)
            {
                const char *camera_message = "Camera imformation received!";
                send(sockfd, camera_message, strlen(camera_message), 0);
            }
            memcpy(camera_info.CameraMatrix, buffer.Data, sizeof(camera_info.CameraMatrix));
            memcpy(camera_info.DistortionCoefficients, buffer.Data + sizeof(camera_info.CameraMatrix), sizeof(camera_info.CameraMatrix));
            std::cout << "CameraMatrix:" << camera_info.CameraMatrix << std::endl;
            std::cout << "DistortionCoefficients:" << camera_info.DistortionCoefficients << std::endl;
            camera_matrix = (cv::Mat_<double>(3, 3) << camera_info.CameraMatrix[0],
                             camera_info.CameraMatrix[1],
                             camera_info.CameraMatrix[2],
                             camera_info.CameraMatrix[3],
                             camera_info.CameraMatrix[4],
                             camera_info.CameraMatrix[5],
                             camera_info.CameraMatrix[6],
                             camera_info.CameraMatrix[7],
                             camera_info.CameraMatrix[8]);
            cv::Mat dist_coeffs = (cv::Mat_<double>(5, 1) << camera_info.DistortionCoefficients[0],
                                   camera_info.DistortionCoefficients[1],
                                   camera_info.DistortionCoefficients[2],
                                   camera_info.DistortionCoefficients[3],
                                   camera_info.DistortionCoefficients[4]);
            framehandle.get_camera_info(camera_matrix, dist_coeffs);
        }

        read(sockfd, &(buffer.End), 2);
        if(buffer.End != 0x0721)
        {
            std::cout << "Error end data" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    // 关闭 socket
    close(sockfd);

    return 0;
}

