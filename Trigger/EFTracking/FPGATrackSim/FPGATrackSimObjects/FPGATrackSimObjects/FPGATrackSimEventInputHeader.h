/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGFPGATrackSimOBJECTS_FPGATrackSimEVENTINPUTHEADER_H
#define TRIGFPGATrackSimOBJECTS_FPGATrackSimEVENTINPUTHEADER_H

#include <TObject.h>
#include <vector>
#include <iostream>
#include <sstream>

#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimEventInfo.h"
#include "FPGATrackSimObjects/FPGATrackSimOptionalEventInfo.h"


class FPGATrackSimEventInfo;
class FPGATrackSimOptionalEventInfo;

class FPGATrackSimEventInputHeader
{
public:

  FPGATrackSimEventInputHeader() = default;
  virtual ~FPGATrackSimEventInputHeader();

  void newEvent(FPGATrackSimEventInfo const& event) { reset(); m_event = event; }
  void setOptional(FPGATrackSimOptionalEventInfo const& optional) { m_optional = optional; }
  void reset();//reset per event variables


  FPGATrackSimEventInfo const& event()            const { return m_event; }
  FPGATrackSimOptionalEventInfo const& optional() const { return m_optional; }

  //  handling hits
  const std::vector<FPGATrackSimHit>& hits() const { return m_Hits; }
  int  nHits()                      const { return m_Hits.size(); }
  void addHit(FPGATrackSimHit const& s) { m_Hits.push_back(s); }
  void clearHits() { m_Hits.clear(); }
  void reserveHits(size_t size) { m_Hits.reserve(size); }


private:
  FPGATrackSimEventInfo                 m_event;
  FPGATrackSimOptionalEventInfo         m_optional;
  std::vector<FPGATrackSimHit>          m_Hits;


  ClassDefNV(FPGATrackSimEventInputHeader, 4);
};

std::ostream& operator<<(std::ostream&, const FPGATrackSimEventInputHeader&);
#endif // TRIGFPGATrackSimOBJECTS_FPGATrackSimEVENTINPUTHEADER_H
