import cv2
import numpy as np
import os
import time as ts
# 设置棋盘格的大小
chessboard_size = (6,4)  # 棋盘格内角点的数量（9x6）
frame_count = 60  # 要保存的标定图像数量

# 创建保存标定图像的文件夹
if not os.path.exists('calibration_images'):
    os.makedirs('calibration_images')

# 设置世界坐标系中的棋盘格角点坐标
objp = np.zeros((chessboard_size[0] * chessboard_size[1], 3), np.float32)
objp[:, :2] = np.mgrid[0:chessboard_size[0], 0:chessboard_size[1]].T.reshape(-1, 2)

# 存储世界坐标系和图像坐标系中的角点
objpoints = []  # 世界坐标系中的角点
imgpoints = []  # 图像坐标系中的角点

# 打开摄像头
cap = cv2.VideoCapture(0)

if not cap.isOpened():
    print("Error: Could not open video stream from camera.")
    exit()

saved_images = 0

while saved_images < frame_count:
    ret, frame = cap.read()
    
    if not ret:
        print("Error: Could not read frame from camera.")
        break

    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    # 寻找棋盘格角点
    ret, corners = cv2.findChessboardCorners(gray, chessboard_size, None)

    # 如果找到足够的角点，则将其存储
    if ret:
        objpoints.append(objp)
        imgpoints.append(corners)
        ts.sleep(1)

        # 绘制并显示角点
        frame = cv2.drawChessboardCorners(frame, chessboard_size, corners, ret)
        saved_images += 1
        cv2.imwrite(f'calibration_images/calibration_image_{saved_images}.jpg', frame)
        print(f'Saved calibration_image_{saved_images}.jpg')

    # 显示当前帧
    cv2.imshow('Camera Calibration', frame)

    # 按下 'q' 键退出循环
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
    

# 释放摄像头并关闭所有窗口
cap.release()
cv2.destroyAllWindows()

# 标定摄像机
ret, camera_matrix, dist_coeffs, rvecs, tvecs = cv2.calibrateCamera(objpoints, imgpoints, gray.shape[::-1], None, None)

# 打印标定结果
# print("Camera matrix:")
# print(camera_matrix)

# print("\nDistortion coefficients:")
# print(dist_coeffs)
print(ret)

np.savetxt("distortion.txt", camera_matrix,fmt="%f")
np.savetxt("dist_coeffs.txt", dist_coeffs,fmt="%f")
# np.save('camera_matrix.npy', camera_matrix)
# np.save('dist_coeffs.npy', dist_coeffs)



print("Calibration matrices have been saved to camera_matrix.npy and dist_coeffs.npy")
