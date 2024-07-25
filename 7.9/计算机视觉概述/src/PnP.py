import cv2
import numpy as np
import os

# 设置棋盘格的大小
chessboard_size = (6, 4)

# 准备世界坐标系中的棋盘格角点坐标
objp = np.zeros((chessboard_size[0] * chessboard_size[1], 3), np.float32)
objp[:, :2] = np.mgrid[0:chessboard_size[0], 0:chessboard_size[1]].T.reshape(-1, 2)

# 加载相机矩阵和畸变系数
camera_matrix = [[9080.619849, 0.000000, 715.491593], [0.000000, 9358.782652, 349.371211], [0.000000, 0.000000, 1.000000]]
dist_coeffs = [5.691436, 3463.561528, 0.244806, 0.040067, -2.980298]

# 打开摄像头
cap = cv2.VideoCapture(0)

if not cap.isOpened():
    print("Error: Could not open video stream from camera.")
    exit()

while True:
    ret, frame = cap.read()
    if not ret:
        print("Error: Could not read frame from camera.")
        break

    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    # 寻找棋盘格角点
    ret, corners = cv2.findChessboardCorners(gray, chessboard_size, None)

    if ret:
        # 提高角点的准确度
        corners2 = cv2.cornerSubPix(gray, corners, (11, 11), (-1, -1), 
                                    (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001))

        # 使用 solvePnP 求解位姿
        _, rvec, tvec = cv2.solvePnP(objp, corners2, camera_matrix, dist_coeffs)

        # 绘制坐标轴
        axis = np.float32([[3, 0, 0], [0, 3, 0], [0, 0, -3]]).reshape(-1, 3)
        imgpts, _ = cv2.projectPoints(axis, rvec, tvec, camera_matrix, dist_coeffs)

        # 绘制结果
        corner = tuple(corners2[0].ravel())
        frame = cv2.line(frame, corner, tuple(imgpts[0].ravel()), (255, 0, 0), 5)
        frame = cv2.line(frame, corner, tuple(imgpts[1].ravel()), (0, 255, 0), 5)
        frame = cv2.line(frame, corner, tuple(imgpts[2].ravel()), (0, 0, 255), 5)

    # 显示当前帧
    cv2.imshow('Pose Estimation', frame)

    # 按下 'q' 键退出循环
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# 释放摄像头并关闭所有窗口
cap.release()
cv2.destroyAllWindows()
