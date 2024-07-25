#include <iostream>
#include <stdio.h>
#include <array>
#include "Quaternion.hpp"

class Pose
{
public:
    std::array<double, 3> position;
    Quaternion rotation;

    Pose(const std::array<double, 3> &pos = {0, 0, 0}, const Quaternion &rot = Quaternion(1, 0, 0, 0))
        : position(pos), rotation(rot) {}

    // 组合两个位姿
    Pose operator+(const Pose &other) const
    {
        std::array<double, 3> newpos = position;
        Quaternion Q_newpos = Quaternion(0, newpos[0], newpos[1], newpos[2]);
        Quaternion Q_rot_newpos = other.rotation * Q_newpos * (other.rotation).inverse();
        Q_rot_newpos.x += other.position[0];
        Q_rot_newpos.y += other.position[1];
        Q_rot_newpos.z += other.position[2];
        newpos[0] = Q_rot_newpos.x;
        newpos[1] = Q_rot_newpos.y;
        newpos[2] = Q_rot_newpos.z;
        return Pose(newpos, rotation * other.rotation);
    }

    void print()
    {
        double roll, pitch, yaw;
        rotation.toEularAngles(roll, pitch, yaw);
        printf("XYZ:\n");
        printf("%lf %lf %lf\n", position[0], position[1], position[2]);
        printf("YPR:\n");
        printf("%lf %lf %lf\n", yaw, pitch, roll);
    }
};
