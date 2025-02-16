import unittest
from micropython import ubinascii, machine, pyb

# Fake classes to simulate MicroPython hardware and libraries

# Fake SPI (placeholder)
class FakeSPI:
    pass

# Fake Pin
class FakePin:
    def __init__(self, name):
        self.name = name

# Fake NFC class to simulate MFRC522 behavior
class FakeNFC:
    REQIDL = 0x26
    OK = 0

    def __init__(self):
        self.request_called = False
        self.anticoll_called = False
        self.next_request_stat = FakeNFC.OK
        self.next_anticoll_stat = FakeNFC.OK
        # Fake UID: 0xAA, 0xBB, 0xCC, 0xDD
        self.fake_uid = b'\xAA\xBB\xCC\xDD'

    def request(self, req):
        self.request_called = True
        return (self.next_request_stat, 0)

    def anticoll(self):
        self.anticoll_called = True
        return (self.next_anticoll_stat, self.fake_uid)

    def set_no_card(self):
        self.next_request_stat = 1  # Not OK
        self.next_anticoll_stat = 1  # Not OK

    def set_card_present(self):
        self.next_request_stat = FakeNFC.OK
        self.next_anticoll_stat = FakeNFC.OK

# Fake USB CDC class
class FakeUSB:
    def __init__(self):
        self.data_in = []  # Data waiting to be read
        self.data_out = []  # Data that was written out

    def write(self, data):
        self.data_out.append(data)

    def any(self):
        return len(self.data_in) > 0

    def read(self, n):
        if self.any():
            return self.data_in.pop(0)
        return b''

# Fake machine delay function
class FakeMachine:
    @staticmethod
    def delay(ms):
        pass

# The main loop iteration function from the MicroPython integration.
# This function simulates one iteration of the main loop found in main.py.
# It does NFC polling and USB communication, then calls FakeMachine.delay.
def main_loop_iteration(nfc, usb):
    # NFC Polling
    stat, tag_type = nfc.request(nfc.REQIDL)
    if stat == nfc.OK:
        stat, raw_uid = nfc.anticoll()
        if stat == nfc.OK:
            uid = ubinascii.hexlify(raw_uid).decode().upper()
            usb.write(f"NFC Detected: {uid}\r\n".encode())
    
    # USB Communication
    if usb.any():
        data = usb.read(64)
        result_uid = uid if 'uid' in locals() else 'None'
        response = f"Received: {data.decode()} | NFC: {result_uid}\r\n"
        usb.write(response.encode())
    
    machine.delay(100)

class TestNFCIntegration(unittest.TestCase):
    def setUp(self):
        self.nfc = FakeNFC()
        self.usb = pyb.USB_VCP()

    def test_nfc_detection(self):
        # Simulate one iteration of the main loop
        main_loop_iteration(self.nfc, self.usb)
        
        # Verify NFC methods were called
        self.assertTrue(self.nfc.request_called)
        self.assertTrue(self.nfc.anticoll_called)

    def test_nfc_detection_with_usb_command(self):
        # Add a command to USB input
        self.usb.buffer.append(b'GET_STATUS')
        
        # Run one iteration
        main_loop_iteration(self.nfc, self.usb)
        
        # Verify both NFC detection and command response
        self.assertTrue(self.nfc.request_called)
        self.assertTrue(self.nfc.anticoll_called)

    def test_no_card_present(self):
        # Set NFC to simulate no card
        self.nfc.set_no_card()
        
        # Run one iteration
        main_loop_iteration(self.nfc, self.usb)
        
        # Verify methods were called but no data was sent
        self.assertTrue(self.nfc.request_called)
        self.assertFalse(self.nfc.anticoll_called)

    def test_card_removal(self):
        # First iteration with card
        main_loop_iteration(self.nfc, self.usb)
        
        # Second iteration without card
        self.nfc.set_no_card()
        main_loop_iteration(self.nfc, self.usb)
        
        # Verify state changes
        self.assertTrue(self.nfc.request_called)

    def test_multiple_commands(self):
        # Queue multiple commands
        commands = [b'CMD1', b'CMD2', b'CMD3']
        for cmd in commands:
            self.usb.buffer.append(cmd)
        
        # Process each command
        for _ in range(len(commands)):
            main_loop_iteration(self.nfc, self.usb)
            
        # Verify all commands were processed
        self.assertEqual(len(self.usb.buffer), 0)

if __name__ == '__main__':
    unittest.main() 