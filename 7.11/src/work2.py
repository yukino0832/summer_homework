import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# 读取数据
data = pd.read_csv('/home/zcy/ZCY/hdu/rm/7.11/kalman_filter/stock_prices.csv')
days = data['Day']
prices = data['Price']

# 参数设置
n = len(prices)
Q = np.array([[1, 0], [0, 1]])  # 过程噪声协方差
R = np.array([[1]])              # 观测噪声协方差
x = np.zeros((n+1, 2))            # 状态变量初始化
P = np.eye(2)                   # 状态协方差初始化

# 初始状态
x[0] = [prices[0], 0]

# 卡尔曼滤波
predicted_prices = []
for t in range(n):
    # 预测步骤
    if t >= 1:
        F = np.array([[1, 1], [0, 1]])  # 状态转移矩阵
        x[t] = np.dot(F, x[t-1])  # 状态转移
        if t == n-1:
            print(x[t][0])
        P = np.dot(np.dot(F, P), F.T) + Q  # 更新协方差

    # 更新步骤
    H = np.array([[1, 0]])  # 观测矩阵
    S = np.dot(np.dot(H, P), H.T) + R  # 观测协方差
    K = np.dot(P, H.T) / S  # 卡尔曼增益

    x[t] = x[t] + np.dot(K, (prices[t] - np.dot(H, x[t])).T).T  # 更新状态
    P = P - np.dot(K, np.dot(H, P))  # 更新协方差

    predicted_prices.append(x[t][0])
    
# 绘制结果
plt.plot(days, prices, label='Actual Prices')
plt.plot(days, predicted_prices, label='Kalman Filter Predictions', linestyle='--')
plt.xlabel('Day')
plt.ylabel('Stock Price')
plt.legend()
plt.title('Kalman Filter Stock Price Prediction')
plt.show()