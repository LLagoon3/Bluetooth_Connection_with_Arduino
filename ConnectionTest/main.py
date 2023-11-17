from Preset import *
from Camera_Record import processVideo
from BLE_connection import notifyRead, write


def _makeGraph():
    df = pd.read_csv('test.csv', header=0, index_col = 0, names=['0', '1', '2', '3', '4']).sort_index()
    index_data = list(map(lambda x: x /1000, df.index.values.tolist()))
    plt.plot(index_data, df['0'].values.tolist(),
             index_data, df['1'].values.tolist(),
             index_data, df['2'].values.tolist(),
             index_data, df['3'].values.tolist(),
             index_data, df['4'].values.tolist())
    plt.show()
    return 

def makeGraph(file_name):
    import matplotlib.pyplot as plt
    figure = plt.figure(figsize=(5, 4), dpi=100)
    ax = figure.add_subplot(111)
    labels = ['Sensor0', 'Sensor1', 'Sensor2', 'Sensor3', 'Sensor4']
    df = pd.read_csv(file_name, header=0, index_col=0, names=labels).sort_index()
    index_data = list(map(lambda x: x / 1000, df.index.values.tolist()))
    x_data = index_data  # 시간 값
    y_data = []
    for l in labels:
        y_data.append(df[l].values.tolist())
    for i, y_vals in enumerate(y_data):
        ax.plot(x_data, y_vals, label=labels[i])
    ax.set_xlabel('time')
    ax.set_ylabel('data values')
    ax.grid()
    ax.legend()
    plt.show()

if __name__ == "__main__":
    # video = processVideo(settings, True)
    # START, END = 0, 100000000
    # result = Queue()
    # th1 = Process(target=video.convertVideo(), args=(1, START, END//2, result))
    # th2 = Process(target=notifyRead, args=(2, END//2, END, result))
    # th1.start()
    # th2.start()
    # th1.join()
    # th2.join()
    # print(result)
    
    
    # loop = asyncio.get_event_loop()
    # loop.run_until_complete(notifyRead(DEVICE_ADDRESSS))
    
    file_name = str(datetime.now().strftime('%Y-%m-%d_%H-%M')) + ".csv"
    file_name = "t.csv"
    loop = asyncio.get_event_loop()
    loop.run_until_complete(write(DEVICE_ADDRESSS))
    # loop.run_until_complete(notifyRead(DEVICE_ADDRESSS, file_name))
    # makeGraph(file_name)