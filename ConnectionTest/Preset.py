import cv2
import time
import mediapipe as mp

settings = {
    'file_name': 'test_video.mp4',
    'frame_rate': 30,
    'frame_width': 1920,
    'frame_height': 1080,
    'codec': 'avc1',
    'time': 30
}

import asyncio 
from bleak import BleakScanner, BleakClient
import csv

DEVICE_ADDRESSS = "880747D3-2C1B-25C6-3A7E-FBC1871EE8B1"
RX_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
TX_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

from multiprocessing import Process, Queue
import threading

import pandas as pd
import matplotlib.pyplot as plt
from datetime import datetime