/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////////////////////
// Class for Z-window RoI from truth HS position.
/////////////////////////////////////////////////////////////////////////////////

#ifndef SiSpacePointsSeedTool_xk_TruthHSRoISeedTool_h
#define SiSpacePointsSeedTool_xk_TruthHSRoISeedTool_h

#include "InDetRecToolInterfaces/IZWindowRoISeedTool.h"
#include "GaudiKernel/EventContext.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODTruth/TruthEvent.h"
#include "xAODTruth/TruthEventContainer.h"

#include <vector>

namespace InDet {

  class TruthHSRoISeedTool final: 
    public extends<AthAlgTool, IZWindowRoISeedTool>
  {

    ///////////////////////////////////////////////////////////////////
    // Public methods:
    ///////////////////////////////////////////////////////////////////
      
  public:
      
    ///////////////////////////////////////////////////////////////////
    // Standard tool methods
    ///////////////////////////////////////////////////////////////////

    TruthHSRoISeedTool(const std::string&,const std::string&,const IInterface*);
    virtual ~TruthHSRoISeedTool() = default;
    virtual StatusCode               initialize() override;

    /** Compute RoI */
    virtual std::vector<ZWindow> getRoIs(const EventContext& ctx) const override;

  protected:

    /**    @name Disallow default instantiation, copy, assignment **/
    TruthHSRoISeedTool() = delete;
    TruthHSRoISeedTool(const TruthHSRoISeedTool&) = delete;
    TruthHSRoISeedTool &operator=(const TruthHSRoISeedTool&) = delete;

    ///////////////////////////////////////////////////////////////////
    // Protected data and methods
    ///////////////////////////////////////////////////////////////////
  
    SG::ReadHandleKey<xAOD::TruthEventContainer> m_inputTruthEventsKey{this, "InputTruthEventsCollection", "TruthEvents", "Input truth events collection."};
    FloatProperty m_z0Window{this, "TrackZ0Window", 1.0, "width of z0 window"};
    
  }; // TruthHSRoISeedTool
} //InDet namespace

#endif // SiSpacePointsSeedMaker_TruthHSRoISeedTool

