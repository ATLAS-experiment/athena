/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <iomanip>
#include "TrigDataAccessMonitoring/ROBDataMonitor.h"

using namespace robmonitor;

//
//--- ROBDataStruct 
//    -------------
ROBDataStruct::ROBDataStruct(const uint32_t srcId)
  : rob_id(srcId)
{}

bool ROBDataStruct::isUnclassified() const {
  return ((rob_history == robmonitor::UNCLASSIFIED) ? true : false);
}

bool ROBDataStruct::isHLTCached() const {
  return ((rob_history == robmonitor::HLT_CACHED) ? true : false);
}

bool ROBDataStruct::isDCMCached() const {
  return ((rob_history == robmonitor::DCM_CACHED) ? true : false);
}

bool ROBDataStruct::isRetrieved() const {
  return ((rob_history == robmonitor::RETRIEVED) ? true : false);
}

bool ROBDataStruct::isIgnored() const {
  return ((rob_history == robmonitor::IGNORED) ? true : false);
}

bool ROBDataStruct::isUndefined() const {
  return ((rob_history == robmonitor::UNDEFINED) ? true : false);
}

bool ROBDataStruct::isStatusOk() const {
  return (rob_status_word == 0) ? true : false;
}

// Extraction operator for ROBDataStruct
std::ostream& robmonitor::operator<<(std::ostream& os, const ROBDataStruct& rhs) {
  os << "[SourceID,Size(words),History,(Status words)]=["
     << std::hex <<  std::setfill( '0' ) << "0x" << std::setw(6) << rhs.rob_id
     << std::dec << std::setfill(' ')
     << "," << std::setw(8) << rhs.rob_size;
  os << "," << std::setw(12);
  if (rhs.rob_history == robmonitor::UNCLASSIFIED) {
    os << "UNCLASSIFIED";
  } else if (rhs.rob_history == robmonitor::RETRIEVED) {
    os << "RETRIEVED";
  } else if (rhs.rob_history == robmonitor::HLT_CACHED) {
    os << "HLT_CACHED";
  } else if (rhs.rob_history == robmonitor::DCM_CACHED) {
    os << "DCM_CACHED";
  }else if (rhs.rob_history == robmonitor::IGNORED) {
    os << "IGNORED";
  } else if (rhs.rob_history == robmonitor::UNDEFINED) {
    os << "UNDEFINED";
  } else {
    os << "invalid code";
  }
  os << ",(";
  os << std::hex <<  std::setfill( '0' ) << "0x" << std::setw(8) << rhs.rob_status_word;
  os << ")]";
  return os;
}

//
//--- ROBDataMonitorStruct
//    --------------------
ROBDataMonitorStruct::ROBDataMonitorStruct(const uint32_t l1_id, const std::string& req_nam="UNKNOWN")
  :lvl1ID(l1_id),
   requestor_name(req_nam)
{}

ROBDataMonitorStruct::ROBDataMonitorStruct(const uint32_t l1_id, 
					   const std::vector<uint32_t>& req_robs,
					   const std::string& req_nam="UNKNOWN")
  :lvl1ID(l1_id),
   requestor_name(req_nam)
{
  for (uint32_t rob : req_robs) {
    requested_ROBs[ rob ] = robmonitor::ROBDataStruct( rob ) ;
  }
}

unsigned ROBDataMonitorStruct::allROBs() const {
  return requested_ROBs.size(); 
}

unsigned ROBDataMonitorStruct::unclassifiedROBs() const {
  ptrdiff_t ret=0;
  for (const auto& p : requested_ROBs) {
    if (p.second.isUnclassified()) ++ret;
  }     
  return ret;
} 

unsigned ROBDataMonitorStruct::HLTcachedROBs() const {
  ptrdiff_t ret=0;
  for (const auto& p : requested_ROBs) {
    if (p.second.isHLTCached()) ++ret;
  }     
  return ret;
}

unsigned ROBDataMonitorStruct::DCMcachedROBs() const {
  ptrdiff_t ret=0;
  for (const auto& p : requested_ROBs) {
    if (p.second.isDCMCached()) ++ret;
  }     
  return ret;
}

unsigned ROBDataMonitorStruct::retrievedROBs() const {
  ptrdiff_t ret=0;
  for (const auto& p : requested_ROBs) {
    if (p.second.isRetrieved()) ++ret;
  }     
  return ret;
}

unsigned ROBDataMonitorStruct::ignoredROBs() const {
  ptrdiff_t ret=0;
  for (const auto& p : requested_ROBs) {
    if (p.second.isIgnored()) ++ret;
  }     
  return ret;
}

unsigned ROBDataMonitorStruct::undefinedROBs() const {
  ptrdiff_t ret=0;
  for (const auto& p : requested_ROBs) {
    if (p.second.isUndefined()) ++ret;
  }     
  return ret;
}

unsigned ROBDataMonitorStruct::statusOkROBs() const {
  ptrdiff_t ret=0;
  for (const auto& p : requested_ROBs) {
    if (p.second.isStatusOk()) ++ret;
  }     
  return ret;
}

float ROBDataMonitorStruct::elapsedTime() const {
  float secs = 0 ;
  if (end_time >= start_time)
    secs = (end_time - start_time)/1e3;
  return secs;
}

// Extraction operator for ROBDataMonitorStruct
std::ostream& robmonitor::operator<<(std::ostream& os, const ROBDataMonitorStruct& rhs) {
  const std::string prefix("   ");
  const std::string prefix2("-> ");
  os << "ROB Request for L1 ID = " << std::dec << rhs.lvl1ID << " (decimal), L1 ID = 0x"
     << std::hex << rhs.lvl1ID << " (hex)" << std::dec;
  os << "\n" << prefix << "Requestor name = " << rhs.requestor_name;

  const std::time_t s_time(rhs.start_time / static_cast<int>(1e6));
  struct tm buf;
  gmtime_r(&s_time, &buf);
  os << "\n" << prefix << "Start time of ROB request         = "
     << std::put_time(&buf, "%c")
     << " UTC + " << (rhs.start_time % static_cast<int>(1e6)) / 1000.0f << " [ms]";

  const std::time_t e_time(rhs.end_time / static_cast<int>(1e6));
  gmtime_r(&e_time, &buf);
  os << "\n" << prefix << "Stop  time of ROB request         = "
     << std::put_time(&buf, "%c")
     << " UTC + " << (rhs.end_time % static_cast<int>(1e6)) / 1000.0f << " [ms]";
  os << "\n" << prefix << "Elapsed time for ROB request [ms] = " << rhs.elapsedTime();
  os << "\n" << prefix << "Requested ROBs:";
  os << "\n" << prefix << prefix2 << "All          " << rhs.allROBs()          ;
  os << "\n" << prefix << prefix2 << "Unclassified " << rhs.unclassifiedROBs() ;
  os << "\n" << prefix << prefix2 << "HLT Cached   " << rhs.HLTcachedROBs()    ;
  os << "\n" << prefix << prefix2 << "DCM Cached   " << rhs.DCMcachedROBs()    ;
  os << "\n" << prefix << prefix2 << "Retrieved    " << rhs.retrievedROBs()    ;
  os << "\n" << prefix << prefix2 << "Ignored      " << rhs.ignoredROBs()      ;
  os << "\n" << prefix << prefix2 << "Undefined    " << rhs.undefinedROBs()    ;
  os << "\n" << prefix << prefix2 << "Status OK    " << rhs.statusOkROBs()     ;
  for (const auto& [id, rob] : rhs.requested_ROBs ) {
    os << "\n" << prefix << prefix2 << rob;
  }
  return os;
}
