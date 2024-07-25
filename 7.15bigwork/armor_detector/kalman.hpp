#include <iostream>
#include <Eigen/Dense>

Eigen::VectorXd f(const Eigen::VectorXd &x)
{
    Eigen::VectorXd x_new = x;
    x_new.head<3>() += x.tail<3>();
    return x_new;
}

Eigen::MatrixXd F_jacobian(const Eigen::VectorXd &x)
{
    Eigen::MatrixXd F = Eigen::MatrixXd::Identity(6, 6);
    F(0, 3) = 1;
    F(1, 4) = 1;
    F(2, 5) = 1;
    return F;
}

Eigen::VectorXd h(const Eigen::VectorXd &x)
{
    return x.head<3>();
}

Eigen::MatrixXd H_jacobian(const Eigen::VectorXd &x)
{
    Eigen::MatrixXd H = Eigen::MatrixXd::Zero(3, 6);
    H(0, 0) = 1;
    H(1, 1) = 1;
    H(2, 2) = 1;
    return H;
}

void initializeKalmanFilter(Eigen::VectorXd &x, Eigen::MatrixXd &P, Eigen::MatrixXd &R, Eigen::MatrixXd &Q)
{
    // 状态向量 (位置和速度)
    x = Eigen::VectorXd(6);
    x << 0, 0, 0, 0, 0, 0;

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

void predict(Eigen::VectorXd &x, Eigen::MatrixXd &P, const Eigen::MatrixXd &Q)
{
    Eigen::MatrixXd F = F_jacobian(x);
    x = f(x);
    P = F * P * F.transpose() + Q;
}

void update(Eigen::VectorXd &x, Eigen::MatrixXd &P, const Eigen::MatrixXd &R, const Eigen::VectorXd &z)
{
    Eigen::MatrixXd H = H_jacobian(x);
    Eigen::VectorXd y = z - h(x);
    Eigen::MatrixXd S = H * P * H.transpose() + R;
    Eigen::MatrixXd K = P * H.transpose() * S.inverse();
    x = x + K * y;
    P = (Eigen::MatrixXd::Identity(x.size(), x.size()) - K * H) * P;
}

void printState(const Eigen::VectorXd &x, const Eigen::MatrixXd &P)
{
    std::cout << "Estimated state: \n"
              << x << std::endl;
    std::cout << "Estimated covariance: \n"
              << P << std::endl;
}
