#!/usr/bin/env python3

import argparse
import secrets
import serial
import threading
import time
import zmq

class RedirectUplink:
  def __init__(self, ip, port, dev, baud):
    self.ip = ip
    self.port = port
    self.dev = dev
    self.baud = baud

    self.uart_thread = threading.Thread(target=self.start_uart_client, args=(self.dev, self.baud,))
    self.sock_thread = threading.Thread(target=self.start_tcp_client,  args=(self.ip, self.port,))

    self.uart_thread.start()
    self.sock_thread.start()

  def start_uart_client(self, dev, baud):
    for i in range(1000):
      data = b"\x00" + secrets.token_bytes(4) + b"\x00"
      print("uart recv: {}".format(data))
      time.sleep(1)

  def start_tcp_client(self, ip, port):
    context = zmq.Context()

    #  Socket to talk to server
    print("Connecting to hello world server…")
    socket = context.socket(zmq.REQ)
    socket.connect("tcp://{}:{}".format(ip, port))

    #  Do 10 requests, waiting each time for a response
    for request in range(1000):
      print("Sending request %s …" % request)
      socket.send(b"Hello")

      #  Get the reply.
      message = socket.recv()
      print("Received reply %s [ %s ]" % (request, message))

if __name__ == "__main__":
  ap = argparse.ArgumentParser("Very simple tool to bridge uart and TCP socket communications.")

  ap.add_argument("--ip", default="localhost", type=str, help="IP number to bind to.")
  ap.add_argument("--port", default=50000, type=int, help="Port number to bind to.")
  ap.add_argument("--dev", default="/dev/ttyUSB0", type=str, help="Serial device to connect to.")
  ap.add_argument("--baud", default=115200, type=int, help="Serial baudrate.")

  args = ap.parse_args()

  redirectUplink = RedirectUplink(args.ip, args.port, args.dev, args.baud)
