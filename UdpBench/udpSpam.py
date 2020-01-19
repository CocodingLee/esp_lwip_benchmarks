import socket

UDP_IP = "192.168.4.1"
UDP_PORT = 4120
PACKET_SIZE = 1460

print "UDP target IP:", UDP_IP
print "UDP target port:", UDP_PORT

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP

#Ask for a large send-buffer
sock.setsockopt(socket.SOL_SOCKET,
                        socket.SO_SNDBUF,
                        1024*1024)

#The following sockopts disable UDP checksum generation,
#and UDP checksum checking on the receiver.
#This should only be done on networks where all hops
#are protected by a better integrity mechanism, such
#as WiFi's FCS. It should not be used on the general internet.
#sock.setsockopt(socket.SOL_SOCKET,socket.SO_NO_CHECK,1) #Linux
#sock.setsockopt(socket.IPPROTO_UDP, 1, 1) #Windows

while True:
	MESSAGE = "H"*PACKET_SIZE
	sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))
