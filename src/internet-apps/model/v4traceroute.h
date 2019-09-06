/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 Ritsumeikan University, Shiga, Japan
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Alberto Gallegos Ramonet <ramonet@fc.ritsumei.ac.jp>
 */


#ifndef V4TRACEROUTE_H
#define V4TRACEROUTE_H

#include "ns3/application.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/average.h"
#include "ns3/simulator.h"
#include "ns3/output-stream-wrapper.h"
#include <map>

namespace ns3 {

class Socket;


class V4TraceRoute: public Application {
public:
  /**
  * \brief Get the type ID.
  * \return the object TypeId
  */
  static TypeId GetTypeId (void);

  V4TraceRoute();
  virtual ~V4TraceRoute();

  void Print (Ptr<OutputStreamWrapper> stream);

private:

  // inherited from Application base class.
  virtual void StartApplication (void);
  virtual void StopApplication (void);
  virtual void DoDispose (void);
  /**
   * \brief Return the application ID in the node.
   * \returns the application id
   */
  uint32_t GetApplicationId (void) const;
  /**
   * \brief Receive an ICMP Echo
   * \param socket the receiving socket
   *
   * This function is called by lower layers through a callback.
   */
  void Receive (Ptr<Socket> socket);

  /**
   * \brief Writes data to buffer in little-endian format.
   *
   * Least significant byte of data is at lowest buffer address
   *
   * \param buffer the buffer to write to
   * \param data the data to write
   */
  void Write32 (uint8_t *buffer, const uint32_t data);


  /**
   * \brief Writes data from a little-endian formatted buffer to data.
   *
   * \param buffer the buffer to read from
   * \param data the read data
   */
  void Read32 (const uint8_t *buffer, uint32_t &data);

  /*brief Send one (ICMP ECHO) to the destination*/
  void Send ();

  void StartWaitReplyTimer ();

  void HandleWaitReplyTimeout ();

  /// Remote address
  Ipv4Address m_remote;
  /// Wait  interval  seconds between sending each packet
  Time m_interval;
  /**
   * Specifies  the number of data bytes to be sent.
   * The default is 56, which translates into 64 ICMP data bytes when combined with the 8 bytes of ICMP header data.
   */
  uint32_t m_size;
  /// The socket we send packets from
  Ptr<Socket> m_socket;
  /// ICMP ECHO sequence number
  uint16_t m_seq;
  /// produce traceroute style output if true
  bool m_verbose;
  /// received packets counter
  uint32_t m_recv;
  /// Start time to report total ping time
  Time m_started;
  /// Next packet will be sent
  EventId m_next;
  // The Current probe value
  uint32_t m_probeCount;
  // The maximum number of probe packets per hop
  u_int16_t m_maxProbes;
  // The current TTL value
  uint16_t m_ttl;
  // The maximium Ttl (Max number of hops to trace)
  uint32_t m_maxTtl;
  // The wait time until the response is considered lost.
  Time  m_waitIcmpReplyTimeout;

  EventId m_waitIcmpReplyTimer;
  /// All sent but not answered packets. Map icmp seqno -> when sent
  std::map<uint16_t, Time> m_sent;
  // Stream of characters used for printing the traceroute results
  std::ostringstream os;
  // Stream of characters used for printing a single route
  std::ostringstream osRoute;
  // The Ipv4 address of the latest hop found
  std::ostringstream routeIpv4;
  // Stream of the traceroute used for the output file
  Ptr<OutputStreamWrapper> printStream;


};

} //ns3 namespace

#endif /*V4TRACEROUTE_H*/
