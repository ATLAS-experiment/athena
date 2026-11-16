/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_TBBEAMQUALITYMC_H
#define TBREC_TBBEAMQUALITYMC_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include <vector>

class TBBeamQualityMC : public AthAlgorithm
{
 public:
  TBBeamQualityMC(const std::string & name, ISvcLocator * pSvcLocator);

  virtual ~TBBeamQualityMC() {};

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;
  virtual StatusCode finalize() override;

 private:
  /** Get Xcryo and Ytable from a text file */
  Gaudi::Property<bool> m_readFileforXcryo{this, "ReadFileforXcryo", true, "Get Xcryo and Ytable from a text file"};
  StatusCode getXcryoYtable(float &x, float &y, float &eBeam);
  int m_nRun{0};

  Gaudi::Property<float> m_bm_cut_x{this, "TrackCutX", 35., "cut on beam track x position"};
  Gaudi::Property<float> m_bm_cut_y{this, "TrackCutY", 35., "cut on beam track y position"};
  Gaudi::Property<bool> m_check_trackpar{this, "CheckTrackParams", false, "check track parameters"};

  Gaudi::Property<bool> m_check_primary{this, "CheckPrimaryTrack", true, "check primary energy deposition in scintillators"};
  Gaudi::Property<std::vector<int>> m_scint_prim{this, "ScintForPrimaryTrack", {}, "scintillators for primary track check"};

  Gaudi::Property<bool> m_check_veto{this, "CheckVetoScint", true, "check for hit in veto scintillator"};

  Gaudi::Property<bool> m_check_clus{this, "CheckClusters", false, "reject events without good cluster"};
  Gaudi::Property<bool> m_check_trackreco{this, "CheckTrackReco", false, "reject events with bad track reconstruction"};

  Gaudi::Property<std::string> m_clusterCollName{this, "ClusterCollectionName", "CaloTopoClusters", "cluster collection name"};
};

#endif
