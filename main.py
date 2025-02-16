import machine
import ubinascii
import pyb
from mfrc522 import MFRC522

# NFC Initialization (from nfc.cpp lines 70-132)
def nfc_init():
    spi = machine.SPI(1, baudrate=2500000, polarity=0, phase=0)
    nfc = MFRC522(spi=spi, cs=machine.Pin('D2'), rst=machine.Pin('D3'))
    return nfc

# USB CDC Setup (from main.cpp lines 89-94, 226-229)
usb = pyb.USB_VCP()
nfc = nfc_init()

def on_card_detected(uid):
    usb.write(f"NFC Detected: {uid}\r\n".encode())

while True:
    # NFC Polling (from nfc.cpp lines 88-105)
    (stat, tag_type) = nfc.request(nfc.REQIDL)
    if stat == nfc.OK:
        (stat, raw_uid) = nfc.anticoll()
        if stat == nfc.OK:
            uid = ubinascii.hexlify(raw_uid).decode().upper()
            on_card_detected(uid)
    
    # USB Communication (from main.cpp lines 204-229)
    if usb.any():
        data = usb.read(64)
        # Echo back with NFC status
        response = f"Received: {data.decode()} | NFC: {uid if 'uid' in locals() else 'None'}\r\n"
        usb.write(response.encode())
    
    machine.delay(100) 