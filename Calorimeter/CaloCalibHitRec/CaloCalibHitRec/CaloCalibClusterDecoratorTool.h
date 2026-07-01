/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOCALIBHITREC_CALOCALIBCLUSTERDECORATORTOOL_H
#define CALOCALIBHITREC_CALOCALIBCLUSTERDECORATORTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "CaloUtils/CaloClusterCollectionProcessor.h"
#include "CaloCalibHitRec/CaloCalibDefineTypes.h"
#include "CaloCalibHitRec/ICaloCalibClusterTruthAttributerTool.h"

#include "xAODTruth/TruthParticle.h"
#include "xAODCaloEvent/CaloClusterContainer.h"

#include <map>
#include <vector>

class CaloCalibrationHit;

class CaloCalibClusterDecoratorTool
  : public extends<AthAlgTool, CaloClusterCollectionProcessor> {

public:
  using base_class::base_class;

  using CaloClusterCollectionProcessor::execute;

  virtual StatusCode execute(const EventContext& ctx,
                             xAOD::CaloClusterContainer* theClusColl) const override final;

  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  virtual ~CaloCalibClusterDecoratorTool() {};

private:
  SG::ReadHandleKey<std::map<Identifier,std::vector<const CaloCalibrationHit*> > >
    m_mapIdentifierToCalibHitsReadHandleKey{
      this,
      "IdentifierToCalibHitsMapName",
      "IdentifierToCalibHitsMap",
      "ReadHandleKey for the map between Identifiers and sets of calibration hits"
    };

  SG::WriteDecorHandleKey<xAOD::CaloClusterContainer>
    m_caloClusterWriteDecorHandleKeyNLeadingTruthParticles{
      this,
      "CaloClusterWriteDecorHandleKey_NLeadingTruthParticles",
      "CaloCalTopoClusters.calclus_NLeadingTruthParticleBarcodeEnergyPairs"
    };

  ToolHandle<ICaloCalibClusterTruthAttributerTool>
    m_truthAttributerTool{
      this,
      "TruthAttributerTool",
      "",
      "ToolHandle to a tool to create the calibration hit truth information that we need for the decoration"
    };

  Gaudi::Property<unsigned int>
    m_numTruthParticles{
      this,
      "NumTruthParticles",
      20,
      "Set number of truth particles per CaloCluster/PFO for which we store calibration hit energy"
    };
};

#endif
