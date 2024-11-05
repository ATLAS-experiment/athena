/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RPC_TIMINGTOOL_H
#define RPC_TIMINGTOOL_H

#include "MuonRecToolInterfaces/IMuonHitTimingTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

/** @class RPC_TimingTool
    
    RPC_TimingTool calculates the time shift for a set of RPC hits WRT the current bunch

    @author MCP projects
*/

namespace Muon{

  class MuonClusterOnTrack;
  
  class RPC_TimingTool : public extends <AthAlgTool, Muon::IMuonHitTimingTool> {
  public:
    RPC_TimingTool(const std::string&, const std::string&, const IInterface*);

    virtual ~RPC_TimingTool()=default;

    virtual StatusCode initialize() override final;

    /** Calculate the time offset of a given set of hits wrt to the current bunch */
    virtual TimingResult calculateTimingResult( const std::vector<const MuonClusterOnTrack*>& hits ) const override final;


  private:
    /** calculate error on the RPC time */
    double getError(const Muon::MuonClusterOnTrack&) const ; 

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

  };
}

#endif
