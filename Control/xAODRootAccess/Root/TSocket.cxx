/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s).
#include "xAODRootAccess/tools/TSocket.h"

// ROOT include(s).
#include <TInetAddress.h>
#include <TString.h>

// System include(s).
extern "C" {
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
}
#include <cstring>
#include <memory>

namespace xAOD {

TSocket::TSocket() : m_socket(-1) {}

TSocket::~TSocket() {

  // Close the socket if it's open:
  if (isConnected()) {
    close().ignore();
  }
}

/// It turns out that translating a string address name into an integer
/// address that the connect(...) function understands, is no easy feat.
/// So instead we leave that up to ROOT. This translation just needs to
/// be done "early enough" in the job when ROOT can still do it.
///
/// @param address The address to connect to
/// @param port The port number to connect to
/// @returns The usual return codes...
///
StatusCode TSocket::connect(const TInetAddress& address, int port) {

  // If the address is invalid, give up now:
  if (!address.IsValid()) {
    return StatusCode::FAILURE;
  }

  // If not, translate it to a network address:
  const UInt_t adr = htonl(address.GetAddress());

  // "Translate" the port number:
  short sport = 0;
  struct servent sp_buf;
  struct servent* sp;
  char buf[1024];  // needs to be large enough for all strings in servent
  getservbyport_r(htons(port), "tcp", &sp_buf, buf, sizeof(buf), &sp);
  if (sp) {
    sport = sp->s_port;
  } else {
    sport = htons(port);
  }

  // Create a socket:
  m_socket = ::socket(AF_INET, SOCK_STREAM, 0);
  if (m_socket < 0) {
    return StatusCode::FAILURE;
  }

  // Create the address that we want to connect to:
  struct sockaddr_in server;
  memset(&server, 0, sizeof(server));
  memcpy(&server.sin_addr, &adr, sizeof(adr));
  server.sin_family = address.GetFamily();
  server.sin_port = sport;

  // Connect to the address:
  if (::connect(m_socket, reinterpret_cast<const sockaddr*>(&server),
                sizeof(server)) < 0) {
    m_socket = -1;
    return StatusCode::FAILURE;
  }

  // Return gracefully:
  return StatusCode::SUCCESS;
}

StatusCode TSocket::close() {

  // Check if anything needs to be done:
  if (!isConnected()) {
    return StatusCode::RECOVERABLE;
  }

  // Close the socket:
  if (::close(m_socket) != 0) {
    return StatusCode::FAILURE;
  }

  // Return gracefully:
  m_socket = -1;
  return StatusCode::SUCCESS;
}

bool TSocket::isConnected() const {

  return (m_socket != -1);
}

/// This function is custom made for our use case. It sends HTML style
/// information to the connected server.
///
/// @param payload The HTML style payload to send to the server
/// @returns The usual return codes...
///
StatusCode TSocket::send(const TString& payload) {

  // Check if we're connected:
  if (!isConnected()) {
    return StatusCode::FAILURE;
  }

  // The buffer we are sending from:
  const char* buffer = payload.Data();
  // The total length of the message:
  const auto length = payload.Length();
  // Number of bytes already sent:
  int sent = 0;

  // Keep sending the message until everything is through:
  for (int i = 0; i < length; i += sent) {
    sent = ::send(m_socket, buffer + i, length - i, 0);
    if (sent < 0) {
      return StatusCode::FAILURE;
    } else if (sent == 0) {
      break;
    }
  }

  // Return gracefully:
  return StatusCode::SUCCESS;
}

StatusCode TSocket::receive(std::size_t maxSize, TString& response) {

  // Check if we're connected.
  if (!isConnected()) {
    return StatusCode::FAILURE;
  }

  // Buffer to receive the message into.
  auto buffer = std::make_unique<char[]>(maxSize);

  // Try to receive a message.
  const auto n = ::recv(m_socket, buffer.get(), maxSize, 0);
  if (n <= 0) {
    return StatusCode::FAILURE;
  }

  // Return the message as a TString.
  response = TString(buffer.get(), static_cast<Ssiz_t>(n));
  return StatusCode::SUCCESS;
}

}  // namespace xAOD
