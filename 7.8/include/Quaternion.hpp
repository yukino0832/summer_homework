#include <iostream>
#include <cmath>
#include <array>

class Quaternion
{
public:

    double w, x, y, z;

    Quaternion(double w, double x, double y, double z): w(w), x(x), y(y), z(z) { }

    Quaternion(double roll, double pitch, double yaw)  // from EulerAngles
    {
        double cosr = std::cos(roll / 2);
        double sinr = std::sin(roll / 2);
        double cosp = std::cos(pitch / 2);
        double sinp = std::sin(pitch / 2);
        double cosy = std::cos(yaw / 2);
        double siny = std::sin(yaw / 2);
        this->w = cosr * cosp * cosy + sinr * sinp * siny;
        this->x = sinr * cosp * cosy - cosr * sinp * siny;
        this->y = cosr * sinp * cosy + sinr * cosp * siny;
        this->z = cosr * cosp * siny - sinr * sinp * cosy;
    }

    Quaternion operator+(const Quaternion& other) const
    {
        return Quaternion(w + other.w, x + other.x, y + other.y, z + other.z);
    }

    Quaternion operator-(const Quaternion &other) const
    {
        return Quaternion(w - other.w, x - other.x, y - other.y, z - other.z);
    }

    Quaternion operator*(const Quaternion &other) const
    {
        return Quaternion(w * other.w - x * other.x - y * other.y - z * other.z,
                          w * other.x + x * other.w + y * other.z - z * other.y,
                          w * other.y - x * other.z + y * other.w + z * other.x,
                          w * other.z + x * other.y - y * other.x + z * other.w);
    }

    double abs()
    {
        return sqrt(w * w + x * x + y * y + z * z);
    }

    Quaternion conjugate() const // 求共轭
    {
        return Quaternion(w, -x, -y, -z);
    }

    Quaternion inverse() const // 求逆
    {
        double absQ = w * w + x * x + y * y + z * z;
        return Quaternion(w / absQ, -x / absQ, -y / absQ, -z / absQ);
    }

    void toEularAngles(double& roll, double& pitch, double& yaw)
    {
        double sinr_cosp = 2 * (w * x + y * z);
        double cosr_cosp = 1 - 2 * (x * x + y * y);
        roll = std::atan2(sinr_cosp, cosr_cosp);

        double sinp = 2 * (w * y - z * x);
        if (std::abs(sinp) >= 1)
            pitch = std::asin(std::copysign(M_PI / 2, sinp)); // 超过1就用正负90度代替(符号取决于sinp)
        else
            pitch = std::asin(sinp);

        double siny_cosp = 2 * (w * z + x * y);
        double cosy_cosp = 1 - 2 * (y * y + z * z);
        yaw = std::atan2(siny_cosp, cosy_cosp);
        
    }

    std::array<double, 3> rotate(const std::array<double, 3> &vector) const // 使用四元数旋转向量
    {
        Quaternion q_vector(0, vector[0], vector[1], vector[2]);
        Quaternion rotated_vector = (*this * q_vector) * this->inverse();
        return {rotated_vector.x, rotated_vector.y, rotated_vector.z};
    }

    void getQuaternion(double& w, double& x, double& y, double& z)
    {
        w = (*this).w;
        x = (*this).x;
        y = (*this).y;
        z = (*this).z;
    }

};