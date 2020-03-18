#ifndef __SLIP_H__
#define __SLIP_H__

/* SEND_PACKET: sends a packet of length "len", starting at
 * location "p".
 */
void send_packet(unsigned char *p, int len);

/* RECV_PACKET: receives a packet into the buffer located at "p".
 *      If more than len bytes are received, the packet will
 *      be truncated.
 *      Returns the number of bytes stored in the buffer.
 */
int recv_packet(unsigned char *p, int len);


#endif
