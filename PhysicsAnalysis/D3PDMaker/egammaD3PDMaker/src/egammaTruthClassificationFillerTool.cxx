/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id$
/**
 * @file egammaD3PDMaker/src/egammaTruthClassificationFillerTool.cxx
 * @author scott snyder
 * @date Jan, 2010
 * @brief Fill in type/origin from MC classifier tool for an egamma.
 */


#include "egammaTruthClassificationFillerTool.h"
#include "xAODEgamma/Egamma.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"
#include "xAODEgamma/EgammaTruthxAODHelpers.h"
#include "AthenaKernel/errorcheck.h"
#include <cstdlib>


namespace D3PD {


/**
 * @brief Standard Gaudi tool constructor.
 * @param type The name of the tool type.
 * @param name The tool name.
 * @param parent The tool's Gaudi parent.
 */
egammaTruthClassificationFillerTool::egammaTruthClassificationFillerTool
  (const std::string& type,
   const std::string& name,
   const IInterface* parent)
    : BlockFillerTool<xAOD::Egamma> (type, name, parent),
      m_classifier ("MCTruthClassifier")
{
  declareProperty ("Classifier", m_classifier, "Classifier tool instance.");

  declareProperty ("DoBkgElecOrigin", m_doBkgElecOrigin = false,
                   "If true, fill in variables for the origin and type "
                   " of a photon for background electrons from conversions.");

  book().ignore();  // Avoid coverity warnings.
}


/**
 * @brief Standard Gaudi initialize method.
 */
StatusCode egammaTruthClassificationFillerTool::initialize()
{
  CHECK( BlockFillerTool<xAOD::Egamma>::initialize() );
  CHECK( m_classifier.retrieve() );
  return StatusCode::SUCCESS;
}


/**
 * @brief Book variables for this block.
 */
StatusCode egammaTruthClassificationFillerTool::book()
{
  CHECK( addVariable ("type",      m_type,
                      "MC particle type, from classifier tool.") );
  CHECK( addVariable ("origin",    m_origin,
                      "MC particle origin, from classifier tool.") );

  if (m_doBkgElecOrigin) {
    CHECK( addVariable ("typebkg", m_typebkg,
                        "Type of photon for background electron "
                        "from conversions, from classifier tool") );
    CHECK( addVariable ("originbkg", m_originbkg,
                        "Origin of photon for background electron "
                        "from conversions, from classifier tool.") );
  }

  return StatusCode::SUCCESS;
}


/**
 * @brief Fill one block --- type-safe version.
 * @param p The input object.
 *
 * This is called once per object.  The caller
 * is responsible for arranging that all the pointers for booked variables
 * are set appropriately upon entry.
 */
StatusCode egammaTruthClassificationFillerTool::fill (const xAOD::Egamma& p)
{
  std::tuple<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin, const xAOD::TruthParticle*,MCTruthPartClassifier::ParticleOutCome> res;

  if (const xAOD::Electron* q =
      dynamic_cast<const xAOD::Electron*>(&p))
  {
    res = m_classifier->particleTruthClassifier_full (q);
  }
  else if (const xAOD::Photon* q =
           dynamic_cast<const xAOD::Photon*>(&p))
  {
    res = m_classifier->particleTruthClassifier_full (q);
  }
  else
    std::abort();

  std::tie(*m_type, *m_origin, std::ignore, std::ignore) = res;

  if (m_doBkgElecOrigin) {
    if (std::get<0>(res) == MCTruthPartClassifier::BkgElectron &&
        std::get<1>(res) == MCTruthPartClassifier::PhotonConv &&
        std::get<2>(res))
    {
      const xAOD::TruthParticle* last = xAOD::EgammaHelpers::getBkgElectronMother(std::get<2>(res));
      if(last){
        std::tie(*m_typebkg, *m_originbkg, std::ignore, std::ignore) = m_classifier->particleTruthClassifier_full(last);
      }
    }
  }

  return StatusCode::SUCCESS;
}


} // namespace D3PD
