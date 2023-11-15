import tkinter as tk
from tkinter import Label, Button
import cv2
from PIL import Image, ImageTk
from threading import Thread
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import numpy as np
import asyncio
import csv
from bleak import BleakClient
import threading
import queue

DEVICE_ADDRESS = "880747D3-2C1B-25C6-3A7E-FBC1871EE8B1"
RX_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
TX_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

class WebcamApp:
    def __init__(self, window, window_title, video_source=0):
        self.window = window
        self.window.title(window_title)
        self.window.geometry("640x800")

        self.video_graph_container = tk.Frame(self.window)
        self.video_graph_container.pack(side=tk.TOP, padx=10, pady=10)

        self.videoFrame = tk.Frame(self.video_graph_container, width = 640, height = 400)
        self.videoFrame.pack(side=tk.TOP, padx=10, pady=10)

        # Create a label for video display
        self.canvas = Label(self.videoFrame)
        self.canvas.pack(padx=10, pady=10, fill=tk.BOTH, expand=True)

        self.graphFrame = tk.Frame(self.video_graph_container, width = 640, height = 120, bg="blue")
        self.graphFrame.pack(side=tk.BOTTOM, padx=10, pady=10)

        self.btnFrame = tk.Frame(self.window, width = 640, height = 100, bg="green")
        self.btnFrame.pack(side=tk.BOTTOM, padx=10, pady=10)

        self.btnFrame_ble_connect = tk.Button(self.btnFrame, text="BLE Connect", command=self.start_ble_connect)
        self.btnFrame_ble_connect.grid(row=0, column=0, padx=10, pady=10)

        self.btnFrame_data_notify = tk.Button(self.btnFrame, text="BLE Notify", command=self.notify_callback)
        self.btnFrame_data_notify.grid(row=0, column=1, padx=10, pady=10)

        self.btnFrame_video_record = tk.Button(self.btnFrame, text="Start Recording", command=self.toggle_recording)
        self.btnFrame_video_record.grid(row=0, column=2, padx=10, pady=10)

        self.video_source = video_source
        self.vid = cv2.VideoCapture(self.video_source)
        
        self.is_recording = False
        self.out = None

       # 그래프 초기화
        self.figure = Figure(figsize=(5, 4), dpi=100)
        self.ax = self.figure.add_subplot(111)
        self.canvas = FigureCanvasTkAgg(self.figure, self.window)
        self.canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=1)

# 데이터 저장을 위한 리스트 초기화
        self.x_data = []
        self.y_data = [[] for _ in range(5)]  # 5개의 센서 값들을 위한 리스트
        self.graph_data = []
        self.x_values = []
        # self.update_graph()  # Update the graph

        self.update_video()
        self.window.mainloop()

    def run_asyncio_coroutine(self, coroutine):
        # 통신을 위한 큐 생성
        self.queue = queue.Queue()

        # 별도의 스레드에서 asyncio 루프 시작
        threading.Thread(target=self._asyncio_thread, args=(coroutine,)).start()

        # 메인 스레드에서 큐를 주기적으로 확인
        self.window.after(100, self.check_asyncio_queue)

    def _asyncio_thread(self, coroutine):
        loop = asyncio.new_event_loop()
        asyncio.set_event_loop(loop)
        try:
            result = loop.run_until_complete(coroutine)
            self.queue.put(("success", result))
        except Exception as e:
            self.queue.put(("error", str(e)))
        finally:
            loop.close()

    def check_asyncio_queue(self):
        try:
            status, result = self.queue.get_nowait()
            if status == "success":
                print("Coroutine completed successfully with result:", result)
            elif status == "error":
                print("Coroutine raised an exception:", result)
        except queue.Empty:
            # 큐가 비어 있으면 일정 시간 후에 다시 확인
            self.window.after(100, self.check_asyncio_queue)

    # def run_asyncio_coroutine(self, coroutine):
    #     loop = asyncio.new_event_loop()
    #     asyncio.set_event_loop(loop)
    #     return loop.run_until_complete(coroutine)
    
    def start_ble_connect(self):
        self.run_asyncio_coroutine(self.connect_ble())
    # Method to connect to a BLE device
    async def connect_ble(self):
        f = open("test.csv", 'w')
        global writer
        writer = csv.writer(f)
        
        async with BleakClient(DEVICE_ADDRESS) as client:
            print('connected')
            services = await client.get_services()
            for service in services:
                print('service uuid:', service.uuid)
                for characteristic in service.characteristics:
                    print('  uuid:', characteristic.uuid)
                    print('  handle:', characteristic.handle)
                    print('  properties: ', characteristic.properties)
                    if characteristic.uuid == TX_UUID:
                        if 'notify' in characteristic.properties:
                            print('try to activate notify.')
                            await client.start_notify(characteristic, self.notify_callback)
            if client.is_connected:
                await asyncio.sleep(60)
                print('try to deactivate notify.')
                await client.stop_notify(TX_UUID)
        f.close()
        print('disconnect')


    def notify_callback(self, sender: int, data: bytearray):
        print('sender: ', sender, 'data: ', data)
        encodedData, tmp = list(), ""
        for b in data:
            encodedCh = chr(b)
            if encodedCh == '_':
                encodedData.append(tmp)
                tmp = ""
            elif encodedCh == ',':
                encodedData.append(tmp)
                tmp = ""
            elif encodedCh == '\x00':
                encodedData.append(tmp) 
            else: tmp += encodedCh
        print(encodedData)
        writer.writerow(encodedData[:6])
        # if len(encodedData) > 6: 
        #     writer.writerow(encodedData[:6])
        #     writer.writerow(encodedData[7:-1])
        # else: writer.writerow(encodedData[:-1])
        self.update_graph(encodedData[:6])


    def update_video(self):
        if self.is_recording:
            ret, frame = self.vid.read()
            if ret:
                self.photo = self.get_image_from_frame(frame)
                self.canvas.config(image=self.photo)

                if self.is_recording:
                    self.out.write(frame)
        
        self.window.after(10, self.update_video)

    @staticmethod
    def get_image_from_frame(frame):
        frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        image = Image.fromarray(frame)
        photo = ImageTk.PhotoImage(image=image)
        return photo

    def toggle_recording(self):
        if not self.is_recording:
            self.start_recording()
        else:
            self.stop_recording()

    def start_recording(self):
        if not self.is_recording:
            self.is_recording = True
            fourcc = cv2.VideoWriter_fourcc(*'XVID')
            self.out = cv2.VideoWriter('output.avi', fourcc, 20.0, (int(self.vid.get(3)), int(self.vid.get(4))))
            self.btnFrame_video_record.config(text="Stop Recording")
        else:
            self.stop_recording()

    def stop_recording(self):
        if self.is_recording:
            self.is_recording = False
            if self.out:
                self.out.release()
            self.btnFrame_video_record.config(text="Start Recording")
            self.vid.release()  # 비디오 캡처 자원 해제

    def update_graph(self, data):
        # 데이터 추출
        x = float(data[0])  # 시간 값
        y_values = [float(val) for val in data[1:]]  # 센서 값들

        # 데이터 저장
        self.x_data.append(x)
        for i, y in enumerate(y_values):
            self.y_data[i].append(y)

        # 그래프에 데이터 추가
        self.ax.clear()  # 그래프를 초기화
        labels = ['Sensor0', 'Sensor1', 'Sensor2', 'Sensor3', 'Sensor4']
        for i, y_vals in enumerate(self.y_data):
            self.ax.plot(self.x_data, y_vals, label=labels[i])

        # 그래프 설정
        self.ax.set_xlabel('time')
        self.ax.set_ylabel('data values')
        self.ax.grid()
        self.ax.legend()

        # 그래프를 다시 그림
        self.canvas.draw()

        self.window.after(100, self.update_graph)  # 이 부분은 필요에 따라 조정하실 수 있습니다.

root = tk.Tk()
app = WebcamApp(root, "Webcam Display with Recording")