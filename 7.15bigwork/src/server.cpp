#include <iostream>
#include <vector>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <opencv2/opencv.hpp>

int main()
{
    const char *server_ip = "127.0.0.1";           // 修改为你的本机 IP 地址
    const int server_port = 8080;                      // 服务器端口
    const char *video_path = "/home/zcy/ZCY/hdu/rm/7.15bigwork/images/7月14日.mp4"; // 修改为你的视频文件路径

    // 创建 socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        std::cerr << "Socket creation failed!" << std::endl;
        return 1;
    }

    // 设置服务器地址
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
    {
        std::cerr << "Invalid address/ Address not supported!" << std::endl;
        close(server_fd);
        return 1;
    }

    // 绑定 socket 到端口
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cerr << "Bind failed!" << std::endl;
        close(server_fd);
        return 1;
    }

    // 监听连接
    if (listen(server_fd, 3) < 0)
    {
        std::cerr << "Listen failed!" << std::endl;
        close(server_fd);
        return 1;
    }

    std::cout << "Server is listening on " << server_ip << ":" << server_port << std::endl;

    // 接受客户端连接
    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_fd < 0)
    {
        std::cerr << "Accept failed!" << std::endl;
        close(server_fd);
        return 1;
    }

    // 打开视频文件
    cv::VideoCapture cap(video_path);
    if (!cap.isOpened())
    {
        std::cerr << "Failed to open video file!" << std::endl;
        close(client_fd);
        close(server_fd);
        return 1;
    }

    // 读取第一帧
    cv::Mat frame;
    if (!cap.read(frame))
    {
        std::cerr << "Failed to read frame from video file!" << std::endl;
        close(client_fd);
        close(server_fd);
        return 1;
    }

    // 使用 JPEG 编码压缩帧并存储在缓冲区
    std::vector<uchar> buffer;
    cv::imencode(".jpg", frame, buffer);

    // 发送图片大小
    std::size_t image_size = buffer.size();
    send(client_fd, &image_size, sizeof(image_size), 0);

    // 发送图片数据
    send(client_fd, buffer.data(), image_size, 0);

    // 关闭连接
    close(client_fd);
    close(server_fd);
    return 0;
}
