/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Masahiro Morii, Harvard University
 */

// class include
#include "SolenoidTest.h"

// root
#include "TFile.h"
#include "TRandom3.h"
#include "TTree.h"

//
#include <cmath>
#include <vector>

///////////////////////////////////////////////////////////////////
// Public methods:
///////////////////////////////////////////////////////////////////

// Constructor
////////////////
MagField::SolenoidTest::SolenoidTest(const std::string& name,
                                     ISvcLocator* pSvcLocator)
    : AthAlgorithm(name, pSvcLocator), m_thistSvc("THistSvc/THistSvc", name) {}

// Athena hook:
StatusCode MagField::SolenoidTest::initialize() {
  ATH_CHECK(m_fieldCacheKey.initialize());
  // retrieve the histogram service
  if (m_thistSvc.retrieve().isFailure()) {
    ATH_MSG_ERROR("Unable to retrieve HistogramSvc");
    return StatusCode::FAILURE;
  }
  // Create the prefix of histogram names for the THistSvc
  std::string prefix = "/" + m_histStream + "/";

  std::string treeName = m_treeName.value();
  // the ROOT tree
  m_tree = new TTree(treeName.c_str(), "Magnetic field in the solenoid");
  m_tree->Branch("pos", &m_xyzt, "x/D:y/D:z/D");
  m_tree->Branch("fieldFull", &m_field, "Bx/D:By/D:Bz/D");
  m_tree->Branch("deriv", &m_deriv,
                 "dBxdx/D:dBxdy/D:dBxdz/D:dBydx/D:dBydy/D:dBydz/"
                 "D:dBzdx/D:dBzdy/D:dBzdz/D");
  m_tree->Branch("fieldFast", &m_fieldZR, "Bx2/D:By2/D:Bz2/D");
  m_tree->Branch("derivZR", &m_derivZR,
                 "dBxdx2/D:dBxdy2/D:dBxdz2/D:dBydx2/D:dBydy2/D:dBydz2/"
                 "D:dBzdx2/D:dBzdy2/D:dBzdz2/D");
  // register this ROOT TTree to the THistSvc
  if (m_thistSvc->regTree(prefix + treeName, m_tree).isFailure()) {
    ATH_MSG_ERROR("Unable to register TTree to THistSvc");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode MagField::SolenoidTest::execute() {
  SG::ReadCondHandle<AtlasFieldCacheCondObj> rh{m_fieldCacheKey,
                                                Gaudi::Hive::currentContext()};
  const AtlasFieldCacheCondObj* fieldCondObj{*rh};
  if (fieldCondObj == nullptr) {
    ATH_MSG_ERROR("Failed to retrieve AtlasFieldCacheCondObj with key "
                  << m_fieldCacheKey.key());
    return StatusCode::FAILURE;
  }

  MagField::AtlasFieldCache fieldCache;
  fieldCondObj->getInitializedCache(fieldCache);

  if (m_eventsSeen == m_event) {

    double deltaZ = (m_maxZ - m_minZ) / (m_stepsZ - 1);
    double deltaR = (m_maxR - m_minR) / m_stepsR - 1;
    double deltaPhi = (2 * M_PI) / (m_stepsPhi - 1);

    for (int k = 0; k < m_stepsPhi; k++) {  // loop over phi
      double phi = deltaPhi * k;
      for (int j = 0; j < m_stepsZ; j++) {  // loop over Z
        m_xyzt[2] = m_minZ + deltaZ * j;
        for (int i = 0; i < m_stepsR; i++) {  // loop over R
          double r = m_minR + deltaR * i;
          m_xyzt[0] = r * cos(phi);
          m_xyzt[1] = r * sin(phi);

          fieldCache.getField(m_xyzt, m_field, m_deriv);
          fieldCache.getFieldZR(m_xyzt, m_fieldZR, nullptr);
          ATH_MSG_INFO("Field at r,phi " << r << ", " << phi << ", "
                                        << " with xyz: " << m_xyzt[0] << ", "
                                        << m_xyzt[1] << ", " << m_xyzt[2]
                                        << " with value " << m_field[0] << ", "
                                        << m_field[1] << ", " << m_field[2]);

          m_tree->Fill();
        }
      }
    }
  }
  m_eventsSeen++;
  return StatusCode::SUCCESS;
}

