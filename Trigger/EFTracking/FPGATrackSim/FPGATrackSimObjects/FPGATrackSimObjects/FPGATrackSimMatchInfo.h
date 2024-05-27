/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGFPGATrackSimOBJECTS_FPGATrackSimMATCHINFO_H
#define TRIGFPGATrackSimOBJECTS_FPGATrackSimMATCHINFO_H

#include <TObject.h>

class FPGATrackSimMatchInfo : public TObject {
public:
  FPGATrackSimMatchInfo() : m_barcode(0), m_evtindex(-1) { ; }
  FPGATrackSimMatchInfo(int v1, int v2) : m_barcode(v1), m_evtindex(v2) { ; }

  int barcode() const { return m_barcode; }
  int evtindex() const { return m_evtindex; }

  bool operator==(const FPGATrackSimMatchInfo& o) const { return (m_barcode == o.m_barcode) && (m_evtindex == o.m_evtindex); }
  bool operator<(const FPGATrackSimMatchInfo& o) const { if (m_evtindex != o.m_evtindex) return (m_evtindex < o.m_evtindex); else return m_barcode < o.m_barcode; }


private:
  int m_barcode;
  int m_evtindex;


  ClassDef(FPGATrackSimMatchInfo, 1)
};

std::ostream& operator<<(std::ostream&, const FPGATrackSimMatchInfo&);
#endif // TRIGFPGATrackSimOBJECTS_FPGATrackSimMATCHINFO_H
