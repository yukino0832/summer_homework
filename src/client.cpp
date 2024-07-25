#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <chrono>
#include <opencv2/opencv.hpp>
#include "../include/final_cal.hpp"

int main()
{
    FrameHandle framehandle;

    CameraInfoData camera_info;
    TransformData transform_data;

    cv::Mat camera_matrix;
    cv::Mat dist_coeffs;

    const char *server_ip = "10.2.20.55"; // 修改为服务端的 IP 地址
    const int server_port_camera = 5140;        // 相机内参
    const int server_port_request = 4399;        // request
    const int server_port_image = 8000;        // image

    // 创建 socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        std::cerr << "Socket creation failed!" << std::endl;
        return -1;
    }

    while(true)
    {
        std::string command;
        std::cin >> command;
        sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        if(command == "q")
        {
            std::cout << "quit" << std::endl;
            break;
        }
        else if(command == "camera")
        {
            // 获取开始时间
            std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
            // 设置超时时间为20秒
            std::chrono::seconds timeout_duration(20);

            server_addr.sin_port = htons(server_port_camera);
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

            MessageBuffer buffer;
            
            read(sockfd, &(buffer.Start), 2);

            if (buffer.Start != 0x0D00)
            {
                std::cout << "Error start data!" << std::endl;
                exit(EXIT_FAILURE);
            }

            read(sockfd, &(buffer.MessageType), 2);

            if(buffer.MessageType != CAMERA_INFO)
            {
                std::cout << "Error Message Type" << std::endl;
                continue;
            }
            else
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
            // 获取当前时间
            std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_time = current_time - start_time;

            // 检查是否超时
            if (elapsed_time >= timeout_duration)
            {
                std::cout << "Timeout,quit" << std::endl;
                break;
            }
        }
        else if(command == "image")
        {
            // 获取开始时间
            std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
            // 设置超时时间为20秒
            std::chrono::seconds timeout_duration(20);
            sockaddr_in server_addr;
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(server_port_image);
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

                if(buffer.MessageType != IMAGE_MSG)
                {
                    std::cout << "Error Message Type" << std::endl;
                    continue;
                }

                else
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

                    if (buffer.Offset + buffer.DataLenth >= buffer.DataTotalLenth)
                    {
                        std::vector<unsigned char> image_data = data_temp[buffer.DataID];
                        std::string img_ID = std::to_string(buffer.DataID);
                        img_ID.append(".jpg");
                        std::cout << "receive " << img_ID << std::endl;
                        data_temp.erase(buffer.DataID);
                        image_data_decode(image_data, framehandle, sockfd);
                        framehandle.send_pose(sockfd);
                    }
                }
                // 获取当前时间
                std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
                std::chrono::duration<double> elapsed_time = current_time - start_time;

                // 检查是否超时
                if (elapsed_time >= timeout_duration)
                {
                    std::cout << "Timeout,quit" << std::endl;
                    break;
                }
            }
        }

        else if(command == "request")
        {
            // 获取开始时间
            std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
            // 设置超时时间为20秒
            std::chrono::seconds timeout_duration(20);
            sockaddr_in server_addr;

            // 设置服务器地址
            sockaddr_in server_addr;
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(server_port_request);
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

            TransformRequestData request;
            std::cin >> request.From;
            std::cin >> request.To;
            send(sockfd, request.From, strlen(request.From), 0);
            send(sockfd, request.To, strlen(request.To), 0);

            MessageBuffer buffer;

            read(sockfd, &(buffer.Start), 2);

            if (buffer.Start != 0x0D00)
            {
                std::cout << "Error start data!" << std::endl;
                exit(EXIT_FAILURE);
            }

            read(sockfd, &(buffer.MessageType), 2);
            read(sockfd, &(buffer.DataID), 4);
            read(sockfd, &(buffer.DataTotalLenth), 4);
            read(sockfd, &(buffer.Offset), 4);
            read(sockfd, &(buffer.DataLenth), 4);
            read(sockfd, &(buffer.Data), buffer.DataLenth);
            
            memcpy(transform_data.Translation, buffer.Data, sizeof(transform_data.Translation));
            memcpy(transform_data.Rotation, buffer.Data + sizeof(transform_data.Translation), sizeof(transform_data.Rotation));

            // 获取当前时间
            std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_time = current_time - start_time;

            // 检查是否超时
            if (elapsed_time >= timeout_duration)
            {
                std::cout << "Timeout,quit" << std::endl;
                break;
            }
        }
    }

    // 关闭 socket
    close(sockfd);

    return 0;
}
