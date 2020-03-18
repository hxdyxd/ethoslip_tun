
MTU = 1400
PHY_DEV = ens33
TAP_DEV = tap0

TAP_STATIC_GW_IP = 10.10.0.1
TAP_STATIC_NATMASK = 255.255.255.0
TAP_STATIC_IP = 10.10.0.23

TAP_NAT_GW_IP = 10.10.0.111
TAP_NAT_NATMASK = 255.255.255.0
TAP_NAT_CLIENT_RANGE = 10.10.0.0/24


CC = $(CROSS_COMPILE)gcc
#STRIP = $(CROSS_COMPILE)strip


OBJ = ethoslip_tun


C_SRCS = \
main.c\
serial.c\
slip.c\
uart2.c

LIB_NAME = \
-lpthread

CFLAGS = -Os -Wall -std=gnu99


OBJS = $(patsubst %.c,%.o,$(C_SRCS))

all:$(OBJS)
	$(CC)  $(CFLAGS)  -o $(OBJ) $(OBJS) $(LIB_NAME)


%.o:%.c
	$(CC)  $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(OBJS)

dhcp:
	ifconfig $(TAP_DEV) mtu $(MTU)
	udhcpc -i $(TAP_DEV)
	
kc:
	# kill udhcpc
	killall udhcpc

static:
	ifconfig $(TAP_DEV) mtu $(MTU)
	ifconfig $(TAP_DEV) $(TAP_STATIC_IP) netmask $(TAP_STATIC_NATMASK)
	route add default gw $(TAP_STATIC_GW_IP)
	########################################################
	# gateway: $(TAP_STATIC_GW_IP)
	# netmask: $(TAP_STATIC_NATMASK)
	# ip: $(TAP_STATIC_IP)
	########################################################

nat:
	# please edit [tap-udhcpd.conf] file
	# Start ip forward
	echo 1 > /proc/sys/net/ipv4/ip_forward
	ifconfig $(TAP_DEV) mtu $(MTU)
	ifconfig $(TAP_DEV) $(TAP_NAT_GW_IP) netmask $(TAP_NAT_NATMASK)
	ifconfig $(TAP_DEV) mtu $(MTU)
	iptables -t nat -F POSTROUTING
	iptables -P FORWARD ACCEPT
	iptables -A FORWARD -m state --state ESTABLISHED,RELATED -j ACCEPT
	iptables -t nat -A POSTROUTING -s $(TAP_NAT_CLIENT_RANGE) -o $(PHY_DEV) -j MASQUERADE
	iptables -t mangle -A FORWARD -p tcp -m tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu
	# Start udhcpd
	udhcpd ./tap-udhcpd.conf&
	cat ./tap-udhcpd.conf
	########################################################
	# gateway: $(TAP_NAT_GW_IP)
	# netmask: $(TAP_NAT_NATMASK)
	# clientrange: $(TAP_NAT_CLIENT_RANGE)
	########################################################

kd:
	iptables -t nat -F POSTROUTING
	# kill udhcpd
	killall udhcpd



