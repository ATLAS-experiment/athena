/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IOVSVC_IOVENTRY_H
#define IOVSVC_IOVENTRY_H

/*****************************************************************************
 *
 *  IOVEntry.h
 *  IOVSvc
 *
 *  Author: Charles Leggett
 *
 *  Validity range object that manages start and stop times, holding link
 *  to object DataProxy
 *
 *****************************************************************************/

#include "AthenaKernel/IOVRange.h"
#include "SGTools/DataProxy.h"

#include <memory>
#include <set>

class IOVEntry {
public: 

  // Ordered by increasing start times
  class IOVEntryStartCritereon {
  public: 
    bool operator() ( const IOVEntry &p1, const IOVEntry &p2 ) const {
      return p1.range()->start() > p2.range()->start();
    }
    bool operator() ( const IOVEntry *p1, const IOVEntry *p2 ) const {
      return p1->range()->start() > p2->range()->start();
    }
  };

  // Order by decreasing stop times
  class IOVEntryStopCritereon {
  public: 
    bool operator() ( const IOVEntry &p1, const IOVEntry &p2 ) const {
      return p1.range()->stop() < p2.range()->stop();
    }
    bool operator() ( const IOVEntry *p1, const IOVEntry *p2 ) const {
      return p1->range()->stop() < p2->range()->stop();
    }
  };

  typedef std::multiset<IOVEntry*,IOVEntryStartCritereon>::iterator startITR;
  typedef std::multiset<IOVEntry*,IOVEntryStopCritereon>::iterator  stopITR;

  IOVEntry( SG::DataProxy *proxy, std::unique_ptr<IOVRange> range):
    m_proxy(proxy), m_range(std::move(range))
    {}

  const IOVRange* range() const { return m_range.get(); }
  void setRange( std::unique_ptr<IOVRange> range) { m_range = std::move(range); }

  SG::DataProxy* proxy() { return m_proxy; }
  const SG::DataProxy* proxy() const { return m_proxy; }

  bool removedStart() const { return m_removedStart; }
  bool removedStop()  const { return m_removedStop; }

  void setRemovedStart(bool b) { m_removedStart = b; }
  void setRemovedStop(bool b)  { m_removedStop = b;  }

  void setStartITR( startITR itr ) { m_startITR = itr; }
  void setStopITR(  stopITR  itr ) { m_stopITR  = itr; }

  startITR getStartITR() const { return m_startITR; }
  stopITR  getStopITR()  const { return m_stopITR;  }

private:
  SG::DataProxy* m_proxy{};
  std::unique_ptr<IOVRange> m_range;

  bool m_removedStart{false};
  bool m_removedStop{false};

  startITR m_startITR{};
  stopITR  m_stopITR{};

};

#endif
