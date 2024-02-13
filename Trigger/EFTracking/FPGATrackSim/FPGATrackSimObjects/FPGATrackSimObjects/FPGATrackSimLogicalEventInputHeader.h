// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#ifndef TRIGFPGATrackSimOBJECTS_FPGATrackSimLOGICALEVENTINPUTHEADER_H
#define TRIGFPGATrackSimOBJECTS_FPGATrackSimLOGICALEVENTINPUTHEADER_H

#include <TObject.h>

#include <vector>
#include <iostream>
#include <sstream>

#include "FPGATrackSimObjects/FPGATrackSimTowerInputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimEventInfo.h"
#include "FPGATrackSimObjects/FPGATrackSimOptionalEventInfo.h"


class FPGATrackSimEventInfo;
class FPGATrackSimOptionalEventInfo;

class FPGATrackSimLogicalEventInputHeader : public TObject
{
public:

    FPGATrackSimLogicalEventInputHeader() = default;
    virtual ~FPGATrackSimLogicalEventInputHeader() = default;

    void reset(); //reset per event variables

    void newEvent(FPGATrackSimEventInfo& event) { reset(); m_event = event; }
    FPGATrackSimEventInfo const& event() const { return m_event; }

    FPGATrackSimOptionalEventInfo const& optional() const { return m_optional; }
    void setOptional(const FPGATrackSimOptionalEventInfo& o) { m_optional = o; }

    //  handling towers
    const std::vector<FPGATrackSimTowerInputHeader>& towers() const { return m_towers; }
    int                  nTowers() const { return m_towers.size(); }
    void                 addTower(FPGATrackSimTowerInputHeader s) { m_towers.push_back(s); }
    FPGATrackSimTowerInputHeader* getTower(size_t index) { return &m_towers[index]; } //get the pointer
    void                 reserveTowers(size_t size) { m_towers.reserve(size); }
    void                 addTowers(const std::vector<FPGATrackSimTowerInputHeader>& towers) { m_towers = towers; }

private:
    FPGATrackSimEventInfo                      m_event;
    FPGATrackSimOptionalEventInfo              m_optional; // This is only available for 1st stage
    std::vector<FPGATrackSimTowerInputHeader>  m_towers;

    ClassDef(FPGATrackSimLogicalEventInputHeader, 1)
};


std::ostream& operator<<(std::ostream&, const FPGATrackSimLogicalEventInputHeader&);



#endif // FPGATrackSimEVENTINPUTHEADER_H
