from Preset import *
TMP_TIME = 0


def notify_callback(sender: int, data: bytearray):
    global TMP_TIME
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
    
    
    if(TMP_TIME < int(encodedData[0]) and int(encodedData[0]) != 60000):
        TMP_TIME = int(encodedData[0])
        print(encodedData)
        writer.writerow(encodedData[:6])
    else:
        print(TMP_TIME)
    # if len(encodedData) > 6: 
    #     writer.writerow(encodedData[:6])
    #     # writer.writerow(encodedData[7:-1])
    # else: writer.writerow(encodedData[:-1])


async def notifyRead(address, file_name):    
    f = open(file_name, 'w+')
    tmpList = list()
    global writer 
    writer = csv.writer(f)
    async with BleakClient(address) as client:
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
                        await client.start_notify(characteristic, notify_callback)
        if client.is_connected:
            await asyncio.sleep(1) 
            print('try to deactivate notify.')
            await client.stop_notify(TX_UUID)
    f.close()
    print('disconnect')
    
async def write(address):    
    async with BleakClient(address) as client:
        print('connected')
        services = await client.get_services()        
        for service in services:
            await client.write_gatt_char(RX_UUID, bytes(b'ST'))
            print('Send Data')

    print('disconnect')
    
# loop = asyncio.get_event_loop()
# # loop.run_until_complete(findDevices())
# loop.run_until_complete(write(DEVICE_ADDRESSS))
# loop.run_until_complete(notifyRead(DEVICE_ADDRESSS))

