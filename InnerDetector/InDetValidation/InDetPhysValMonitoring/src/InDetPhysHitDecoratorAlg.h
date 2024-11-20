/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETPHYSVALMONITORING_InDetPhysHitDecoratorAlg_H
#define INDETPHYSVALMONITORING_InDetPhysHitDecoratorAlg_H
/**
 * @file InDetPhysHitDecoratorAlg.h
 * header file for class of same name
 * @author shaun roe
 * @date 27 March 2014
 **/
// STL includes
#include <mutex>
#include <string>
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "StoreGate/WriteDecorHandle.h"
#include "GaudiKernel/EventContext.h"
#include "InDetCondTools/ISiLorentzAngleTool.h"
#include "TrkToolInterfaces/ITrackHoleSearchTool.h"
#include "TrkToolInterfaces/IUpdator.h"
#include "TrkToolInterfaces/IResidualPullCalculator.h"

class Identifier;
class AtlasDetectorID;
class PixelID;
class SCT_ID;
class TRT_ID;

// class to decorate xAOD::TruthParticles with additional information required by validation
class InDetPhysHitDecoratorAlg: public AthReentrantAlgorithm {
public:
  // L0PIXBARR : layer0 of pixel barrel (IBL in Run2)
  // PIXEL : in barrel all layers appart from layer0. In end-caps all disks/rings.
  enum Subdetector {
    INVALID_DETECTOR=-1, L0PIXBARR, PIXEL, SCT, TRT, N_SUBDETECTORS
  };
  enum Region {
    INVALID_REGION=-1, BARREL, ENDCAP
  };
  InDetPhysHitDecoratorAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual
  ~InDetPhysHitDecoratorAlg ();
  virtual StatusCode initialize() override;
  virtual StatusCode finalize()   override;
  virtual StatusCode execute(const EventContext &ctx) const override;

private:
  bool
  decorateTrack(const xAOD::TrackParticle &particle,
                std::vector< SG::WriteDecorHandle<xAOD::TrackParticleContainer,std::vector<float> > > &float_decor,
                std::vector< SG::WriteDecorHandle<xAOD::TrackParticleContainer,std::vector<int> > > &int_decor,
                std::vector< SG::WriteDecorHandle<xAOD::TrackParticleContainer,std::vector<uint64_t> > > &uint64_decor) const;

  ToolHandle<Trk::ITrackHoleSearchTool> m_holeSearchTool
    {this, "InDetTrackHoleSearchTool", "InDet::InDetTrackHoleSearchTool"};
  ToolHandle<Trk::IUpdator> m_updatorHandle
    {this, "Updator", "Trk::KalmanUpdator/TrkKalmanUpdator",
     "Tool handle of updator for unbiased states"};
  ToolHandle<Trk::IResidualPullCalculator> m_residualPullCalculator
    {this, "ResidualPullCalculator",
     "Trk::ResidualPullCalculator/ResidualPullCalculator"};
  ToolHandle<ISiLorentzAngleTool> m_lorentzAngleTool{this, "LorentzAngleTool", "SiLorentzAngleTool", "Tool to retrieve Lorentz angle"};

  enum EIntDecorations {
    kDecorRegion,
    kDecorDet,
    kDecorILayer,
    kDecorType,
    kDecorPhiWidth,
    kDecorEtaWidth,
    kNIntDecorators
  };
  enum EUInt64Decorations {
    kDecorID,
    kNUInt64Decorators
  };
  enum EDecorations {
    kDecorResidualLocX,
    kDecorPullLocX,
    kDecorMeasLocX,
    kDecorTrkParamLocX,
    kDecorMeasLocCovX,
    kDecorResidualLocY,
    kDecorPullLocY,
    kDecorMeasLocY,
    kDecorTrkParamLocY,
    kDecorMeasLocCovY,
    kDecorAngle,
    kDecorEtaLoc,
    kNFloatDecorators
  };
  // need tracks particle key, to compose decoration keys
  SG::ReadHandleKey<xAOD::TrackParticleContainer>  m_trkParticleName
    {this,"TrackParticleContainerName", "InDetTrackParticles",""};

  std::vector<SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> >   m_intDecor;
  std::vector<SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> >   m_uint64Decor;
  std::vector<SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> >   m_floatDecor;

  Gaudi::Property<std::string> m_prefix
    {this, "Prefix", "", "Decoration prefix to avoid clashes."};

  mutable std::atomic<bool> m_alreadyWarned{false};
  // the following help identify a surface in the detector
  const AtlasDetectorID* m_idHelper{nullptr};
  const PixelID* m_pixelID{nullptr};
  const SCT_ID* m_sctID{nullptr};
  const TRT_ID* m_trtID{nullptr};

  // private member functions
  bool decideDetectorRegion(const Identifier& id, Subdetector& det, Region& r, int& layer) const;
  const Trk::TrackParameters*
  getUnbiasedTrackParameters(const Trk::TrackParameters* trkParameters,
                             const Trk::MeasurementBase* measurement,
                             bool &isUnbiased) const;
};

#endif
