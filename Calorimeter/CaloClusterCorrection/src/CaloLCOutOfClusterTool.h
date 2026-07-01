/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOUTILS_CALOLCOUTOFCLUSTERTOOL_H
#define CALOUTILS_CALOLCOUTOFCLUSTERTOOL_H
/**
 * @class CaloLCOutOfClusterTool
 * @version \$Id: CaloLCOutOfClusterTool.h,v 1.3 2009-01-27 09:09:14 gunal Exp $
 * @author Sven Menke <menke@mppmu.mpg.de>
 * @date 2-March-2007
 * @brief calculates out-of-cluster corrections based on cluster quantities
 *
 * concrete class implementing a IClusterCellWeightTool to calculate
 * the out-of-cluster corrections for topo clusters.  The
 * cluster moments and its energy are used to derive the correction
 * - therefore the correction is not called cell-by-cell, but for the
 * entire cluster This tool reads its data from pool containing
 * TProfile2D based correction data. */

#include "CaloInterface/IClusterCellWeightTool.h"
#include "CaloConditions/CaloLocalHadCoeff.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/EventContext.h"

class CaloLCOutOfClusterTool : public extends<AthAlgTool, IClusterCellWeightTool>
{
 public:

  using base_class::base_class;
  virtual ~CaloLCOutOfClusterTool();

  virtual StatusCode weight(xAOD::CaloCluster* theCluster, const EventContext& ctx) const override;
  virtual StatusCode initialize() override;

 private:

  /**
   * @brief name of the key for out-of-cluster weights */
  SG::ReadCondHandleKey<CaloLocalHadCoeff> m_key{this, "CorrectionKey", "OOCCorrection"};

  /**
   * @brief look for em-probability moment and apply relative weight only
   *
   * The classification provides the probability p for the current cluster to 
   * be em-like. OOC weights are applied with the additional em-prob factor p
   * to all clusters for the cases EM and HAD. */
  Gaudi::Property<bool>  m_useEmProbability{this, "UseEmProbability", false};

  /**
   * @brief look for em-probability moment and apply relative weight only
   *
   * The classification provides the probability p for the current cluster to 
   * be em-like. OOC weights are applied with the additional had-prob factor (1-p)
   * to all clusters for the cases EM and HAD. */
  Gaudi::Property<bool>  m_useHadProbability{this, "UseHadProbability", false};

  /** 
   * @brief vector of names of the calorimeter samplings not to use
   * when applying the out-of-cluster weights.
   *
   * The pre-sampler layers (barrel and endcap) and the
   * gap-scintillators should not receive out-of-cluster weights as
   * they mainly deal with dead-material corrections. */
  Gaudi::Property<std::vector<std::string>>  m_invalidSamplingNames{this
    , "InvalidSamplings"
    , {"PreSamplerB", "PreSamplerE", "TileGap3"}
  };

  /** 
   * @brief actual set of samplings to be ignored for out-of-cluster weights
   *
   * This set is created according to the names given in the property
   * m_invalidSamplingNames.  */
  std::set<int> m_invalidSamplings;                    

  /**
   * @brief interpolate correction coefficients */
  Gaudi::Property<bool> m_interpolate{this, "Interpolate", false};

  /**
   * @brief update also sampling variables */
  Gaudi::Property<bool> m_updateSamplingVars{this, "UpdateSamplingVars", false};

  /** 
   * @brief vector of names of dimensions in look-up tables to interpolate */
  Gaudi::Property<std::vector<std::string>>  m_interpolateDimensionNames{this
    , "InterpolateDimensionNames"
    , {"DIMO_ENER", "DIMO_ETA", "DIMO_LAMBDA"}
  };
  
  /** 
   * @brief actual set of dimension id's to interpolate */
  std::vector<int> m_interpolateDimensions;

};

#endif


