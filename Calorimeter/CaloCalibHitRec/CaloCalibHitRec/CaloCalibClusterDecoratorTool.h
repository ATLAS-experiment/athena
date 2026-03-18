/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOCALIBHITREC_CALOCALIBCLUSTERDECORATORTOOL_H
#define CALOCALIBHITREC_CALOCALIBCLUSTERDECORATORTOOL_H

#include "GaudiKernel/ToolHandle.h"
#include "CaloUtils/CaloClusterCollectionProcessor.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "CaloCalibHitRec/CaloCalibDefineTypes.h"
#include "CaloCalibHitRec/ICaloCalibClusterTruthAttributerTool.h"

//EDM Classes
#include "CaloSimEvent/CaloCalibrationHit.h"
#include "xAODTruth/TruthParticle.h"

//EDM Container Classes
#include "xAODCaloEvent/CaloClusterContainer.h"

//C++ classes
#include <string>
#include <vector>
#include <set>
#include <map>
#include <atomic>
#include <array>

/**
 * @class CaloCalibClusterDecoratorTool
 * @brief Calibration hit truth information decoration of xAOD::CaloClusters
 *
 * This algorithm decorates xAOD::CaloCluster with calibration hit truth information. 
 * It relies on upstream creation of several maps in CaloCalibClusterTruthMapMakerAlgorithm,
 * stored in Storegate, to provide fast access to required information.
 * The actual calculations are taken care of by an ICaloCalibClusterTruthAttributerTool.
 * The user may toggle how many truth particles to consider per xAOD::CaloCluster,
 * ordered in leading calibration hit truth pt, via a Gaudi Property "NumTruthParticles".
 */
class CaloCalibClusterDecoratorTool : public AthAlgTool, virtual public CaloClusterCollectionProcessor {

public:

  /**
   * @brief Constructor
   */
  CaloCalibClusterDecoratorTool(const std::string& type, const std::string& name,
                                const IInterface* parent);

  using CaloClusterCollectionProcessor::execute;


  virtual StatusCode execute(const EventContext& ctx,
                             xAOD::CaloClusterContainer* theClusColl) const override final;

  
  virtual StatusCode initialize() override;


  virtual StatusCode finalize() override;

  /**
   * @brief Destructor
   */
  virtual ~CaloCalibClusterDecoratorTool() {};

private:

  /**
   * @brief Calculate truth energies for a given cluster
   */
  StatusCode calculateTruthEnergies(const xAOD::CaloCluster& theCaloCluster,
                                    unsigned int numTruthParticles,
                                    const std::map<Identifier,std::vector<const CaloCalibrationHit*> >& identifierToCaloHitMap,
                                    std::vector<std::pair<unsigned int,double>>& truthIDTrueCalHitEnergy) const;

  /**
   * @brief ReadHandleKey for the map between Identifiers and sets of calibration hits
   */
  SG::ReadHandleKey<std::map<Identifier,std::vector<const CaloCalibrationHit*> > >
    m_mapIdentifierToCalibHitsReadHandleKey{this,"IdentifierToCalibHitsMapName","IdentifierToCalibHitsMap",
                                            "ReadHandleKey for the map between Identifiers and sets of calibration hits"};

  /**
   * @brief Write handle key to decorate CaloCluster with threeN leading truth particle barcode and energy
   */
  SG::WriteDecorHandleKey<xAOD::CaloClusterContainer>
    m_caloClusterWriteDecorHandleKeyNLeadingTruthParticles{
      this,
      "CaloClusterWriteDecorHandleKey_NLeadingTruthParticles",
      "CaloTopoClustersNew.calclus_NLeadingTruthParticleBarcodeEnergyPairs"};

  /**
   * @brief Allow user to set the number of truth particles per clusterCaloCluster or PFO,
   * in descending pt order, for which to store calibration hit enery
   */
  Gaudi::Property<unsigned int> m_numTruthParticles{
      this,"NumTruthParticles",100,
      "Set number of truth particles per CaloCluster/PFO for which we store calibration hit energy"};

  /**
   * @brief Toggle whether to use cell weights or not to calculate calibration hit contribution
   */
  Gaudi::Property<bool> m_useCellWeights{
      this,"useCellWeights",true,
      "Toggle whether to use cell weights or not to calculate calibration hit contribution"};

  /**
   * @brief Toggle whether to store full truth energy (include invisible + escaped)
   */
  Gaudi::Property<bool> m_storeFullTruthEnergy{
      this,"StoreFullTruthEnergy",false,
      "If true, include escaped + invisible energy in truth energy calculation"};

  /**
   * @brief External tool that handles calculation logic of truth energies
   */
  ToolHandle<ICaloCalibClusterTruthAttributerTool>
    m_truthAttributerTool{
      this,"TruthAttributerTool","",
      "ToolHandle to a tool to create the calibration hit truth information that we need for the decoration"};

};

#endif // CALOCALIBCLUSTERDECORATORTOOL_H