// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

// System include(s):
#include <atomic>
#include <cstdlib>
#include <mutex>
#include <set>

// ROOT include(s):
#include <TError.h>
#include <TFile.h>
#include <TRandom.h>
#include <TSystem.h>
#include <TTimeStamp.h>
#include <TUrl.h>

// EDM include(s):
#include "xAODCore/tools/IOStats.h"
#include "xAODCore/tools/ReadStats.h"

// Local include(s):
#include "xAODRootAccess/tools/Message.h"
#include "xAODRootAccess/tools/TFileAccessTracer.h"
#include "xAODRootAccess/tools/TSocket.h"

namespace xAOD {

struct TFileAccessTracer::Impl {

  /// Helper struct storing information about the accessed files
  struct AccessedFile {
    /// The full path to the file
    std::string m_filePath;
    /// The name of the file
    std::string m_fileName;
    /// Operator to be able to put this into an std::set
    bool operator<(const AccessedFile& rhs) const {
      if (m_filePath != rhs.m_filePath) {
        return m_filePath < rhs.m_filePath;
      }
      if (m_fileName != rhs.m_fileName) {
        return m_fileName < rhs.m_fileName;
      }
      return false;
    }
    /// Function returning the full file path
    std::string fullFilePath() const {
      if (m_filePath == "") {
        return m_fileName;
      } else {
        return (m_filePath + "/" + m_fileName);
      }
    }
  };

  /// List of all the input files that were accessed in the job
  std::set<AccessedFile> m_accessedFiles;

  /// Address of the server to send monitoring information to
  std::string m_serverAddress{"http://rucio-lb-prod.cern.ch:18762/traces/"};
  /// Overall flag for enabling/disabling the data submission
  std::atomic_bool m_enableDataSumbission{true};

  /// Permanent reference to the job's ReadStats object.
  const ReadStats& m_readStats{IOStats::instance().stats()};

  /// Fraction of jobs that should send monitoring information
  double m_monitoredFraction{1.};

  /// Mutex for modifying the object
  mutable std::mutex m_mutex;

};  // struct TFileAccessTracer::Impl

/// The destructor of the class is the one doing most of the heavy lifting.
/// If the $SEND_XAOD_FILE_ACCESS_STAT environment variable is set when the
/// object gets deleted, it posts all the information it collected, to the
/// address defined by <code>SERVER_ADDRESS</code>. By constructing an HTTP
/// message from scratch.
///
TFileAccessTracer::~TFileAccessTracer() {

  // If the user turned off the data submission, then stop already here...
  if (m_impl->m_enableDataSumbission == false) {
    return;
  }

  // Decide what monitoring fraction to take. To make it possible for Panda
  // to override it with larger/smaller values if needed.
  double monitoredFraction = m_impl->m_monitoredFraction;
  const char* fractionString = gSystem->Getenv("XAOD_ACCESSTRACER_FRACTION");
  if (fractionString) {
    char* endptr = 0;
    const double fraction = strtod(fractionString, &endptr);
    if (endptr != fractionString) {
      monitoredFraction = fraction;
    }
  }

  // Decide randomly whether to send the monitoring data or not. While
  // TRandom is not good enough for statistical purposes, it's fast, and is
  // perfectly good to make this decision...
  ::TRandom rng(::TTimeStamp().GetNanoSec());
  if (rng.Rndm() > monitoredFraction) {
    return;
  }

  // Decide what server to send the information to. To make it possible for
  // Panda to override it if needed.
  std::string serverAddress = m_impl->m_serverAddress;
  const char* serverAddressString = gSystem->Getenv("XAOD_ACCESSTRACER_SERVER");
  if (serverAddressString) {
    serverAddress = serverAddressString;
  }

  // Construct the "technical address" of the host:
  const ::TUrl url(serverAddress.c_str());
  const ::TInetAddress serverInetAddress =
      gSystem->GetHostByName(url.GetHost());

  // Open a socket to the server:
  TSocket socket;
  if (socket.connect(serverInetAddress, url.GetPort()).isSuccess() == false) {
    // Just exit silently. If we can't send the info, we can't send the
    // info. It's not a problem.
    return;
  }

  // Let the user know what's happening:
  ::Info("xAOD::TFileAccessTracer", "Sending file access statistics to %s",
         serverAddress.c_str());

  // Start constructing the header of the message to send to the server:
  ::TString hdr = "POST /";
  hdr += url.GetFile();
  hdr += " HTTP/1.1\r\n";
  hdr += "From: ";
  hdr += gSystem->HostName();
  hdr += "\r\n";
  hdr += "Host: ";
  hdr += url.GetHost();
  hdr += "\r\n";
  hdr += "User-Agent: xAODRootAccess\r\n";
  hdr += "Content-Type: application/json\r\n";
  hdr += "Connection: close\r\n";
  hdr += "Content-Length: ";

  //
  // Now construct the message payload:
  //
  ::TString pld = "{";

  //
  // Collect the names of all the accessed files:
  //
  pld += "\"accessedFiles\": [";
  bool first = true;
  for (const Impl::AccessedFile& info : m_impl->m_accessedFiles) {
    if (!first) {
      pld += ", ";
    }
    pld += "\"";
    pld += info.fullFilePath();
    pld += "\"";
    first = false;
  }
  pld += "], ";
  //
  // Collect the names of all the containers that were accessed:
  //
  pld += "\"accessedContainers\": [";
  first = true;
  for (const auto& bs : m_impl->m_readStats.containers()) {
    if (!bs.second.readEntries()) {
      continue;
    }
    if (!first) {
      pld += ", ";
    }
    pld += "{\"";
    pld += bs.second.GetName();
    pld += "\": ";
    pld += bs.second.readEntries();
    pld += "}";
    first = false;
  }
  pld += "], ";
  //
  // Collect the names of all the branches that were accessed:
  //
  pld += "\"accessedBranches\": [";
  first = true;
  for (const auto& branch : m_impl->m_readStats.branches()) {
    for (const xAOD::BranchStats* bs : branch.second) {
      if ((!bs) || (!bs->readEntries())) {
        continue;
      }
      if (!first) {
        pld += ", ";
      }
      pld += "{\"";
      pld += bs->GetName();
      pld += "\": ";
      pld += bs->readEntries();
      pld += "}";
      first = false;
    }
  }
  pld += "]";
  //
  // Collect some possible Panda information in case the job is running
  // on the grid:
  //
  const char* pandaID = gSystem->Getenv("PandaID");
  if (pandaID) {
    pld += ", \"PandaID\": ";
    pld += pandaID;
    pld += "";
  }
  const char* taskID = gSystem->Getenv("PanDA_TaskID");
  if (taskID) {
    pld += ", \"PanDA_TaskID\": ";
    pld += taskID;
    pld += "";
  }
  //
  // Add some simple information about the host:
  //
  pld += ", \"ROOT_RELEASE\": \"";
  pld += ROOT_RELEASE;
  pld += "\"";
  pld += ", \"ReportRate\": ";
  pld += monitoredFraction;
  //
  // Add some information about the file access pattern:
  //
  pld += ", \"ReadCalls\": ";
  pld += m_impl->m_readStats.fileReads();
  pld += ", \"ReadSize\": ";
  pld +=
      (m_impl->m_readStats.fileReads() != 0
           ? m_impl->m_readStats.bytesRead() / m_impl->m_readStats.fileReads()
           : 0);
  pld += ", \"CacheSize\": ";
  pld += m_impl->m_readStats.cacheSize();
  pld += "}";

  // Now finish constructing the header, and merge the two into a single
  // message:
  hdr += TString::Format("%i", pld.Length());
  hdr += "\r\n\r\n";
  const ::TString msg = hdr + pld;

  // Send the message, and try to receive an answer.
  socket.send(msg).ignore();
  TString response;
  socket.receive(4096, response).ignore();
}

TFileAccessTracer& TFileAccessTracer::instance() {

  static TFileAccessTracer instance;
  return instance;
}

/// This function is called by TEvent to record which files were read from
/// during the job.
///
/// @param fileName The name of the file that is being read from
///
void TFileAccessTracer::add(std::string_view fileName) {

  // Protect this call:
  std::lock_guard<std::mutex> lock(m_impl->m_mutex);

  // Remember this file:
  m_impl->m_accessedFiles.insert(
      {gSystem->DirName(fileName.data()), gSystem->BaseName(fileName.data())});
}

const std::string& TFileAccessTracer::serverAddress() const {

  // Protect this call:
  std::lock_guard<std::mutex> lock(m_impl->m_mutex);

  return m_impl->m_serverAddress;
}

void TFileAccessTracer::setServerAddress(const std::string& addr) {

  // Protect this call:
  std::lock_guard<std::mutex> lock(m_impl->m_mutex);

  // Set the address itself:
  m_impl->m_serverAddress = addr;
}

double TFileAccessTracer::monitoredFraction() const {

  // Protect this call:
  std::lock_guard<std::mutex> lock(m_impl->m_mutex);

  return m_impl->m_monitoredFraction;
}

void TFileAccessTracer::setMonitoredFraction(double value) {

  // Protect this call:
  std::lock_guard<std::mutex> lock(m_impl->m_mutex);

  m_impl->m_monitoredFraction = value;
}

/// This function can be used by concerned users to turn off data collection
/// and submission for their jobs completely.
///
/// @param value Setting whether data submission should be enabled or not
///
void TFileAccessTracer::enableDataSubmission(::Bool_t value) {

  m_impl->m_enableDataSumbission = value;
}

TFileAccessTracer::TFileAccessTracer() : m_impl(std::make_unique<Impl>()) {}

}  // namespace xAOD
