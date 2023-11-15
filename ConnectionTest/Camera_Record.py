from Preset import *



class processVideo:
    def __init__(self, settings: dict, record: bool) -> None:
        if not record: self.settings = self.getVideoInfo(settings = settings)
        else: self.settings = settings
        self.record = record
        # self.convertVideo()
            
    def getVideoInfo(self, settings: dict) -> dict:
        cap = cv2.VideoCapture(settings['file_name'])
        if not cap.isOpened():
            print("Could not Open File")
            exit(0)
        new_settings = {
            'file_name': settings['file_name'],
            'frame_rate': cap.get(cv2.CAP_PROP_FPS),
            'frame_width': int(cap.get(cv2.CAP_PROP_FRAME_WIDTH)),
            'frame_height': int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT)),
            'codec': settings['codec'],
            'time': settings['time']
        }
        return new_settings
    
    def convertVideo(self, camera_index = 1) -> None:
        mpDraw = mp.solutions.drawing_utils
        mpPose = mp.solutions.pose
        pose = mpPose.Pose()

        out = cv2.VideoWriter(self.settings['file_name'] if self.record else "Convert_" + self.settings['file_name'], 
                            cv2.VideoWriter_fourcc(*self.settings['codec']), 
                            self.settings['frame_rate'], 
                            (self.settings['frame_width'], self.settings['frame_height'])
                            )
        
        if self.record: cap = cv2.VideoCapture(camera_index)  # 카메라 인덱스 (일반적으로 0이 내장 카메라를 의미) -> 0 안되면 1로 설정해보기
        else: cap = cv2.VideoCapture('./' + self.settings['file_name']) 
        
        start_time = time.time()
        end_time = start_time + self.settings['time']
        frame_arr = []
        
        while cap.isOpened():
            
            ret, frame = cap.read()  # 프레임 읽기
            if not ret:
                print("Load Video Fail")
                break
            
            if not self.record:
                imgRGB = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
                results = pose.process(imgRGB)
                if results.pose_landmarks:
                    mpDraw.draw_landmarks(frame, results.pose_landmarks, mpPose.POSE_CONNECTIONS)
                    for id, lm in enumerate(results.pose_landmarks.landmark):
                        h, w, c = frame.shape
                        cx, cy = int(lm.x*w), int(lm.y*h)
                        cv2.circle(frame, (cx, cy), 5, (255, 0, 0), cv2.FILLED)
                cv2.putText(frame, "Text" , (10,10), cv2.FONT_HERSHEY_SIMPLEX, 1, (0,255,0), 2)
            # 현재 화면을 OpenCV 창에 표시
            cv2.imshow('HeartSignal', frame)
            cv2.waitKey(1)
            if time.time() > end_time:
                break
            
            frame_arr.append(frame)

        for f in frame_arr: out.write(f)
        # 해제 및 창 닫기
        cap.release()
        out.release()
        cv2.destroyAllWindows()

        print("Convert Complete")






