/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOUTILS_CALOLCDEADMATERIALTOOL_H
#define CALOUTILS_CALOLCDEADMATERIALTOOL_H

/**
 * @class CaloLCDeadMaterialTool
 * @author Gennady Pospelov <guennadi.pospelov@cern.ch>
 * @date 09-September-2009
 * @brief dead material correction tool for local hadronic calibration
 *
 * Calculates dead material energy of given cluster. Cluster energy, lambda and eta
 * are used to get appropriate set of DM correction coefficients for appropriate dead material
 * area ( dead material area definition and set of correction coefficients are holded by CaloHadDMCoeff2 
 * class).
 * DM coefficients are applied to cluster cells sums to calculate DM energy in different
 * calorimeter areas. Finally, calculated DM energy is added to the cluster by changing 
 * (increasing) cluster cells weights in appropriate samplings. */


#include "CaloInterface/IClusterCellWeightTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloConditions/CaloLocalHadCoeff.h"
#include "GaudiKernel/ToolHandle.h" 
#include "CaloGeoHelpers/CaloSampling.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "GaudiKernel/EventContext.h"
#include "CaloEvent/CaloRecoStatus.h"
#include "CLHEP/Units/SystemOfUnits.h"

#include <map>
#include <vector>
#include <string>


class CaloCell_ID;


//class CaloLCDeadMaterialTool : public AthAlgTool, virtual public IClusterCellWeightTool
class CaloLCDeadMaterialTool : public extends<AthAlgTool, IClusterCellWeightTool>
{
  public:
    using base_class::base_class;
  
    /**
     * @brief Dead Material area number
     *
     * These keys are used to identify specific dead material areas in the calorimeter where
     * procedure of DM correction will be applied */
    enum dm_area_keys
    {
      sDM_EMB0_EMB1,   // 0 before PreSamplerB, between PreSamplerB and EMB1
      sDM_EMB3_TILE0,  // 1 between barrel and tile
      sDM_SCN,         // 2 before TileGap3 (scintillator)
      sDM_EME0_EME12,  // 3 before PreSamplerE, between PreSamplerE and EME1
      sDM_EME3_HEC0,   // 4 between EME3 and HEC0
      sDM_FCAL,        // 5 between HEC and FCAL, before FCAL
      sDM_LEAKAGE,     // 6 leakage behind calorimeter
      sDM_UNCLASS,     // 7 unclassified DM enegry
      sDM
    };

    virtual ~CaloLCDeadMaterialTool() override;

    virtual StatusCode weight(xAOD::CaloCluster* theCluster, const EventContext& ctx) const override;
    virtual StatusCode initialize() override;

  private:
    // Hold information about one cell.
    struct Cell
    {
      int dm;
      float energy;
      float weight;
    };

    // Hold information about one dead material area.
    struct Area
    {
      Area() : eprep(0), erec(0), sub_ener_old(0), corr(0), edm_wrong(0) {}
      float eprep;
      float erec;
      float sub_ener_old;
      float corr;
      float edm_wrong;
    };

    StatusCode prepare_for_cluster(const xAOD::CaloCluster* theCluster,
                                   std::vector<Area>& areas,
                                   std::vector<Cell>& cells,
                                   float* smp_energy,
                                   float& cls_unweighted_energy, const CaloLocalHadCoeff* data) const;

    /**
     * @brief name of the key for DM cell weights */
    SG::ReadCondHandleKey<CaloLocalHadCoeff>  m_key{this, "HadDMCoeffKey", "HadDMCoeff2"};

    /**
     * @brief Required Reco Status for the clusters in order to be calibrated */
    Gaudi::Property<int> m_recoStatus{this, "ClusterRecoStatus", CaloRecoStatus::UNKNOWNSTATUS};

    /**
     * @brief method of assignment of DM energy to cluster
     *
     * if 0: setting cluster energy to the new value without changing of cells weights
     * if 1: weights of all cells in the cluster with energy > m_MinCellEnergyToDeal will be changed
     * if 2: changing only weights of cells which have been involved into DM calculation */
    Gaudi::Property<int> m_weightModeDM{this, "WeightModeDM", 1};

    /**
     * @brief Minimum energy of clusters to apply correction */
    Gaudi::Property<float> m_MinClusterEnergyToDeal{this, "MinClusterEnergyToDeal", 200.0*CLHEP::MeV};

    /**
     * @brief minimum number of events in one lookup bin to use */
    Gaudi::Property<int> m_MinLookupBinNentry{this, "MinLookupBinNentry", 40};

    /**
     * @brief minimum cell energy to deal*/
    Gaudi::Property<float> m_MinCellEnergyToDeal{this, "MinCellEnergyToDeal", 0.0};

    /**
     * @brief maximum allowed change in cell weights */
    Gaudi::Property<float> m_MaxChangeInCellWeight{this, "MaxChangeInCellWeight", 30.0};

    /**
     * @brief calculate DM energy using em-probability moment
     *
     * The classification provides the probability p for the current
     * cluster to be em-like. Dead material enegry of cluster is calculated as
     * a mixture of DM energies for pure EM and pure HAD clusters using p as
     * engDM = p *engDM_EM+(1-p)*endDM_HAD */
    Gaudi::Property<bool> m_useHadProbability{this, "UseHadProbability", false};

    /**
     * @brief interpolate correction coefficients */
    Gaudi::Property<bool> m_interpolate{this, "Interpolate", false};

    /**
     * @brief update also sampling variables */
    Gaudi::Property<bool> m_updateSamplingVars{this, "UpdateSamplingVars", false};

   /** 
    * @brief vector of names of dimensions to interpolate (for different correction types different set of dimensions) */
    Gaudi::Property<std::map<std::string, std::vector<std::string>>>  m_interpolateDimensionNames {this
     , "InterpolateDimensionNames"
     , {
        {"AREA_DMFIT" , {"DIMD_ETA", "DIMD_ENER"}}
	,  {"AREA_DMLOOKUP" , {"DIMD_ETA", "DIMD_ENER", "DIMD_LAMBDA"}}
	, {"AREA_DMSMPW" , {"DIMD_ETA", "DIMD_LAMBDA"}}
      }
    };
  
   /** 
    * @brief actual set of dimension id's to interpolate (in the DM areas corrected with TProfile approach)*/
   std::vector<int > m_interpolateDimensionsFit;
   /** 
    * @brief actual set of dimension id's to interpolate (in the DM areas corrected with lookup approach)*/
   std::vector<int > m_interpolateDimensionsLookup;
   /** 
    * @brief actual set of dimension id's to interpolate (in the DM areas corrected with sampling weight approach)*/
   std::vector<int > m_interpolateDimensionsSampling;
   
   /** 
    * @brief In Abs Option case, DM calculation has to be handled in a slightly different way*/
   Gaudi::Property<bool> m_absOpt{this, "WeightingOfNegClusters", false};

};


#endif
