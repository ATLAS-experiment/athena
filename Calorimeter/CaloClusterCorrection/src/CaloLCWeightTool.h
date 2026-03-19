/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOUTILS_CALOLCWEIGHTTOOL_H
#define CALOUTILS_CALOLCWEIGHTTOOL_H
/**
 * @class CaloLCWeightTool
 * @version \$Id: CaloLCWeightTool.h,v 1.8 2009-01-27 09:09:14 gunal Exp $
 * @author Sven Menke <menke@mppmu.mpg.de>
 * @date 26-August-2009
 * @brief calculates hadronic cell weights based on cluster and cell quantities
 *
 * concrete class implementing a IClusterCellWeightTool to calculate
 * the H1-type cell hadronic weights for cells inside a cluster.  The
 * cluster moments and its energy are also used to derive the weights
 * - therefore the weighting is not called cell-by-cell, but for the
 * entire cluster This tool reads its data from pool containing
 * TProfile2D based weighting data. */

#include "CaloInterface/IClusterCellWeightTool.h"
#include "CaloConditions/CaloLocalHadCoeff.h"
#include "GaudiKernel/ToolHandle.h" 
#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "CaloConditions/CaloNoise.h"
#include "GaudiKernel/EventContext.h"

class CaloCell_ID;
class CaloCluster;

class CaloLCWeightTool : public extends<AthAlgTool, IClusterCellWeightTool>
{
 public:
  using base_class::base_class;
  virtual ~CaloLCWeightTool();

  virtual StatusCode weight(xAOD::CaloCluster* theCluster, const EventContext& ctx) const override;
  virtual StatusCode initialize() override;

 private:

  /**
   * @brief name of the key for had cell weights */
  SG::ReadCondHandleKey<CaloLocalHadCoeff> m_key{this, "CorrectionKey", "HadWeights"};

  /**
   * @brief minimal signal/elec_noise ratio for a cell to be weighted
   *
   * Only cells with |energy| above this value times the RMS of the electronics
   * noise are considered in weighting. */
  Gaudi::Property<double>  m_signalOverNoiseCut{this, "SignalOverNoiseCut", 2};

  /**
   * @brief look for em-probability moment and apply relative weight only
   *
   * The classification provides the probability p for the current
   * cluster to be em-like. Hadronic weights are applied with the
   * additional hadronic probablity factor (1-p) to all clusters for
   * the cases EM and HAD. */
  Gaudi::Property<bool>  m_useHadProbability{this, "UseHadProbability", false} ;


  /**
   * @brief vector of names of individual samplings
   *
   * needed to not call many times CaloSamplingHelper::getSamplingName */

  std::vector<std::string> m_sampnames;

  /**
   * @brief interpolate correction coefficients */
  Gaudi::Property<bool> m_interpolate{this, "Interpolate", false};

  /**
   * @brief update also sampling variables */
  Gaudi::Property<bool> m_updateSamplingVars{this, "UpdateSamplingVars", false};
  
  /** 
   * @brief vector of names of dimensions in look-up tables to interpolate */
  Gaudi::Property<std::vector<std::string>>  m_interpolateDimensionNames{this, "InterpolateDimensionNames"
    , {"DIMW_ETA", "DIMW_ENER", "DIMW_EDENS"}};

  /** 
   * @brief actual set of dimension id's to interpolate */
  std::vector<int> m_interpolateDimensions;

  const CaloCell_ID* m_calo_id{};
  
  SG::ReadCondHandleKey<CaloNoise> m_noiseCDOKey{this,"CaloNoiseKey","electronicNoise","SG Key of CaloNoise data object"};
};

#endif


