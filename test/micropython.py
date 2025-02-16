"""Mock MicroPython environment for testing."""
import binascii

class ubinascii:
    @staticmethod
    def hexlify(data):
        return binascii.hexlify(data)

class machine:
    class Pin:
        def __init__(self, name):
            self.name = name

    class SPI:
        def __init__(self, id, **kwargs):
            self.id = id
            self.kwargs = kwargs

    @staticmethod
    def delay(ms):
        pass

class pyb:
    class USB_VCP:
        def __init__(self):
            self.buffer = []

        def any(self):
            return bool(self.buffer)

        def read(self, nbytes):
            if self.buffer:
                return self.buffer.pop(0)
            return b''

        def write(self, data):
            return len(data) 