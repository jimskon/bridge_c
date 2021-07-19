#include <iostream>
#include <map>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>

#include<sys/socket.h>
#include <pthread.h>

#include "brmap.h"
#include "logger.h"
using namespace std;


brmap::brmap (logger & log):_log (log)
{;
}

void
brmap::print ()
{
  map < MACADDR, Bridge_entry >::iterator it = bridge.begin ();

  _log (0) << "Bridge table (" << bridge.size () << ")\n";
  for (; it != bridge.end (); it++)
    {
      _log (0) << it->first;
      _log (0) << " I: " << it->second.src_interface << " VID: " << it->
	second.vid << endl;
    }
}

  /* Transparent learning bridge map
     pkt_src    - source interface
     packet     - The packet to bridge
     returns:   0: broadcast
     1-n - dest interface
     -1 - not found, drop
   */
int
brmap::map_pkt (int pkt_src, unsigned char *packet)
{

  uint32_t dest_i = 0;
  MACADDR src_mac, dest_mac;

  // Get the addresses
  src_mac.get_src_mac (packet);
  dest_mac.get_dest_mac (packet);

  /*  Process the source address.  Add if not in bridge.
     Update bridge if interface is different
     drop if broadcast from this interface */
  if (bridge.find (src_mac) != bridge.end ())
    {
      // check if changed
      Bridge_entry b = bridge.find (src_mac)->second;

      int src_i = b.src_interface;
      if (src_i != pkt_src)
	{
	  // update source
	  b.src_interface = pkt_src;
	  bridge[src_mac] = b;
	}
    }
  else
    {
      Bridge_entry new_entry (pkt_src);
      // Entry vid will be zero until we hear different
      bridge.emplace (src_mac, new_entry);
    }

  if (dest_mac.is_multicast () || dest_mac.is_broadcast ())
    {
      dest_i = 0;
    }
  else
    {

      /* find where to send packet in the bridge */
      if (bridge.find (dest_mac) != bridge.end ())
	{
	  dest_i = bridge[dest_mac].src_interface;
	}
      else
	{
	  // If we don't find the destination, just drop
	  dest_i = -1;
	}
    }
  return dest_i;
}


void
brmap::add_vid (unsigned char *mac, uint32_t v)
{
  // Check if entry exists
  MACADDR aMac (mac);
  if (bridge.find (aMac) != bridge.end ())
    {
      Bridge_entry b = bridge.find (aMac)->second;
      b.vid = v;
      bridge[aMac] = b;
    }
  else
    {
      // New entry.  -1 means no interface known yet
      Bridge_entry new_entry (-1, v);
      bridge.emplace (aMac, new_entry);
    }

}

struct thread_arg
{
  brmap *brmap_ptr;
  int port;
};

void
error (const char *msg)
{
  perror (msg);
  exit (1);
}


void *
socket_listener (void *arg)
{
  int sockfd, readsockfd;
  unsigned int readlen;
  struct vid_mess aVid_mess;
  struct sockaddr_in serv_addr, read_addr;
  int port = ((struct thread_arg *) arg)->port;
  brmap *brmap_ptr = ((struct thread_arg *) arg)->brmap_ptr;
  sockfd = socket (VLAN_ASSIGN_CONN_TYPE, SOCK_STREAM, 0);
  if (sockfd < 0)
    error ("ERROR opening update socket");
  bzero ((char *) &serv_addr, sizeof (serv_addr));
  serv_addr.sin_family = VLAN_ASSIGN_CONN_TYPE;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons (port);
  if (bind (sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0)
    error ("ERROR on binding update socket");
  for (;;)
    {
      listen (sockfd, 5);
      readlen = sizeof (read_addr);
      readsockfd = accept (sockfd, (struct sockaddr *) &read_addr, &readlen);
      if (readsockfd < 0)
	error ("ERROR on accept bridge assignments");
      bzero ((void *) &aVid_mess, sizeof (struct vid_mess));
      int n = read (readsockfd, &aVid_mess, sizeof (struct vid_mess));
      MACADDR mac (aVid_mess.be_haddr);
      uint32_t vid = aVid_mess.be_vid;
      if (n < 0)
	error ("ERROR reading from bridge socket");
      cout << "Received vid: " << mac << " VID: " << vid << endl;;
      brmap_ptr->add_vid (mac.mac, vid);
      close (readsockfd);
    }
}

void
brmap::start_listener (int port)
{
  pthread_t fifo_thrd;
  struct thread_arg *arg = new struct thread_arg;
  arg->port = port;
  arg->brmap_ptr = this;
  pthread_create (&fifo_thrd, NULL, socket_listener, (void *) arg);

}
