import numpy as np
import matplotlib.pyplot as plt

# 读取数据
data = np.loadtxt('/home/zcy/ZCY/hdu/rm/7.11/kalman_filter/homework_data_3.txt')

# 分离数据
time = data[:, 0]
value = data[:, 1]

# 卡尔曼滤波实现(Q为过程噪声方差，R为观测噪声方差)
def kalman_filter(data, Q, R):
    n = len(data)
    X_update = np.zeros(n)
    P_update = np.zeros(n)

    X_prior = np.zeros(n)
    P_prior = np.zeros(n)

    K = np.zeros(n)

    X_update[0] = data[0]
    P_update[0] = 1

    for k in range(1, n):
        X_prior[k] = X_update[k-1]
        P_prior[k] = P_update[k-1] + Q    # 一维没有状态转移

        K[k] = P_prior[k] / (P_prior[k] + R)
        X_update[k] = X_prior[k] + K[k] * (data[k] - X_prior[k])
        P_update[k] = (1 - K[k]) * P_prior[k]

    return X_update

# # 设置卡尔曼滤波参数(data1)
# Q = 1e-5
# R = 0.1 ** 2

# # 设置卡尔曼滤波参数(data2)
# Q = 0.9e-6
# R = 0.1 ** 4.9

# 设置卡尔曼滤波参数(data3)
Q = 1e-3
R = 0.1 ** 2

# # 设置卡尔曼滤波参数(data4)
# Q = 1e-5
# R = 0.1 ** 2

# 使用卡尔曼滤波
kalman_filtered_data = kalman_filter(value, Q, R)

# 使用多项式拟合平滑后的数据
coefficients = np.polyfit(time, kalman_filtered_data, 2)
polynomial = np.poly1d(coefficients)

# 生成拟合曲线
fitted_values = polynomial(time)

# 绘制原始数据、平滑数据和拟合曲线
plt.figure(figsize=(12, 6))
plt.plot(time, value, label='Original Data', color='blue', alpha=0.5)
plt.plot(time, kalman_filtered_data, label='Kalman Filtered Data', color='red')
plt.plot(time, fitted_values, label='Fitted Curve', color='green')
plt.xlabel('Time')
plt.ylabel('Measurements')
plt.legend()
plt.title('Original Data, Kalman Filtered Data, and Fitted Curve')
plt.show()

# 输出拟合方程
print("Fitted Polynomial Equation:")
print(polynomial)
