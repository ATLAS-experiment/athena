/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETPHYSVALMONITORING_INDETPHYSVALTRUTHDECORATORTOOL_H
#define INDETPHYSVALMONITORING_INDETPHYSVALTRUTHDECORATORTOOL_H
/**
 * @file InDetPhysValTruthDecoratorAlg.h
 * header file for class of same name
 * @author shaun roe
 * @date 27 March 2014
 **/
// STL includes
#include <string>
#include "xAODTruth/TruthParticleContainer.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"
#include "AthContainers/AuxElement.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"
#include "GaudiKernel/EventContext.h"
#include "TrkTruthTrackInterfaces/IAthSelectionTool.h"
#include "InDetPhysValMonitoring/CutFlow.h"
#include "CxxUtils/checker_macros.h"
#include <atomic>
#include <utility>
#include <vector>


// class to decorate xAOD::TruthParticles with additional information required by validation
class InDetPhysValTruthDecoratorAlg: public AthReentrantAlgorithm {
public:
  InDetPhysValTruthDecoratorAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual
  ~InDetPhysValTruthDecoratorAlg ();
  virtual StatusCode initialize();
  virtual StatusCode finalize();
  virtual StatusCode execute(const EventContext &ctx) const;

private:
  enum {kPixel,kSCT,kNClusterTypes};
  bool decorateTruth(const xAOD::TruthParticle& particle,
                     std::vector< std::pair<SG::WriteDecorHandle<xAOD::TruthParticleContainer,float>,
                                            bool > > &float_decor,
                     const Amg::Vector3D& beamPos,
                     const std::vector<std::array<uint16_t, kNClusterTypes> > &counts) const;

  PublicToolHandle<Trk::IExtrapolator> m_extrapolator
     {this,"Extrapolator","Trk::Extrapolator/AtlasExtrapolator",""};
  SG::ReadDecorHandleKeyArray<xAOD::EventInfo> m_beamSpotDecoKey
     {this, "BeamSpotDecoKeys",
        {"EventInfo.beamPosX", "EventInfo.beamPosY", "EventInfo.beamPosZ"},
	"Beamspot position decoration keys"};

  PublicToolHandle<IAthSelectionTool>         m_truthSelectionTool
     {this,"TruthSelectionTool","",""};

  mutable std::mutex m_mutex;
  mutable CutFlow m_cutFlow ATLAS_THREAD_SAFE; // Guarded by m_mutex

  mutable std::atomic<std::size_t> m_nMissingTruthParticles = 0u;
  mutable std::atomic<bool> m_errorEmitted{false};

  ///TruthParticle container's name needed to create decorators
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticleName
    {this, "TruthParticleContainerName",  "TruthParticles", ""};

  Gaudi::Property<std::string> m_prefix
    {this, "Prefix", "", "Decoration prefix to avoid clashes."};
  
  ///TruthPixelClusterContainer and TruthSCTClusterContainer needed for truth silicon hit cut
  SG::ReadHandleKey<xAOD::TrackMeasurementValidationContainer> m_truthPixelClusterName
    {this, "PixelClusterContainerName",  "PixelClusters", ""};
  
  SG::ReadHandleKey<xAOD::TrackMeasurementValidationContainer> m_truthSCTClusterName
    {this, "SCTClusterContainerName",  "SCT_Clusters", ""};

  SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_truthParticleIndexDecor
    {this, "TruthParticleIndexDecoration", m_truthParticleName, "origTruthIndex", "decoration name for the original truth particle index."};
   
  // decoration helper
  enum EDecorations {
    kDecorD0,
    kDecorZ0,
    kDecorPhi,
    kDecorTheta,
    kDecorZ0st,
    kDecorQOverP,
    kDecorProdR,
    kDecorProdZ,
    kDecorNSilHits,
    kNDecorators
  };
  std::vector< std::pair<SG::WriteDecorHandleKey<xAOD::TruthParticleContainer>,SG::AuxElement::ConstAccessor<float> > > m_decor;
};
#endif
