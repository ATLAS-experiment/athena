/*
   Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOCALIBHITREC_CALOCALIBCLUSTERTRUTHMAPMAKERTOOL_H
#define CALOCALIBHITREC_CALOCALIBCLUSTERTRUTHMAPMAKERTOOL_H

#include "CaloCalibHitRec/CaloCalibDefineTypes.h"
#include "CaloUtils/CaloClusterCollectionProcessor.h"

//EDM Container Classes
#include "CaloSimEvent/CaloCalibrationHitContainer.h"
#include "xAODTruth/TruthParticleContainer.h"

//EDM Classes
#include "CaloSimEvent/CaloCalibrationHit.h"
#include "xAODTruth/TruthParticle.h"

//C++ classes
#include <map>
#include <vector>

/**
 * @class CaloCalibClusterTruthMapMakerTool
 * @brief Tool creating lookup maps for calibration hit truth information.
 *
 * This tool creates several maps used for fast access to 
 * information in the calculations related to calibration hit truth energy
 */


class CaloCalibClusterTruthMapMakerTool : public AthAlgTool, virtual public CaloClusterCollectionProcessor {

public:
  /**
   * @brief Constructor
   */
  CaloCalibClusterTruthMapMakerTool(const std::string& type, const std::string& name,
                          const IInterface* parent);
  using CaloClusterCollectionProcessor::execute;
  virtual StatusCode execute(const EventContext& ctx,
                             xAOD::CaloClusterContainer* theClusColl) const override final;
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  /**
   * @brief Destructor
   */
  virtual ~CaloCalibClusterTruthMapMakerTool() {};
  
  

private:
  /**
   * @brief builds a lookup table from cellID to calibration hits
   *
   * Loops over all calibration hits in all calibration hit containers of event 
   * and then associates cell Ids to a vector of calibration hits in that cell 
   * to create efficient lookup maps for future calculations

   * @param identifierToCaloHitMap Output map associating Identifiers with calibration hits that will be filled
   * @param ctx Event context providing event slots
   */

  /** This fills a map between calorimeter cell identifiers and calibration hits for a fast lookup */
  void fillIdentifierToCaloHitMap(std::map<Identifier,std::vector<const CaloCalibrationHit*> >& identifierToCaloHitMap, const EventContext& ctx) const;

/**
   * @brief ReadHandleKey for Active Tile calibration hits.
   */
  SG::ReadHandleKey<CaloCalibrationHitContainer> m_tileActiveCaloCalibrationHitReadHandleKey{
      this,"tileActiveCaloCalibrationHitsName","TileCalibHitActiveCell",
      "ReadHandleKey for Active Tile Calibration Hits"};

  /**
   * @brief ReadHandleKey for Inactive Tile calibration hits.
   */
  SG::ReadHandleKey<CaloCalibrationHitContainer> m_tileInactiveCaloCalibrationHitReadHandleKey{
      this,"tileInactiveCaloCalibrationHitsName","TileCalibHitInactiveCell",
      "ReadHandleKey for Inactive Tile Calibration Hits"};

  /**
   * @brief ReadHandleKey for Tile dead material calibration hits.
   */
  SG::ReadHandleKey<CaloCalibrationHitContainer> m_tileDMCaloCalibrationHitReadHandleKey{
      this,"tileDMCaloCalibrationHitsName","TileCalibHitDeadMaterial",
      "ReadHandleKey for Dead Material Tile Calibration Hits"};

  /**
   * @brief ReadHandleKey for Active LAr calibration hits.
   */
  SG::ReadHandleKey<CaloCalibrationHitContainer> m_lArActiveCaloCalibrationHitReadHandleKey{
      this,"lArActiveCaloCalibrationHitsName","LArCalibrationHitActive",
      "ReadHandleKey for Active LAr Calibration Hits"};

  /**
   * @brief ReadHandleKey for Inactive LAr calibration hits.
   */
  SG::ReadHandleKey<CaloCalibrationHitContainer> m_lArInactiveCaloCalibrationHitReadHandleKey{
      this,"lArInactiveCaloCalibrationHitsName","LArCalibrationHitInactive",
      "ReadHandleKey for Inactive LAr Calibration Hits"};

  /**
   * @brief ReadHandleKey for LAr dead material calibration hits.
   */
  SG::ReadHandleKey<CaloCalibrationHitContainer> m_lArDMCaloCalibrationHitReadHandleKey{
      this,"lArDMCaloCalibrationHitsName","LArCalibrationHitDeadMaterial",
      "ReadHandleKey for Dead Material LAr Calibration Hits"};

  /**
   * @brief ReadHandleKey for the truth particle container.
   */
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticleReadHandleKey{
      this,"truthParticlesName","TruthParticles",
      "ReadHandle for the TruthParticles"};

  /**
   * @brief WriteHandleKey for the Identifier to calibration hits map.
   */
  SG::WriteHandleKey<std::map<Identifier,std::vector<const CaloCalibrationHit*> > >
      m_mapIdentifierToCalibHitsWriteHandleKey{
          this,"IdentifierToCalibHitsMapName","IdentifierToCalibHitsMap",
          "WriteHandleKey for the map between Identifiers and sets of calibration hits"};

};

#endif // CALOCALIBCLUSTERTRUTHMAPMAKERTOOL_H
