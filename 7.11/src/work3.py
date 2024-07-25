import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

true_states = pd.read_csv('/home/zcy/ZCY/hdu/rm/7.11/kalman_filter/true_states.csv')
measurements = pd.read_csv('/home/zcy/ZCY/hdu/rm/7.11/kalman_filter/measurements.csv')

# 参数设置
dt = 1.0  # 时间步长
F = np.array([[1, 0, 0, dt, 0, 0], 
              [0, 1, 0, 0, dt, 0], 
              [0, 0, 1, 0, 0, dt], 
              [0, 0, 0, 1, 0, 0], 
              [0, 0, 0, 0, 1, 0], 
              [0, 0, 0, 0, 0, 1]])  # 状态转移矩阵

B = np.array([[0.5*dt**2, 0, 0], 
              [0, 0.5*dt**2, 0], 
              [0, 0, 0.5*dt**2], 
              [dt, 0, 0], 
              [0, dt, 0], 
              [0, 0, dt]])  # 控制输入矩阵

H = np.array([[1, 0, 0, 0, 0, 0], 
              [0, 1, 0, 0, 0, 0], 
              [0, 0, 1, 0, 0, 0]])  # 观测矩阵

Q = np.eye(6)  # 过程噪声协方差矩阵
R = np.eye(3)  # 观测噪声协方差矩阵

x = np.zeros((6, 1))  # 初始状态
P = np.eye(6)  # 初始协方差矩阵

# 卡尔曼滤波
predicted_states = []
for t in range(len(measurements)):
    u = np.zeros((3, 1))  # 由于没有加速度测量，设为0
    z = measurements.loc[t, ['x', 'y', 'z']].values.reshape((3, 1))
    
    # 预测步骤
    x = np.dot(F, x) + np.dot(B, u)
    P = np.dot(F, np.dot(P, F.T)) + Q

    # 更新步骤
    y = z - np.dot(H, x)  # 计算创新
    S = np.dot(H, np.dot(P, H.T)) + R  # 计算创新协方差
    K = np.dot(P, np.dot(H.T, np.linalg.inv(S)))  # 计算卡尔曼增益
    x = x + np.dot(K, y)  # 更新状态
    P = P - np.dot(K, np.dot(H, P))  # 更新协方差矩阵

    predicted_states.append(x.flatten())

predicted_states = np.array(predicted_states)

# 绘制结果
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
ax.plot(true_states['x'], true_states['y'], true_states['z'], label='True State')
ax.plot(predicted_states[:, 0], predicted_states[:, 1], predicted_states[:, 2], label='Predicted State', linestyle='--')
ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_zlabel('Z')
ax.legend()
plt.show()
