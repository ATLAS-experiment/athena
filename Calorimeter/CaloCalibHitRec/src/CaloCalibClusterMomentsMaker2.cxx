/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//-----------------------------------------------------------------------
// File and Version Information:
// $Id: CaloCalibClusterMomentsMaker2.cxx,v 1.16 2009-05-18 16:16:49 pospelov Exp $
//
// Description: see CaloCalibClusterMomentsMaker2.h
// 
// Environment:
//      Software developed for the ATLAS Detector at CERN LHC
//
// Author List:
//      Sven Menke
//
//-----------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "CaloCalibClusterMomentsMaker2.h"

//---------------
// C++ Headers --
//---------------
#include <iterator>
#include <sstream>
#include <set>

#include "CaloEvent/CaloCell.h"
#include "CaloSimEvent/CaloCalibrationHit.h"
#include "CaloSimEvent/CaloCalibrationHitContainer.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloIdentifier/CaloDM_ID.h"

#include "StoreGate/ReadHandle.h"

#include "TruthUtils/MagicNumbers.h"

#include <CLHEP/Vector/LorentzVector.h>
#include <cmath>


using CLHEP::HepLorentzVector;
using CLHEP::MeV;
using CLHEP::cm;


//###############################################################################

CaloCalibClusterMomentsMaker2::CaloCalibClusterMomentsMaker2(const std::string& type, 
                                                             const std::string& name,
                                                             const IInterface* parent)
  : AthAlgTool(type, name, parent), 
    m_calo_id(nullptr),
    m_caloDM_ID(nullptr),
    m_caloDmDescrManager(nullptr),
    m_useParticleID(true),
    m_energyMin(200*MeV),
    m_energyMinCalib(20*MeV),
    m_apars_alpha(0.5),
    m_apars_r0(0.2),
    m_MatchDmType(kMatchDmLoose)
{
  declareInterface<CaloClusterCollectionProcessor>(this);
  // Name(s) of Moments to calculate
  declareProperty("MomentsNames", m_momentsNames);
  m_validNames.push_back(moment_name_pair(std::string("ENG_CALIB_TOT"), xAOD::CaloCluster::ENG_CALIB_TOT));
  m_validNames.push_back(moment_name_pair(std::string("ENG_CALIB_OUT_L"), xAOD::CaloCluster::ENG_CALIB_OUT_L));
  m_validNames.push_back(moment_name_pair(std::string("ENG_CALIB_OUT_M"), xAOD::CaloCluster::ENG_CALIB_OUT_M));
  m_validNames.push_back(moment_name_pair(std::string("ENG_CALIB_OUT_T"), xAOD::CaloCluster::ENG_CALIB_OUT_T));
  m_validNames.push_back(moment_name_pair(std::string("ENG_CALIB_EMB0"), xAOD::CaloCluster::ENG_CALIB_EMB0));
  m_validNames.push_back(moment_name_pair(std::string("ENG_CALIB_EME0"), xAOD::CaloCluster::ENG_CALIB_EME0));
  m_validNames.push_back(moment_name_pair(std::string("ENG_CALIB_TILEG3"), xAOD::CaloCluster::ENG_CALIB_TILEG3));
  m_validNames.push_back(moment_name_pair(std::string("ENG_CALIB_DEAD_TOT"), xAOD::CaloCluster::ENG_CALIB_DEAD_TOT));
  m_validNames.push_back(moment_name_pair(std::string("ENG_CALIB_DEAD_EMB0"), xAOD::CaloCluster::ENG_CALIB_DEAD_EMB0));
  m_validNames.push_back(moment_name_pair(std::string("ENG_CALIB_DEAD_TILE0"), xAOD::CaloCluster::ENG_CALIB_DEAD_TILE0));
  m_validNames.push_back(moment_name_pair(std::string("ENG_CALIB_DEAD_TILEG3"), xAOD::CaloCluster::ENG_CALIB_DEAD_TILEG3));
  m_validNames.push_back(moment_name_pair(std::string("ENG_CALIB_DEAD_EME0"), xAOD::CaloCluster::ENG_CALIB_DEAD_EME0));
  m_validNames.push_back(moment_name_pair(std::string("ENG_CALIB_DEAD_HEC0"), xAOD::CaloCluster::ENG_CALIB_DEAD_HEC0));
  m_validNames.push_back(moment_name_pair(std::string("ENG_CALIB_DEAD_FCAL"), xAOD::CaloCluster::ENG_CALIB_DEAD_FCAL));
  m_validNames.push_back(moment_name_pair(std::string("ENG_CALIB_DEAD_LEAKAGE"), xAOD::CaloCluster::ENG_CALIB_DEAD_LEAKAGE));
  m_validNames.push_back(moment_name_pair(std::string("ENG_CALIB_DEAD_UNCLASS"), xAOD::CaloCluster::ENG_CALIB_DEAD_UNCLASS));
  m_validNames.push_back(moment_name_pair(std::string("ENG_CALIB_FRAC_EM"), xAOD::CaloCluster::ENG_CALIB_FRAC_EM));
  m_validNames.push_back(moment_name_pair(std::string("ENG_CALIB_FRAC_HAD"), xAOD::CaloCluster::ENG_CALIB_FRAC_HAD));
  m_validNames.push_back(moment_name_pair(std::string("ENG_CALIB_FRAC_REST"), xAOD::CaloCluster::ENG_CALIB_FRAC_REST));

  // Name(s) of Moments which can be stored on the AOD - all others go to ESD
  m_momentsNamesAOD.emplace_back("ENG_CALIB_TOT");
  m_momentsNamesAOD.emplace_back("ENG_CALIB_OUT_L");
  m_momentsNamesAOD.emplace_back("ENG_CALIB_OUT_M");
  m_momentsNamesAOD.emplace_back("ENG_CALIB_OUT_T");
  m_momentsNamesAOD.emplace_back("ENG_CALIB_EMB0");
  m_momentsNamesAOD.emplace_back("ENG_CALIB_EME0");
  m_momentsNamesAOD.emplace_back("ENG_CALIB_TILEG3");
  m_momentsNamesAOD.emplace_back("ENG_CALIB_DEAD_TOT");
  m_momentsNamesAOD.emplace_back("ENG_CALIB_DEAD_EMB0");
  m_momentsNamesAOD.emplace_back("ENG_CALIB_DEAD_TILE0");
  m_momentsNamesAOD.emplace_back("ENG_CALIB_DEAD_TILEG3");
  m_momentsNamesAOD.emplace_back("ENG_CALIB_DEAD_EME0");
  m_momentsNamesAOD.emplace_back("ENG_CALIB_DEAD_HEC0");
  m_momentsNamesAOD.emplace_back("ENG_CALIB_DEAD_FCAL");
  m_momentsNamesAOD.emplace_back("ENG_CALIB_DEAD_LEAKAGE");
  m_momentsNamesAOD.emplace_back("ENG_CALIB_DEAD_UNCLASS");
  m_momentsNamesAOD.emplace_back("ENG_CALIB_FRAC_EM");
  m_momentsNamesAOD.emplace_back("ENG_CALIB_FRAC_HAD");
  m_momentsNamesAOD.emplace_back("ENG_CALIB_FRAC_REST");

  declareProperty("AODMomentsNames", m_momentsNamesAOD);
  declareProperty("CalibrationHitContainerNames", m_CalibrationHitContainerNames);
  declareProperty("DMCalibrationHitContainerNames", m_DMCalibrationHitContainerNames);
  m_n_phi_out = 127; // not more than 127 since we store indices (-127,...-1,0,...,126) and have 8 bits only
  m_n_eta_out = 127;
  m_out_phi_max = M_PI;
  m_out_eta_max = 6;

  m_rmaxOut[0] = 1.0;
  m_rmaxOut[1] = 0.5; 
  m_rmaxOut[2] = 0.3;

  for (int im = 0; im < 3; im++) {
    m_i_phi_eta[im].resize(m_n_eta_out);
  }
  m_doDeadEnergySharing = false;
  m_foundAllContainers = false;
  m_doOutOfClusterL = false;
  m_doOutOfClusterM = false;
  m_doOutOfClusterT = false;
  m_doDeadL = false;
  m_doDeadM = false;
  m_doDeadT = false;
  m_doCalibFrac = false;

  declareProperty("MatchDmType", m_MatchDmType);
  declareProperty("UseParticleID", m_useParticleID);
}


//###############################################################################

StatusCode CaloCalibClusterMomentsMaker2::initialize()
{
  ATH_MSG_INFO("Initializing " << name());

  for (const std::string& name : m_momentsNames) {
    bool isValid(false);
    for (const moment_name_pair& vname : m_validNames) {
      if (name == vname.first) {
        m_validMoments.insert(vname);
        isValid = true;
        ATH_MSG_DEBUG("Inserting " << name);
        break;
      }
    }
    if (!isValid) {
      msg() << MSG::ERROR << "Moment " << name
            << " is not a valid Moment name and will be ignored! "
            << "Valid names are:";
      for (unsigned int i = 0; i < m_validNames.size(); i++) {
        msg() << (i == 0 ? " " : ", ") << m_validNames[i].first;
      }
      msg() << endmsg;
    }
  }

  // to switch on clever DeadMaterial assignment procedure
  // and check if tight, medium and/or loose versions for out-of-cluster and 
  // simple dead-material assignment are wanted
  for (const moment_name_pair& vname : m_validNames) {
    switch (vname.second) {
    case xAOD::CaloCluster::ENG_CALIB_DEAD_TOT:
      m_doDeadEnergySharing = true;
      break;
    case xAOD::CaloCluster::ENG_CALIB_OUT_L:
      m_doOutOfClusterL = true;
      break;
    case xAOD::CaloCluster::ENG_CALIB_OUT_M:
      m_doOutOfClusterM = true;
      break;
    case xAOD::CaloCluster::ENG_CALIB_OUT_T:
      m_doOutOfClusterT = true;
      break;
    case xAOD::CaloCluster::ENG_CALIB_FRAC_EM:
      m_doCalibFrac = true;
      break;
    case xAOD::CaloCluster::ENG_CALIB_FRAC_HAD:
      m_doCalibFrac = true;
      break;
    case xAOD::CaloCluster::ENG_CALIB_FRAC_REST:
      m_doCalibFrac = true;
      break;
    default:
      break;
    }
  }
  
  if (m_doCalibFrac && !m_useParticleID) {
    ATH_MSG_INFO("Usage of ParticleID was switched off (UseParticleID==False), no ENG_CALIB_FRAC_* moments will be available");
    m_doCalibFrac = false;
  }

  for (const std::string& name : m_momentsNamesAOD) {
    for (const moment_name_pair& vname : m_validNames) {
      if (vname.first == name) {
        m_momentsAOD.insert(vname.second);
        break;
      }
    }
  }
  if (m_momentsAOD.find(xAOD::CaloCluster::ENG_CALIB_DEAD_TOT) != m_momentsAOD.end()) {
    m_doDeadEnergySharing = true;
  }

  // dead material identifier description manager
  m_caloDmDescrManager = CaloDmDescrManager::instance(); 

  ATH_CHECK(detStore()->retrieve(m_calo_id, "CaloCell_ID"));
  ATH_CHECK(detStore()->retrieve(m_caloDM_ID));

  initializeOutOfClusterDistanceTables(
      m_n_phi_out,
      m_n_eta_out,
      m_out_phi_max,
      m_out_eta_max,
      m_rmaxOut,
      m_i_phi_eta);

  ATH_CHECK(m_CalibrationHitContainerNames.initialize());
  ATH_CHECK(m_DMCalibrationHitContainerNames.initialize());
  ATH_CHECK(m_truthParticleContainerKey.initialize());
  ATH_CHECK(m_caloDetDescrMgrKey.initialize());

  return StatusCode::SUCCESS;
}


void CaloCalibClusterMomentsMaker2::initializeOutOfClusterDistanceTables(
    int n_phi_out,
    int n_eta_out,
    double out_phi_max,
    double out_eta_max,
    const double (&rmaxOut)[3],
    std::array<std::vector<std::vector<CalibHitIPhiIEtaRange>>, 3>& i_phi_eta)
{
  for (int im = 0; im < 3; ++im) {
    i_phi_eta[im].clear();
    i_phi_eta[im].resize(n_eta_out);
  }

  for (int jeta = 0; jeta < n_eta_out; ++jeta) {
    const double eta0 = (jeta + 0.5) * out_eta_max / n_eta_out;
    HepLorentzVector middle(1, 0, 0, 1);
    middle.setREtaPhi(1. / std::cosh(eta0), eta0, 0.0);

    double x_rmaxOut[3];
    for (int im = 0; im < 3; ++im) {
      x_rmaxOut[im] = rmaxOut[im] * angle_mollier_factor(eta0);
    }

    for (int jp = -n_phi_out; jp < n_phi_out; ++jp) {
      const double phi = (jp + 0.5) * out_phi_max / n_phi_out;
      int ietaMin[3] = {n_eta_out, n_eta_out, n_eta_out};
      int ietaMax[3] = {-n_eta_out, -n_eta_out, -n_eta_out};

      for (int je = -n_eta_out; je < n_eta_out; ++je) {
        const double eta = (je + 0.5) * out_eta_max / n_eta_out;
        HepLorentzVector cpoint(1, 0, 0, 1);
        cpoint.setREtaPhi(1. / std::cosh(eta), eta, phi);
        const double r = middle.angle(cpoint.vect());
        for (int im = 0; im < 3; ++im) {
          if (r < x_rmaxOut[im]) {
            if (je < ietaMin[im]) {
              ietaMin[im] = je;
            }
            if (je > ietaMax[im]) {
              ietaMax[im] = je;
            }
          }
        }
      }

      for (int im = 0; im < 3; ++im) {
        if (ietaMin[im] <= ietaMax[im]) {
          CalibHitIPhiIEtaRange theRange{};
          theRange.iPhi = static_cast<char>(jp);
          theRange.iEtaMin = static_cast<char>(ietaMin[im]);
          theRange.iEtaMax = static_cast<char>(ietaMax[im]);
          i_phi_eta[im][jeta].push_back(theRange);
        }
      }
    }
  }
}


void CaloCalibClusterMomentsMaker2::buildCellInfoMap(
    const xAOD::CaloClusterContainer& theClusColl,
    CellInfoSet_t& cellInfo)
{
  cellInfo.clear();

  int iClus = 0;
  for (const xAOD::CaloCluster* theCluster : theClusColl) {
    xAOD::CaloCluster::const_cell_iterator cellIter = theCluster->cell_begin();
    xAOD::CaloCluster::const_cell_iterator cellIterEnd = theCluster->cell_end();

    for (; cellIter != cellIterEnd; ++cellIter) {
      const CaloCell* pCell = *cellIter;
      const Identifier myId = pCell->ID();

      MyCellInfo info(iClus, cellIter.weight());
      CellInfoSet_t::iterator bookmark = cellInfo.lower_bound(myId);
      if (bookmark == cellInfo.end() || bookmark->first != myId) {
        if (bookmark != cellInfo.begin()) {
          --bookmark;
        }
        cellInfo.emplace_hint(bookmark, myId, std::move(info));
      }
      else {
        bookmark->second.Add(info);
      }
    }

    ++iClus;
  }
}


//###############################################################################

void CaloCalibClusterMomentsMaker2::buildOutOfClusterClusterLists(
    const xAOD::CaloClusterContainer& theClusColl,
    const ClusInfo_t& clusInfoVec,
    int n_phi_out,
    int n_eta_out,
    double out_phi_max,
    double out_eta_max,
    const std::array<std::vector<std::vector<CalibHitIPhiIEtaRange>>, 3>& i_phi_eta,
    const std::array<bool, 3>& doOutOfCluster,
    const std::array<ClusList*, 3>& clusLists)
{
  for (unsigned int ii = 0; ii < 3; ++ii) {
    ClusList* pClusList = clusLists[ii];
    if (!doOutOfCluster[ii] || pClusList == nullptr) {
      continue;
    }

    int iClus = -1;
    for (const xAOD::CaloCluster* theCluster : theClusColl) {
      ++iClus;
      const MyClusInfo& clusInfo = clusInfoVec[iClus];

      if (clusInfo.engCalibIn.engTot <= 0.) {
        continue;
      }

      int iEtaSign = 1;
      if (theCluster->eta() < 0.) {
        iEtaSign = -1;
      }

      const int jeta = static_cast<int>(std::floor(n_eta_out * (theCluster->eta() / out_eta_max)));
      int jphi = static_cast<int>(std::floor(n_phi_out * (theCluster->phi() / out_phi_max)));

      if (jeta < -n_eta_out || jeta >= n_eta_out) {
        continue;
      }

      if (jphi < -n_phi_out) {
        jphi += 2 * n_phi_out;
      }
      if (jphi >= n_phi_out) {
        jphi -= 2 * n_phi_out;
      }

      unsigned int iEtaBin = static_cast<unsigned int>(jeta);
      if (jeta < 0) {
        iEtaBin = std::abs(jeta) - 1;
      }

      const std::vector<CalibHitIPhiIEtaRange>& bins = i_phi_eta[ii][iEtaBin];
      for (const CalibHitIPhiIEtaRange& range : bins) {
        int jp = range.iPhi + jphi;
        if (jp < -n_phi_out) {
          jp += 2 * n_phi_out;
        }
        if (jp >= n_phi_out) {
          jp -= 2 * n_phi_out;
        }

        const int jEtaMin = (iEtaSign < 0) ? -range.iEtaMax - 1 : range.iEtaMin;
        const int jEtaMax = (iEtaSign < 0) ? -range.iEtaMin - 1 : range.iEtaMax;

        for (int je = jEtaMin; je <= jEtaMax; ++je) {
          (*pClusList)[(jp + n_phi_out) * (2 * n_eta_out + 1) + je + n_eta_out].push_back(iClus);
        }
      }
    }
  }
}


//###############################################################################

StatusCode
CaloCalibClusterMomentsMaker2::execute(const EventContext& ctx,
                                       xAOD::CaloClusterContainer *theClusColl) const
{  

  ATH_MSG_DEBUG("Starting CaloCalibClusterMomentsMaker2::execute");
  SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloDetDescrMgrKey,ctx};
  const CaloDetDescrManager* calo_dd_man = *caloMgrHandle;

  bool foundAllContainers (true);
  std::vector<const CaloCalibrationHitContainer *> v_cchc;
  for (const SG::ReadHandleKey<CaloCalibrationHitContainer>& key :
         m_CalibrationHitContainerNames)
  {
    SG::ReadHandle<CaloCalibrationHitContainer> cchc (key, ctx);
    if ( !cchc.isValid() ) {
      if (m_foundAllContainers) {
        // print ERROR message only if there was at least one event with
        // all containers
        msg(MSG::ERROR) << "SG does not contain calibration hit container "
                        << key.key() << endmsg;
      }
      foundAllContainers = false;
    }
    else {
      v_cchc.push_back(cchc.cptr());
    }
  }

  std::vector<const CaloCalibrationHitContainer *> v_dmcchc;
  for (const SG::ReadHandleKey<CaloCalibrationHitContainer>& key :
         m_DMCalibrationHitContainerNames)
  {
    SG::ReadHandle<CaloCalibrationHitContainer> cchc (key, ctx);
    if ( !cchc.isValid() ) {
      if (m_foundAllContainers) {
        // print ERROR message only if there was at least one event with
        // all containers
        ATH_MSG_ERROR("SG does not contain DM calibration hit container "
                      << key.key());
      }
      foundAllContainers = false;
    }
    else {
      v_dmcchc.push_back(cchc.cptr());
    }
  }

  if ( !m_foundAllContainers && foundAllContainers ) {
    m_foundAllContainers = true;
  }

  if ( !foundAllContainers ) {
    return StatusCode::SUCCESS;
  }
  ATH_MSG_DEBUG("SG has all containers ");

  // will contain detailed info about cluster calibration energies
  ClusInfo_t clusInfoVec(theClusColl->size());

  CellInfoSet_t cellInfo;
  buildCellInfoMap(*theClusColl, cellInfo);

  xAOD::CaloClusterContainer::iterator clusIter = theClusColl->begin();
  xAOD::CaloClusterContainer::iterator clusIterEnd = theClusColl->end();

  /* ********************************************
   calculate total calib energy inside clusters
   ******************************************** */
  unsigned int nHitsTotal = 0;
  unsigned int nHitsWithoutParticleUID = 0;

  accumulateClusterCalibHits(
      v_cchc,
      cellInfo,
      *m_calo_id,
      clusInfoVec,
      nHitsTotal,
      nHitsWithoutParticleUID,
      m_useParticleID,
      [this]() {
        ATH_MSG_ERROR("Invalid uniqueID detected - this sample cannot be properly analysed.");
      });

  // if all calibration hits have ParticleUID (i.e. GenParticle::id()) == 0
  // when simulation was done without ParticleUID
  bool doCalibFrac = m_doCalibFrac;
  bool useParticleID = m_useParticleID;
  if (m_useParticleID && (nHitsTotal == nHitsWithoutParticleUID)) {
    ATH_MSG_INFO("Calibration hits do not have ParticleUID, ids of particle-caused hits are always 0. Continuing without ParticleID machinery.");
    useParticleID = false;
  }

  // reading particle information for later calculation of calibration energy fraction caused
  // by particles of different types
  SG::ReadHandle<xAOD::TruthParticleContainer> truthParticleContainerReadHandle(m_truthParticleContainerKey, ctx);

  if (doCalibFrac && !truthParticleContainerReadHandle.isValid()) {
    ATH_MSG_WARNING("Invalid read handle to TruthParticleContainer with key: "
                    << m_truthParticleContainerKey.key());
    doCalibFrac = false;
  }

  std::array<std::vector<double>, 3> engCalibOut;

  /* ****************************************************************
   lists of clusters populating given area [eta*phi][iClus list]
   **************************************************************** */
  std::array<ClusList, 3> clusLists;
  const std::array<bool, 3> doOutOfCluster{{
    m_doOutOfClusterL, m_doOutOfClusterM, m_doOutOfClusterT
  }};
  const std::array<bool, 3> doClusterLists{{
    m_doOutOfClusterL || m_doDeadL ||
      (m_doDeadEnergySharing && m_MatchDmType == kMatchDmLoose),
    m_doOutOfClusterM || m_doDeadM ||
      (m_doDeadEnergySharing && m_MatchDmType == kMatchDmMedium),
    m_doOutOfClusterT || m_doDeadT ||
      (m_doDeadEnergySharing && m_MatchDmType == kMatchDmTight)
  }};

  for (unsigned int ii = 0; ii < 3; ++ii) {
    if (doClusterLists[ii]) {
      clusLists[ii].resize((2 * m_n_phi_out + 1) * (2 * m_n_eta_out + 1));
      engCalibOut[ii].resize(theClusColl->size(), 0.0);
    }
  }

  std::array<ClusList*, 3> clusListPtrs{{&clusLists[0], &clusLists[1], &clusLists[2]}};
  buildOutOfClusterClusterLists(
      *theClusColl,
      clusInfoVec,
      m_n_phi_out,
      m_n_eta_out,
      m_out_phi_max,
      m_out_eta_max,
      m_i_phi_eta,
      doClusterLists,
      clusListPtrs);

  /* ****************************************************************
   calculate out-of-cluster energy of clusters
   **************************************************************** */
  if ( m_doOutOfClusterL || m_doOutOfClusterM || m_doOutOfClusterT ) {
    std::array<const ClusList*, 3> constClusListPtrs{{
      &clusLists[0], &clusLists[1], &clusLists[2]
    }};
    accumulateOutOfClusterEnergy(
        v_cchc,
        cellInfo,
        *calo_dd_man,
        clusInfoVec,
        m_n_phi_out,
        m_n_eta_out,
        m_out_phi_max,
        m_out_eta_max,
        doOutOfCluster,
        constClusListPtrs,
        [&engCalibOut](unsigned int ii, int iClus, int /*uniqueID*/, double energy) {
          engCalibOut[ii][iClus] += energy;
        });
  }

  // ------------------------------------------------------------------------
  // calculate dead-material energy of clusters (way2)
  // + energy is shared among clusters within certain area
  // + distance to clusters and energy in specific samplings are used as sharing criteria
  // + calculations are done separately for different dead material areas
  ClusList* pClusList = nullptr;
  if ( m_MatchDmType == kMatchDmLoose ) {
    pClusList = &clusLists[0];
  } else if ( m_MatchDmType == kMatchDmMedium ) {
    pClusList = &clusLists[1];
  } else if ( m_MatchDmType == kMatchDmTight ) {
    pClusList = &clusLists[2];
  }

  if (m_doDeadEnergySharing && pClusList) {

    for (const CaloCalibrationHitContainer* dmcchc : v_dmcchc) {
      for (const CaloCalibrationHit* hit : *dmcchc) {
        Identifier myId = hit->cellID();
        if (m_calo_id->is_lar_dm(myId) || m_calo_id->is_tile_dm(myId)) {
          CaloDmDescrElement* myCDDE(nullptr);
          myCDDE = m_caloDmDescrManager->get_element(myId);
          if ( myCDDE ) {
            int uniqueID(HepMC::UNDEFINED_ID);
            if (useParticleID) {
              uniqueID = HepMC::uniqueID(hit);
            }

            int jeO = (int)floor(m_n_eta_out * (myCDDE->eta() / m_out_eta_max));
            if ( jeO >= -m_n_eta_out && jeO < m_n_eta_out ) {
              int jpO = (int)floor(m_n_phi_out * (myCDDE->phi() / m_out_phi_max));
              if ( jpO < -m_n_phi_out ) {
                jpO += 2 * m_n_phi_out;
              }
              if ( jpO >= m_n_phi_out ) {
                jpO -= 2 * m_n_phi_out;
              }

              /* ****************************************
               share energy of DM hit among different clusters
               ***************************************** */
              int nDmArea = m_caloDmDescrManager->get_dm_area(myId);
              const CaloDmRegion *dmRegion = m_caloDmDescrManager->get_dm_region(myId);
              int hitClusVecIndex = 0;
              std::vector<int> hitClusIndex(theClusColl->size());
              std::vector<double> hitClusEffEnergy(theClusColl->size());
              double hitClusNorm = 0.0;

              // loop over clusters which match given DM hit
              for (unsigned int i_cls = 0;
                   i_cls < (*pClusList)[(jpO + m_n_phi_out) * (2 * m_n_eta_out + 1) + jeO + m_n_eta_out].size();
                   i_cls++) {
                int iClus = (*pClusList)[(jpO + m_n_phi_out) * (2 * m_n_eta_out + 1) + jeO + m_n_eta_out][i_cls];
                xAOD::CaloCluster * theCluster = theClusColl->at(iClus);

                MyClusInfo& clusInfo = clusInfoVec[iClus];
                auto pos = clusInfo.engCalibParticle.find(uniqueID);
                if (pos != clusInfo.engCalibParticle.end()) {
                  double engClusTruthUniqueIDCalib = pos->second.engTot;

                  if (engClusTruthUniqueIDCalib > m_energyMinCalib && theCluster->e() > m_energyMin) {
                    double sum_smp_energy = 0.0;
                    // loop over calo sampling numbers registered for given DM area
                    for (unsigned int i_smp = 0; i_smp < dmRegion->m_CaloSampleNeighbours.size(); i_smp++) {
                      CaloSampling::CaloSample nsmp =
                          (CaloSampling::CaloSample) dmRegion->m_CaloSampleNeighbours[i_smp];
                      if ( (dmRegion->m_CaloSampleEtaMin[i_smp] - 0.5) <= theCluster->eta() &&
                           theCluster->eta() <= (dmRegion->m_CaloSampleEtaMax[i_smp] + 0.5) ) {
                        sum_smp_energy += pos->second.engSmp[nsmp];
                      }
                    }
                    if (sum_smp_energy > 0.0) {
                      double phi_diff = myCDDE->phi() - theCluster->phi();
                      if (phi_diff <= -M_PI) {
                        phi_diff += 2. * M_PI;
                      } else if (phi_diff > M_PI) {
                        phi_diff -= 2. * M_PI;
                      }
                      double eta_diff = (myCDDE->eta() - theCluster->eta());
                      float distance = sqrt(eta_diff * eta_diff + phi_diff * phi_diff);

                      double effEner = pow(sum_smp_energy, m_apars_alpha) * exp(-distance / m_apars_r0);

                      hitClusIndex[hitClusVecIndex] = iClus;
                      hitClusEffEnergy[hitClusVecIndex++] = effEner;
                      hitClusNorm += effEner;
                    }
                  } // good energetic cluster
                } // we found a uniqueID
              } // loop over clusters in the list

              hitClusIndex.resize(hitClusVecIndex);
              hitClusEffEnergy.resize(hitClusVecIndex);

              // now we have to calculate weight for assignment hit energy to cluster
              if (hitClusNorm > 0.0) {
                const double inv_hitClusNorm = 1. / hitClusNorm;
                for (unsigned int i_cls = 0; i_cls < hitClusIndex.size(); i_cls++) {
                  int iClus = hitClusIndex[i_cls];
                  double dm_weight = hitClusEffEnergy[i_cls] * inv_hitClusNorm;
                  if (dm_weight > 1.0 || dm_weight < 0.0) {
                    std::cout << "CaloCalibClusterMomentsMaker2::execute() ->Error! Strange weight "
                              << dm_weight << std::endl;
                    std::cout << hitClusEffEnergy[i_cls] << " " << hitClusNorm << std::endl;
                  }
                  clusInfoVec[iClus].engCalibDeadInArea[nDmArea] += hit->energyTotal() * dm_weight;
                  clusInfoVec[iClus].engCalibDeadInArea[CaloDmDescrArea::DMA_ALL] += hit->energyTotal() * dm_weight;
                }
              } // hit clus norm

            } // loop over united ieta*iphi
          } // myCDDE
        } // tile and lar
      } // loop over DM calibration hits
    } // loop over containers

  } // doDeadEnergySharing

  // fraction of calibration energies caused by different particles
  std::vector<double> engCalibFrac;
  engCalibFrac.resize(kCalibFracMax, 0.0);

  std::map<unsigned int, int> truthIDToPdgCodeMap;

  // loop on truth particle container is slow, so put needed information in a map for faster key lookup in later loops
  for ( const auto *thisTruthParticle : *truthParticleContainerReadHandle ) {

    if (!thisTruthParticle) {
      ATH_MSG_WARNING("Got invalid pointer to TruthParticle");
      continue;
    }

    truthIDToPdgCodeMap[HepMC::uniqueID(thisTruthParticle)] = thisTruthParticle->pdgId();
  } // truth particle loop

  // assign moments
  int iClus = 0;
  for (clusIter = theClusColl->begin(), iClus = 0;
       clusIter != clusIterEnd;
       ++clusIter, ++iClus) {
    xAOD::CaloCluster * theCluster = *clusIter;
    MyClusInfo& clusInfo = clusInfoVec[iClus];

    // total DM energy assigned to cluster
    double eng_calib_dead_tot = clusInfo.engCalibDeadInArea[CaloDmDescrArea::DMA_ALL]
          + clusInfo.engCalibIn.engSmp[CaloSampling::PreSamplerB]
          + clusInfo.engCalibIn.engSmp[CaloSampling::PreSamplerE]
          + clusInfo.engCalibIn.engSmp[CaloSampling::TileGap3];
    // DM energy before barrel presampler, inside it, and between presampler and strips
    double eng_calib_dead_emb0 = clusInfo.engCalibDeadInArea[CaloDmDescrArea::DMA_EMB0]
          + clusInfo.engCalibDeadInArea[CaloDmDescrArea::DMA_EMB1]
          + clusInfo.engCalibIn.engSmp[CaloSampling::PreSamplerB];
    // DM energy between barrel and tile
    double eng_calib_dead_tile0 = clusInfo.engCalibDeadInArea[CaloDmDescrArea::DMA_EMB3_TILE0];
    // DM energy before scintillator and inside scintillator
    double eng_calib_dead_tileg3 = clusInfo.engCalibDeadInArea[CaloDmDescrArea::DMA_SCN]
          + clusInfo.engCalibIn.engSmp[CaloSampling::TileGap3];
    // DM energy before endcap presampler, inside it and between presampler and strips
    double eng_calib_dead_eme0 = clusInfo.engCalibDeadInArea[CaloDmDescrArea::DMA_EME0]
          + clusInfo.engCalibDeadInArea[CaloDmDescrArea::DMA_EME12]
          + clusInfo.engCalibIn.engSmp[CaloSampling::PreSamplerE];
    // DM energy between emec and hec
    double eng_calib_dead_hec0 = clusInfo.engCalibDeadInArea[CaloDmDescrArea::DMA_EME3_HEC0];
    // DM energy before FCAL and between HEC and FCAL
    double eng_calib_dead_fcal = clusInfo.engCalibDeadInArea[CaloDmDescrArea::DMA_FCAL0]
          + clusInfo.engCalibDeadInArea[CaloDmDescrArea::DMA_HEC_FCAL];
    // DM leakage behind the calorimeter
    double eng_calib_dead_leakage = clusInfo.engCalibDeadInArea[CaloDmDescrArea::DMA_LEAK];
    // the rest of DM energy which remains unclassified
    double eng_calib_dead_unclass = eng_calib_dead_tot - eng_calib_dead_emb0 - eng_calib_dead_tile0
          - eng_calib_dead_tileg3 - eng_calib_dead_eme0 - eng_calib_dead_hec0 - eng_calib_dead_fcal
          - eng_calib_dead_leakage;

    if (doCalibFrac) {
/*****************************************************************************
Calculation of energy fraction caused by particles of different types
*****************************************************************************/
      engCalibFrac.assign(kCalibFracMax, 0.0);
      if (clusInfo.engCalibIn.engTot > 0.0) {
        // each MyClusInfo has a map of particle's uniqueID (GenParticle::id()) and particle calibration deposits in given cluster
        for (const auto& p : clusInfo.engCalibParticle) {
          int pdg_id = 0;
          if (auto it = truthIDToPdgCodeMap.find(p.first); it != truthIDToPdgCodeMap.end()) {
            pdg_id = it->second;
          } else {
            ATH_MSG_WARNING("truthIDToPdgCodeMap cannot find an entry with uniqueID " << p.first);
            continue;
          }
          if (std::abs(pdg_id) == 211) {
            engCalibFrac[kCalibFracHAD] += p.second.engTot;
          } else if (pdg_id == 111 || pdg_id == 22 || std::abs(pdg_id) == 11) {
            engCalibFrac[kCalibFracEM] += p.second.engTot;
          } else {
            engCalibFrac[kCalibFracREST] += p.second.engTot;
          }
        }
        for (size_t i = 0; i < engCalibFrac.size(); i++) {
          engCalibFrac[i] = engCalibFrac[i] / clusInfo.engCalibIn.engTot;
        }
      }
    }

    if ( !m_momentsNames.empty() ) {
      std::vector<double> myMoments(m_validMoments.size(), 0);
      // assign moments
      moment_name_set::const_iterator vMomentsIter = m_validMoments.begin();
      moment_name_set::const_iterator vMomentsIterEnd = m_validMoments.end();

      int iMoment = 0;
      for (; vMomentsIter != vMomentsIterEnd; ++vMomentsIter, ++iMoment) {
        // now calculate the actual moments
        switch (vMomentsIter->second) {
        case xAOD::CaloCluster::ENG_CALIB_TOT:
          ATH_MSG_DEBUG("Inserting ENG_CALIB_TOT");
          myMoments[iMoment] = clusInfo.engCalibIn.engTot;
          break;
        case xAOD::CaloCluster::ENG_CALIB_OUT_L:
          myMoments[iMoment] = engCalibOut[0][iClus];
          break;
        case xAOD::CaloCluster::ENG_CALIB_OUT_M:
          myMoments[iMoment] = engCalibOut[1][iClus];
          break;
        case xAOD::CaloCluster::ENG_CALIB_OUT_T:
          myMoments[iMoment] = engCalibOut[2][iClus];
          break;
        case xAOD::CaloCluster::ENG_CALIB_EMB0:
          myMoments[iMoment] = clusInfo.engCalibIn.engSmp[CaloSampling::PreSamplerB];
          break;
        case xAOD::CaloCluster::ENG_CALIB_EME0:
          myMoments[iMoment] = clusInfo.engCalibIn.engSmp[CaloSampling::PreSamplerE];
          break;
        case xAOD::CaloCluster::ENG_CALIB_TILEG3:
          myMoments[iMoment] = clusInfo.engCalibIn.engSmp[CaloSampling::TileGap3];
          break;
        case xAOD::CaloCluster::ENG_CALIB_DEAD_TOT:
          myMoments[iMoment] = eng_calib_dead_tot;
          break;
        case xAOD::CaloCluster::ENG_CALIB_DEAD_EMB0:
          myMoments[iMoment] = eng_calib_dead_emb0;
          break;
        case xAOD::CaloCluster::ENG_CALIB_DEAD_TILE0:
          myMoments[iMoment] = eng_calib_dead_tile0;
          break;
        case xAOD::CaloCluster::ENG_CALIB_DEAD_TILEG3:
          myMoments[iMoment] = eng_calib_dead_tileg3;
          break;
        case xAOD::CaloCluster::ENG_CALIB_DEAD_EME0:
          myMoments[iMoment] = eng_calib_dead_eme0;
          break;
        case xAOD::CaloCluster::ENG_CALIB_DEAD_HEC0:
          myMoments[iMoment] = eng_calib_dead_hec0;
          break;
        case xAOD::CaloCluster::ENG_CALIB_DEAD_FCAL:
          myMoments[iMoment] = eng_calib_dead_fcal;
          break;
        case xAOD::CaloCluster::ENG_CALIB_DEAD_LEAKAGE:
          myMoments[iMoment] = eng_calib_dead_leakage;
          break;
        case xAOD::CaloCluster::ENG_CALIB_DEAD_UNCLASS:
          myMoments[iMoment] = eng_calib_dead_unclass;
          break;
        case xAOD::CaloCluster::ENG_CALIB_FRAC_EM:
          myMoments[iMoment] = engCalibFrac[kCalibFracEM];
          break;
        case xAOD::CaloCluster::ENG_CALIB_FRAC_HAD:
          myMoments[iMoment] = engCalibFrac[kCalibFracHAD];
          break;
        case xAOD::CaloCluster::ENG_CALIB_FRAC_REST:
          myMoments[iMoment] = engCalibFrac[kCalibFracREST];
          break;
        default:
          // nothing to be done for other moments
          break;
        }

        theCluster->insertMoment(vMomentsIter->second, myMoments[iMoment]);
      }
    }
  }

  return StatusCode::SUCCESS;
}


/* ****************************************************************************

**************************************************************************** */
double CaloCalibClusterMomentsMaker2::angle_mollier_factor(double x) 
{
  double eta = fabs(x);
  double ff;
  if (eta < 1.6) {
    ff = atan(5.0 * 1.7 / (200.0 * cosh(eta)));
  } else if (eta < 3.2) {
    ff = atan(5.0 * 1.6 / (420. / tanh(eta)));
  } else {
    ff = atan(5.0 * 0.95 / (505. / tanh(eta)));
  }
  return ff * (1. / atan(5.0 * 1.7 / 200.0));
}
