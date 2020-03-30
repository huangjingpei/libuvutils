# -*- coding: utf-8
from socket import *
import time
client = socket(AF_INET, SOCK_DGRAM)
 
 
client.sendto(b'hello',('127.0.0.1',8000))
 
client.sendto(b'world',('127.0.0.1',8000))
 
client.close()
