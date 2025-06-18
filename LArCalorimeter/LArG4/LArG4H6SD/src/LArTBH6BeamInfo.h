/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LArTBH6BeamInfo_H
#define LArTBH6BeamInfo_H

#include "AthenaBaseComps/AthAlgorithm.h"

#include "Identifier/Identifier.h"

#include <string>

#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadHandle.h"
#include "TBEvent/TBTrack.h"
#include "TBEvent/TBEventInfo.h"
#include "HitManagement/AthenaHitsVector.h"
#include "LArG4TBSimEvent/LArG4H6FrontHitCollection.h"

class LArTBH6BeamInfo : public AthAlgorithm {

public:

// Constructor
   LArTBH6BeamInfo(const std::string& name, ISvcLocator* pSvcLocator);

// Destructor
   virtual ~LArTBH6BeamInfo() = default;

// Gaudi
   virtual StatusCode initialize() override;
   virtual StatusCode execute() override;
   virtual StatusCode finalize() override;

private:
   typedef std::vector<double> dVect;

  ////////////////////////////////////////////////////////////////////////
  /// \brief Fit data to the function u = a1 + a2*w. and determines
  ///        intercept, slope, residual for each BPC, and chi2 on fit
  /////////////////////////////////////////////////////////////////////////
  bool fitVect(const dVect &vec_x, const dVect &vec_xz, const dVect &vec_ex,
               double &a1, double &a2, double &chi2, dVect &residual);

  Gaudi::Property<std::vector<std::string>> m_HitsCollNames{this, "HitsContainer"};
  Gaudi::Property<bool> m_Primary{this, "PrimaryTrackOnly", true};
  Gaudi::Property<int>  m_pcode{this, "PrimaryParticle", 999};

  float                    m_cryoX{0.f};
  int                      m_numEv{0};

  SG::ReadHandle<TBEventInfo> m_theEventInfo;
  SG::WriteHandle<TBTrack> m_track;
  std::vector< SG::ReadHandle< AthenaHitsVector<LArG4H6FrontHit> > > m_hitcoll;
};
#endif
