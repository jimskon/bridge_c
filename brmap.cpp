#include <iostream>
#include <map>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <ctime>

#include<sys/socket.h>
#include <pthread.h>
#include <signal.h>

#include "brmap.h"
#include "logger.h"
using namespace std;


brmap::brmap (logger & log): br_ttl(DEF_BRIDGE_TIMEOUT),_log (log)
{
  br_last_scan=time(0);
}

void
brmap::print ()
{
  map < MACADDR, Bridge_entry >::iterator it = bridge.begin ();

  _log (0) << "Bridge table (" << bridge.size () << ")\n";
  for (; it != bridge.end (); it++)
    {
      _log (0) << it->first;
      _log (0) << " I: " << it->second.be_src_if << " VID: " << it->
	second.be_vid << endl;
    }
}

/* scan map for entries older then br_lifetime. */
void brmap::clean_map() {
  map < MACADDR, Bridge_entry >::iterator it = bridge.begin ();
  long int curtime = time(0);
  for (; it != bridge.end (); it++)
    {
      if (it->second.be_last_used < curtime-br_ttl)
	{
	  it->second.be_src_if = -1;
	}
    }
  br_last_scan = time(0);
  print();

}
  /* Transparent learning bridge map
     pkt_src    - source interface
     packet     - The packet to bridge
     returns:   0: broadcast
     1-n - dest interface
     -1 - not found, drop
   */
int
brmap::map_pkt (int pkt_src, unsigned char *packet, int * vid)
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
      Bridge_entry b = bridge.find (src_mac)->second;
      b.be_last_used=time(0);

      int src_i = b.be_src_if;
      // return vlan assignment for this destination
      *vid=b.be_vid;

      // check if changed
      if (src_i != pkt_src)
	{
	  // update source
	  b.be_src_if = pkt_src;
	  bridge[src_mac] = b;
	}
    }
  else
    {
      Bridge_entry new_entry (pkt_src,-1);
      // Entry vid will be -1 (unassigned)  until we hear different
      bridge.emplace (src_mac, new_entry);
      *vid=-1;
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
	  dest_i = bridge[dest_mac].be_src_if;
	  //bridge[dest_mac].be_last_used = time(0);
	}
      else
	{
	  // If we don't find the destination, just drop
	  dest_i = -1;
	}
    }
  /* clear out old entries */
  if (br_last_scan+br_ttl <= time(0)) clean_map();
  
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
      b.be_vid = v;
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
  int sockfd;
  int readsocketfd;
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
  unsigned int readlen;
  struct vid_mess aVid_mess;
  struct sockaddr_in serv_addr, read_addr;
  int port = ((struct thread_arg *) arg)->port;
  brmap *brmap_ptr = ((struct thread_arg *) arg)->brmap_ptr;
  brmap_ptr->sockfd = socket (VLAN_ASSIGN_CONN_TYPE, SOCK_STREAM, 0);
  cout << " socket open " << port << endl;
  
  if (brmap_ptr->sockfd < 0)
    error ("ERROR opening update socket");
  bzero ((char *) &serv_addr, sizeof (serv_addr));
  serv_addr.sin_family = VLAN_ASSIGN_CONN_TYPE;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons (port);
  if (bind (brmap_ptr->sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0)
    error ("ERROR on binding update socket");
  for (;;)
    {
      listen (brmap_ptr->sockfd, 5);
      readlen = sizeof (read_addr);
      brmap_ptr->readsockfd = accept (brmap_ptr->sockfd, (struct sockaddr *) &read_addr, &readlen);
      if (brmap_ptr->readsockfd < 0)
	error ("ERROR on accept bridge assignments");
      bzero ((void *) &aVid_mess, sizeof (struct vid_mess));
      int n = read (brmap_ptr->readsockfd, &aVid_mess, sizeof (struct vid_mess));
      MACADDR mac (aVid_mess.be_haddr);
      int vid = aVid_mess.be_vid;
      if (n < 0)
	error ("ERROR reading from bridge socket");
      cout << "Received vid: " << mac << " VID: " << vid << endl;;
      brmap_ptr->add_vid (mac.mac, vid);
      close (brmap_ptr->readsockfd);
      brmap_ptr->print();
    }
}

// Define the function to be called when ctrl-c (SIGINT) is sent to process
void signal_callback_handler(int signum) {
   cout << "Caught signal " << signum << endl;
   // Terminate program
   //close(thismap->sockfd);
   exit(signum);   
}

void
brmap::start_listener (int port)
{
  pthread_t fifo_thrd;
  struct thread_arg *arg = new struct thread_arg;
  arg->port = port;
  arg->brmap_ptr = this;
  pthread_create (&fifo_thrd, NULL, socket_listener, (void *) arg);
  signal(SIGINT, signal_callback_handler);  
  
}
