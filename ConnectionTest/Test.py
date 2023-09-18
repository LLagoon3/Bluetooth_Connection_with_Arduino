import asyncio 
from bleak import BleakScanner, BleakClient

DEVICE_ADDRESSS = "880747D3-2C1B-25C6-3A7E-FBC1871EE8B1"
RX_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
TX_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

def detection_callback(device, advertisement_data):
    print(device.address, "RSSI:", device.rssi, advertisement_data)

def on_disconnect(client):
    print("Client with address {} got disconnected!".format(client.address))

def notify_callback(sender: int, data: bytearray):
    print('sender: ', sender, 'data: ', data)
    encodedData, tmp = list(), ""
    for b in data:
        encodedCh = chr(b)
        if encodedCh == '_':
            encodedData.append(tmp)
            tmp = ""
        elif encodedCh == '\x00':
            encodedData.append(tmp)
            
        else: tmp += encodedCh
    
    print(encodedData)



async def findDevices():
    scanner = BleakScanner()
    scanner.register_detection_callback(detection_callback)
    await scanner.start()
    await asyncio.sleep(5)
    await scanner.stop()
    devices = await scanner.get_discovered_devices()
    for d in devices:
        print(d)

async def getServices(address):
    client = BleakClient(address)
    try:
        client.set_disconnected_callback(on_disconnect)
        await client.connect()
        print('connected')
        services = await client.get_services()
        print("Services:", type(services))
        for service in services:
            print(service) 
            print('\tuuid:', service.uuid)
            print('\tcharacteristic list:')
            for characteristic in service.characteristics:
                print('\t\t', characteristic)
                print('\t\tuuid:', characteristic.uuid)
                print('\t\tdescription :', characteristic.description)
                print('\t\tproperties :', characteristic.properties)
    except Exception as e:
        print('error: ', e, end='')        
    finally:
        print('start disconnect')
        await client.disconnect()

async def notifyRead(address):    
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
            await asyncio.sleep(20) 
            print('try to deactivate notify.')
            await client.stop_notify(TX_UUID)

    print('disconnect')

async def write(address):    
    async with BleakClient(address) as client:
        print('connected')
        services = await client.get_services()        
        for service in services:
            await client.write_gatt_char(RX_UUID, bytes(b'hello world'))
            print('Send Data')
    
    print('disconnect')

loop = asyncio.get_event_loop()
loop.run_until_complete(notifyRead(DEVICE_ADDRESSS))
loop.run_until_complete(write(DEVICE_ADDRESSS))
