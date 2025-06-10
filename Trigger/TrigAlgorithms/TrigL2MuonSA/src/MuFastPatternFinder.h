/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef  TRIGL2MUONSA_MUFASTPATTERNFINDER_H
#define  TRIGL2MUONSA_MUFASTPATTERNFINDER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

#include "GeoPrimitives/GeoPrimitives.h"
#include "MdtCalibInterfaces/IMdtCalibrationTool.h"
#include "MuonRoad.h"
#include "MdtData.h"
#include "TrackData.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "NswPatternFinder.h"

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

namespace TrigL2MuonSA {

struct MdtLayerHits
{
  unsigned int ntot{0};
  unsigned int ntot_all{0};
  unsigned int ndigi{0};
  unsigned int ndigi_all{0};
  double ResSum{0};
  std::vector<unsigned int> indexes{};
};

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

class MuFastPatternFinder: public AthAlgTool
{
   public:

      using AthAlgTool::AthAlgTool;
    
      virtual StatusCode initialize() override;
    
   private:

      double calc_residual(double aw,double bw,double x,double y) const;
      void  doMdtCalibration(TrigL2MuonSA::MdtHitData& mdtHit, double track_phi, double phi0, bool isEndcap) const;

   public:

      StatusCode findPatterns(const TrigL2MuonSA::MuonRoad& muonRoad,
			      TrigL2MuonSA::MdtHits&        mdtHits,
			      std::vector<TrigL2MuonSA::TrackPattern>& v_trackPatterns) const;

      StatusCode findPatterns(const TrigL2MuonSA::MuonRoad& muonRoad,
			      TrigL2MuonSA::MdtHits&        mdtHits,
			      TrigL2MuonSA::StgcHits&       stgcHits,
			      TrigL2MuonSA::MmHits&         mmHits,
			      std::vector<TrigL2MuonSA::TrackPattern>& v_trackPatterns) const;
   private:
      ToolHandle<NswPatternFinder>  m_nswPatternFinder {this, "NswPatternFinder", "TrigL2MuonSA::NswPatternFinder"};

      // MDT calibration service
      ToolHandle<IMdtCalibrationTool> m_mdtCalibrationTool{this, "CalibrationTool", "MdtCalibrationTool"};

      ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
};

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

} // namespace TrigL2MuonSA

#endif  // MUFASTPATTERNFINDER_H
