/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////////////////////
// Class for Z-window RoI from random position (excluding the HS).
/////////////////////////////////////////////////////////////////////////////////

#ifndef SiSpacePointsSeedTool_xk_RandomRoISeedTool_h
#define SiSpacePointsSeedTool_xk_RandomRoISeedTool_h

#include "BeamSpotConditionsData/BeamSpotData.h"
#include "InDetRecToolInterfaces/IZWindowRoISeedTool.h"
#include "GaudiKernel/EventContext.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/EventContext.h"
#include "AthenaKernel/IAthRNGSvc.h"


#include <vector>

namespace CLHEP {
  class RandGauss;
}

namespace InDet {

  class RandomRoISeedTool final: 
    public extends<AthAlgTool, IZWindowRoISeedTool>
  {

    ///////////////////////////////////////////////////////////////////
    // Public methods:
    ///////////////////////////////////////////////////////////////////
      
  public:
    
    ///////////////////////////////////////////////////////////////////
    // Standard tool methods
    ///////////////////////////////////////////////////////////////////

    RandomRoISeedTool(const std::string&,const std::string&,const IInterface*);
    virtual ~RandomRoISeedTool() = default;
    virtual StatusCode               initialize() override;

    /** Compute RoI */
    virtual std::vector<ZWindow> getRoIs(const EventContext& ctx) const override;

  protected:

    /**    @name Disallow default instantiation, copy, assignment **/
    RandomRoISeedTool() = delete;
    RandomRoISeedTool(const RandomRoISeedTool&) = delete;
    RandomRoISeedTool &operator=(const RandomRoISeedTool&) = delete;

    ///////////////////////////////////////////////////////////////////
    // Protected data and methods
    ///////////////////////////////////////////////////////////////////
  
    FloatProperty m_z0Window{this, "TrackZ0Window", 1.0, "width of z0 window"};
    SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey{this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot"};
    ServiceHandle<IAthRNGSvc> m_atRndmSvc{this, "RndmGenSvc", "AthRNGSvc", "multi-thread safe random number generator"};
    
    std::string m_rndmEngineName{"SINGLE"};// name of random engine
    
  }; // RandomRoISeedTool
} //InDet namespace

#endif // SiSpacePointsSeedMaker_RandomRoISeedTool

