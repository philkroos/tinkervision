import sys

# Libraries to communice with the brick
from tinkerforge.ip_connection import IPConnection
from tinkerforge.brick_red import BrickRED


class RedBrick:
    def __init__(self, uid, host, port):
        self.uid = uid
        self.host = host
        self.port = port

    def __enter__(self):
        class RedBrickResource:
            def connect(self, uid, host, port):
                self.ipcon = IPConnection()
                self.ipcon.connect(host, port)
                self.rb = BrickRED(uid, self.ipcon)

            def disconnect(self):
                self.ipcon.disconnect()

        self.rb = RedBrickResource()
        #self.rb.connect(self.uid, self.host, self.port)
        return self.rb

    def __exit__(self, ex_type, ex_value, traceback):
        try:
            self.rb.disconnect()
        except:
            pass
