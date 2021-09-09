#!/usr/bin/env python3

import argparse
import secrets
import serial
import threading
import time
import zmq

class Redirect:
  def __init__(self, ip, port, dev, baud):
    self.ip = ip
    self.port = port
    self.dev = dev
    self.baud = baud
    self.timeout = 0.5

    self.server = None
    self.uart = None

    #  Socket to talk to server
    self.connect_server_gds()

    # UART connection to talk with FSW
    self.connect_fsw()

    self.uart_thread = threading.Thread(target=self.start_downlink)
    #self.sock_thread = threading.Thread(target=self.start_tcp_client,  args=(self.ip, self.port,))

    self.uart_thread.start()
    #self.sock_thread.start()

  def connect_fsw(self):
    print("Connecting to FSW on dev {} baud {}".format(self.dev, self.baud))
    self.uart = serial.Serial(self.dev, self.baud, timeout=self.timeout)

  def connect_server_gds(self):
    print("Connecting to GDS on ip {} and port {}".format(self.ip, self.port))
    context = zmq.Context()
    self.server = context.socket(zmq.PUB)
    self.server.connect("tcp://{}:{}".format(self.ip, self.port))

  def recv_uart_data(self):
    #return b"\x00" + secrets.token_bytes(4) + b"\x00"
    data = b""
    data = self.uart.read(1)
    while self.uart.in_waiting:
        data += self.uart.read(self.uart.in_waiting)
    return data

  def send_sock_data(self, data):
    self.server.send(data)

  def start_downlink(self):
    #for request in range(1000):
    while True:
      data = self.recv_uart_data()
      #print("uart recv: {}".format(data))
      self.send_sock_data(data)
      #time.sleep(1)
      #message = self.server.recv()
      #print("Received reply %s [ %s ]" % (request, message))

  def start_tcp_client(self, ip, port):
    #  Do 10 requests, waiting each time for a response
    for request in range(1000):
      print("Sending request %s …" % request)
      self.server.send(b"Hello")

      #  Get the reply.
      message = self.server.recv()
      print("Received reply %s [ %s ]" % (request, message))

if __name__ == "__main__":
  ap = argparse.ArgumentParser("Very simple tool to bridge uart and TCP socket communications.")

  ap.add_argument("--ip", default="localhost", type=str, help="IP number to bind to.")
  ap.add_argument("--port", default=50000, type=int, help="Port number to bind to.")
  ap.add_argument("--dev", default="/dev/ttyUSB0", type=str, help="Serial device to connect to.")
  ap.add_argument("--baud", default=115200, type=int, help="Serial baudrate.")

  args = ap.parse_args()

  redirect = Redirect(args.ip, args.port, args.dev, args.baud)
