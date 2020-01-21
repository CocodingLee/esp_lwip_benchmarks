import socket

TCP_IP = "192.168.4.1"
TCP_PORT = 4120
PACKET_SIZE = 1460

print "TCP target IP:", TCP_IP
print "TCP target port:", TCP_PORT

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_STREAM) # TCP

#Ask for a large send-buffer
sock.setsockopt(socket.SOL_SOCKET,
                        socket.SO_SNDBUF,
                        1024*1024)

sock.connect((TCP_IP,TCP_PORT))
MESSAGE = "H"*PACKET_SIZE
while True:
	sock.send(MESSAGE)
	sock.recv(2*PACKET_SIZE)