/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOCALIBHITREC_CALOCALIBCLUSTERMOMENTSMAKER2_H
#define CALOCALIBHITREC_CALOCALIBCLUSTERMOMENTSMAKER2_H
/**
 * @class CaloCalibClusterMomentsMaker2
 * @version \$Id: CaloCalibClusterMomentsMaker2.h,v 1.8 2009-05-18 16:16:48 pospelov Exp $
 * @author Sven Menke <menke@mppmu.mpg.de>, Gennady Pospelov <guennadi.pospelov@cern.ch>
 * @date 17-June-2008
 * @brief Calculate calibration hit based moments for CaloCluster objects using Primary Particle ID
 *
 * This is a CaloClusterCollectionProcessor which can be plugged into
 * a CaloClusterMaker for calculating calibration hit based moments
 * for each CaloCluster. */

class CaloCell_ID;
class CaloDM_ID;
class CaloDmDescrManager;
class CaloDetDescrManager;
class McEventCollection;
class TruthParticleContainer;

#include "GaudiKernel/ToolHandle.h" 

#include "CaloUtils/CaloClusterCollectionProcessor.h"
#include "CaloGeoHelpers/CaloSampling.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloDmDetDescr/CaloDmDescrManager.h"
#include "CaloSimEvent/CaloCalibrationHitContainer.h"
#include "StoreGate/ReadHandleKeyArray.h"

#include "xAODTruth/TruthParticleContainer.h"
#include "CLHEP/Units/SystemOfUnits.h"

#include <string>
#include <vector>
#include <set>
#include <map>
#include <atomic>
#include <array>

class CaloCalibClusterMomentsMaker2: public AthAlgTool, virtual public CaloClusterCollectionProcessor
{
 public:

  /**
   * @brief Class to define range of valid bins in eta x phi plane
   */
  class CalibHitIPhiIEtaRange {
    public:
      char iPhi,iEtaMin,iEtaMax;
  };

  /**
   * @brief Class to store cluster number and weight for calorimeter cells
   */
  class MyCellInfo : public std::vector<std::pair<int, double> > {
    public:
      MyCellInfo(int iClus, double w) { this->emplace_back(iClus, w); }
      void Add(const MyCellInfo& other) { this->insert(this->end(),other.begin(), other.end()); }
  };

  typedef std::map<Identifier, MyCellInfo> CellInfoSet_t;

  /**
   * @brief Class to store cluster's calibration energies
   */
  class MyClusInfo {
    public:
      class ClusCalibEnergy {
        public:
          double engTot = 0.0;
          std::array<double,CaloSampling::Unknown+1> engSmp;
          void Add(double eng, int nsmp)
          {
            engTot += eng;
            engSmp[nsmp] +=eng;
          }
      };

      void Add(double eng, int nsmp, int pid = 0)
      {
        engCalibIn.Add(eng, nsmp);
        engCalibParticle[pid].Add(eng, nsmp);
      }

      ClusCalibEnergy engCalibIn;
      double engCalibOut  = 0.0;
      double engCalibDead = 0.0;
      std::array<double,CaloDmDescrArea::DMA_MAX> engCalibDeadInArea{};
      std::map<int, ClusCalibEnergy > engCalibParticle{};
  };
  typedef std::vector<MyClusInfo> ClusInfo_t;


  /** @brief typedef for a pair to index the enums defined in
    *  CaloClusterMoment with a string. */
  typedef std::pair<std::string,xAOD::CaloCluster::MomentType> 
  moment_name_pair;

  /** @brief vector of pairs defined above.*/
  typedef std::vector<moment_name_pair> moment_name_vector;

  /** @brief set of pairs defined above.*/
  typedef std::set<moment_name_pair> moment_name_set;

  CaloCalibClusterMomentsMaker2(const std::string& type, const std::string& name,
                 const IInterface* parent);


  using CaloClusterCollectionProcessor::execute;
  virtual StatusCode execute(const EventContext& ctx,
                             xAOD::CaloClusterContainer* theClusColl) const override;
  virtual StatusCode initialize() override;

 private:

  /** 
   * @brief vector holding the input list of names of moments to
   * calculate.
   *
   * This is the list of desired names of moments given in the
   * jobOptions.*/
  Gaudi::Property<std::vector<std::string>>  m_momentsNames{this, "MomentsNames", {}};

  /** 
   * @brief vector holding the names of valid moments which can be
   * calculated.
   *
   * Each name has to correspond to one of the enums in
   * CaloClusterMoment. This list is defined in the constructor of
   * CaloCalibClusterMomentsMaker2 and holds the names and enums of
   * moments defined in CaloClusterMoment. The name is used as the key
   * to the enum. */
  moment_name_vector m_validNames;

  /** 
   * @brief set of moments which will be calculated.
   *
   * This set will hold each valid enum indexed with the name if it
   * was found on the input list (m_momentsNames) and in the list of
   * valid moment names (m_validNames). */
  moment_name_set m_validMoments;

  /** 
   * @brief vector holding the list of moment names which go in the first store
   * i.e. they are available on the AOD.
   *
   * Only moments listed in this property can later be directly retrieved from
   * AOD - the others are available in the ESD ... */
  Gaudi::Property<std::vector<std::string>>  m_momentsNamesAOD{this, "AODMomentsNames", {}};

  /** 
   * @brief set holding the list of moment enums which go in the first store
   * i.e. they are available on the AOD.
   *
   * Only moments listed in this property can later be directly retrieved from
   * AOD - the others are available in the ESD ... */
  std::set<xAOD::CaloCluster::MomentType>  m_momentsAOD; 

  /** 
   * @brief vector of calibration hit container names to use. 
   *
   * The containers specified in this property should hold calibration
   * hits inside the calorimeter systems. */
  SG::ReadHandleKeyArray<CaloCalibrationHitContainer> m_CalibrationHitContainerNames{this, "CalibrationHitContainerNames", {}};

  /** 
   * @brief vector of dead material calibration hit container names to use. 
   *
   * The containers specified in this property should hold calibration
   * hits outside the calorimeter systems - i.e. dead material hits ... */
  SG::ReadHandleKeyArray<CaloCalibrationHitContainer> m_DMCalibrationHitContainerNames{this, "DMCalibrationHitContainerNames", {}};

  /** ReadHandleKey for truth particle container */
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticleContainerKey{this,"TruthParticles","TruthParticles","ReadHandleKey for truth particle container"};
  
  /** Conditions Handle Key to access the CaloDetDescrManager */
  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloDetDescrMgrKey{this,"CaloDetDescrManager", "CaloDetDescrManager"};

  const CaloCell_ID* m_calo_id{};
  const CaloDM_ID*   m_caloDM_ID{};
  const CaloDmDescrManager* m_caloDmDescrManager{};

  int m_n_phi_out{127}; // not more than 127 since we store indices (-127,...-1,0,...,126) and have 8 bits only
  int m_n_eta_out{127};
  double m_out_phi_max{M_PI};
  double m_out_eta_max{6};

  double m_rmaxOut[3]{1.0, 0.5, 0.3};

  std::array<std::vector<std::vector<CalibHitIPhiIEtaRange>>,3>  m_i_phi_eta;

  mutable std::atomic<bool> m_foundAllContainers{false};

  enum keys_dm_energy_sharing {kMatchDmOff, kMatchDmLoose, kMatchDmMedium, kMatchDmTight};
  enum keys_calib_frac_origin {kCalibFracEM, kCalibFracHAD, kCalibFracREST, kCalibFracMax};

  bool m_doDeadEnergySharing{false};
  bool m_doOutOfClusterL{false};
  bool m_doOutOfClusterM{false};
  bool m_doOutOfClusterT{false};
  bool m_doDeadL{false};
  bool m_doDeadM{false};
  bool m_doDeadT{false};
  Gaudi::Property<bool> m_useParticleID{this, "UseParticleID", true};
  bool m_doCalibFrac{false};
  float m_energyMin{200*CLHEP::MeV};
  float m_energyMinCalib{20*CLHEP::MeV};
  float m_apars_alpha{0.5};
  float m_apars_r0{0.2};
  Gaudi::Property<int> m_MatchDmType{this, "MatchDmType", kMatchDmLoose};

  static double angle_mollier_factor(double x) ;
};

#endif // CALOCALIBCLUSTERMOMENTSMAKER2_H






