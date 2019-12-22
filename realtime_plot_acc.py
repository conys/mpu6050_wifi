# -*- coding: utf-8 -*-

import numpy as np
import matplotlib.pyplot as plt
from scipy import signal
from socket import socket, AF_INET, SOCK_DGRAM

HOST = ''
PORT = 9000 # 受信ポート番号

N = 10000          # サンプル数
dt = 0.001          # サンプリング間隔
freq = np.linspace(0, 1.0/dt, N) # 周波数軸

if __name__ == "__main__":
    
    s = socket(AF_INET, SOCK_DGRAM)
    s.bind((HOST, PORT))

    data_x = [0] * 10000
    data_y = [0] * 10000
    data_z = [0] * 10000
    t_now = 0
    
    try:
        fig = plt.figure(figsize=(12,8))
        while True:
            data_x[0:9000] = data_x[1000:10000]
            data_y[0:9000] = data_y[1000:10000]
            data_z[0:9000] = data_z[1000:10000]
            t_now += 1000
            t = list(range(t_now - 10000, t_now))
            string = ""

            for j in range(1000):
                msg, address = s.recvfrom(8192) # データ受信
                data = msg.decode('utf-8').split(",") # 受信データを配列化 [acc_x, acc_y, acc_z, nowtime[ms], nowtime[us]]
                data = [i.strip() for i in data]
                data_x[9000 + j] = int(data[0]) / 16384 # 数値を加速度に変換
                data_y[9000 + j] = int(data[1]) / 16384
                data_z[9000 + j] = int(data[2]) / 16384
                
                if j == 0:
                    string += "start %s %s  " % (data[3], data[4])
                elif j == 999:
                    string += "end %s %s" % (data[3], data[4])
            print(string)
            FFT_x = np.fft.fft(data_x) / (N/2) # 離散フーリエ変案＆規格化
            Amp_x = np.abs(FFT_x)
            Amp_x[0] /= 2
            FFT_y = np.fft.fft(data_y) / (N/2) # 離散フーリエ変案＆規格化
            Amp_y = np.abs(FFT_y)
            Amp_y[0] /= 2
            FFT_z = np.fft.fft(data_z) / (N/2) # 離散フーリエ変案＆規格化
            Amp_z = np.abs(FFT_z)
            Amp_z[0] /= 2
            
            fft_plot_xlim = 20 # FFT描画範囲[Hz]
            
            # acc_x 加速度x方向
            plt.subplot(231)
            plt.cla()
            plt.plot(np.array(t)/1000, data_x) # データをプロット
            plt.xlabel("time [s]")
            plt.ylabel("acc_x [g]")

            # FFT_x フーリエ変x方向
            plt.subplot(234)
            plt.cla()
            plt.plot(freq, Amp_x)
            # plt.xlim(0, 1.0/dt/2)
            plt.xlim(0, fft_plot_xlim)
            plt.xlabel("Frequency [Hz]")
            plt.ylabel("Amplitude_x")

            # data_y 加速度y方向
            plt.subplot(232)
            plt.cla()
            plt.plot(np.array(t)/1000, data_y) # データをプロット
            plt.xlabel("time [s]")
            plt.ylabel("acc_y [g]")

            # FFT_y フーリエ変y方向
            plt.subplot(235)
            plt.cla()
            plt.plot(freq, Amp_y)
            # plt.xlim(0, 1.0/dt/2)
            plt.xlim(0, fft_plot_xlim)
            plt.xlabel("Frequency [Hz]")
            plt.ylabel("Amplitude_y")

            # data_z 加速度z方向
            plt.subplot(233)
            plt.cla()
            plt.plot(np.array(t)/1000, data_z) # データをプロット
            plt.xlabel("time [s]")
            plt.ylabel("acc_z [g]")

            # FFT_z フーリエ変z方向
            plt.subplot(236)
            plt.cla()
            plt.plot(freq, Amp_z)
            # plt.xlim(0, 1.0/dt/2)
            plt.xlim(0, fft_plot_xlim)
            plt.xlabel("Frequency [Hz]")
            plt.ylabel("Amplitude_z")

            fig.subplots_adjust(wspace=0.3, hspace=0.3)
            plt.draw() # グラフを画面に表示開始
            plt.pause(1) # 1秒だけ表示を継続

    except KeyboardInterrupt:
        print("exit")
        s.close()


