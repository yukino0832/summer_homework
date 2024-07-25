#include <iostream>
#include "../include/Pose.hpp"

Pose Gimbal(Pose Camera, double X)
{
    Pose pose_variation = Pose({X, 0, 0}, Quaternion(1.57, 0, 1.57));
    // Pose pose_variation = Pose({X, 0, 0}, Quaternion(1, 0, 0, 0));
    return Camera + pose_variation;
}

Pose Shooter(Pose Gimbal, double X, double Z)
{
    Pose pose_variation = Pose({-X, 0, -Z}, Quaternion(1, 0, 0, 0));
    return Gimbal + pose_variation;
}

Pose Odom(Pose Gimbal, double roll, double pitch, double yaw)
{
    Pose pose_variation = Pose({0, 0, 0}, Quaternion(roll, pitch, yaw));
    return Gimbal + pose_variation;
}

int main()
{

    double X = 1.3, Y = 0.2, Z = -0.1, roll = 0, pitch = -0.23, yaw = 1.7;
    Pose Camera = Pose({X, Y, Z}, Quaternion(roll, pitch, yaw));
    std::string s;
    getline(std::cin, s);

    double X_Camera_Gimbal = 0.2;   // Camera相对于Gimbal的X位移量
    double roll_Gimbal_Odom = 0, pitch_Gimbal_Odom = 1.3, yaw_Gimbal_Odom = 0.7;
    double X_Gimbal_Shooter = 0.18, Z_Gimbal_Shooter = -0.05;


    while(s != "quit")
    {
        if (s == "from Camera to Armor:")
        {
            Camera.print();
        }

        else if (s == "from Gimbal to Armor:")
        {

            Gimbal(Camera, X_Camera_Gimbal).print();
        }

        else if (s == "from Odom to Armor:")
        {
            Pose pose_Gimbal = Gimbal(Camera, X_Camera_Gimbal);
            Odom(pose_Gimbal, roll_Gimbal_Odom, pitch_Gimbal_Odom, yaw_Gimbal_Odom).print();
        }

        else if (s == "from Shooter to Armor:")
        {
            Pose pose_Gimbal = Gimbal(Camera, X_Camera_Gimbal);
            Shooter(pose_Gimbal, X_Gimbal_Shooter, Z_Gimbal_Shooter).print();
        }

        // std::cin.ignore();
        getline(std::cin, s);
        
    }

    return 0;
}
