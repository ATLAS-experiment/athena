/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "egammaTransformerCalib/egammaTransformerSvc.h"
#include "xAODEgamma/Egamma.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODEgamma/EgammaDefs.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "PathResolver/PathResolver.h"

#include <TFile.h>
#include <TObjString.h>


StatusCode egammaTransformerSvc::initialize()
{
  ATH_MSG_DEBUG("In initialize of " << name() << "..." );

  if (!m_mvaElectron.empty()) {
    ATH_MSG_DEBUG("Retrieving mvaElectron");
    ATH_CHECK(m_mvaElectron.retrieve());
  } else {
    ATH_MSG_DEBUG("Disabling mvaElectron");
    m_mvaElectron.disable();
  }

  if (!m_mvaFwdElectron.empty()) {
    ATH_MSG_DEBUG("Retrieving mvaFwdElectron");
    ATH_CHECK(m_mvaFwdElectron.retrieve());
  } else {
    ATH_MSG_DEBUG("Disabling mvaFwdElectron");
    m_mvaFwdElectron.disable();
  }

  if (!m_mvaUnconvertedPhoton.empty()) {
    ATH_MSG_DEBUG("Retrieving mvaUnconvertedPhoton");
    ATH_CHECK(m_mvaUnconvertedPhoton.retrieve());
  } else {
    ATH_MSG_DEBUG("Disabling mvaUnconvertedPhoton");
    m_mvaUnconvertedPhoton.disable();
  }

  if (!m_mvaConvertedPhoton.empty()) {
    ATH_MSG_DEBUG("Retrieving mvaConvertedPhoton");
    ATH_CHECK(m_mvaConvertedPhoton.retrieve());
  } else {
    ATH_MSG_DEBUG("Disabling mvaConvertedPhoton");
    m_mvaConvertedPhoton.disable();
  }

  ATH_CHECK(resolve_flags());

  ATH_MSG_INFO(m_maxConvR);
  ATH_MSG_INFO(m_removeTRTConvBarrel);

  return StatusCode::SUCCESS;
}

StatusCode egammaTransformerSvc::resolve_flags()
{
  // For transformer, we always remove the TRT converted photons in the barrel to make it consistent with the training. Therefore we do not support other options for m_removeTRTConvBarrel and set it to true by default. If users set it to other values, we will print a warning and set it to true.
  const bool removeTRTConvBarrelDefault = true;
  if (m_removeTRTConvBarrel == -1) {
    m_removeTRTConvBarrel = removeTRTConvBarrelDefault;
  }
  else if (!m_removeTRTConvBarrel) {
    ATH_MSG_WARNING("m_removeTRTConvBarrel is set to " << m_removeTRTConvBarrel << " which is not supported for Transformer which excluded TRT conversion in the model training. Setting it to " << removeTRTConvBarrelDefault);
    m_removeTRTConvBarrel = removeTRTConvBarrelDefault;
  }
  else {
    ATH_MSG_DEBUG("TRT converted photons in the barrel will be removed for calibration, which is consistent with the training of the Transformer model.");
  }

  return StatusCode::SUCCESS;
}

bool egammaTransformerSvc::isConvCalib(const xAOD::Photon& ph) const
{
  bool isConvCalib = xAOD::EgammaHelpers::isConvertedPhoton(&ph) && 
                     xAOD::EgammaHelpers::conversionRadius(&ph) < m_maxConvR;
  if (m_removeTRTConvBarrel) {
    // special case in Run3 to avoid TRT converted photons in the barrel
    isConvCalib = isConvCalib && xAOD::EgammaHelpers::isConvertedPhoton(&ph, true);
  }
  return isConvCalib;
}

// Note for both egammaTransformerSvc::getEnergy overloads below:
// zero-response handling is delegated to the underlying calibration tools
// (m_mvaElectron/m_mvaFwdElectron/m_mvaConvertedPhoton/m_mvaUnconvertedPhoton).
// By default their property useClusterIf0 (m_clusterEif0) is true, so a zero
// model response returns the uncalibrated cluster energy instead of 0.
// Therefore mvaE is not expected to be 0 by default unless the user explicitly
// changes that tool property in the calibration configuration.

StatusCode egammaTransformerSvc::getEnergy(const xAOD::CaloCluster& cluster,
                                   const xAOD::Egamma& eg,
                                   double& mvaE,
				   const egammaMVACalib::GlobalEventInfo& gei) const
{

  ATH_MSG_DEBUG("calling egammaTransformerSvc::getEnergy with cluster and eg");

  mvaE = 0.;

  if (xAOD::EgammaHelpers::isElectron(&eg)) {
    if (!m_mvaElectron.empty()) {
      mvaE = m_mvaElectron->getEnergy(cluster, &eg, gei);
    } else {
      ATH_MSG_FATAL("Trying to calibrate an electron, but disabled");
      return StatusCode::FAILURE;
    }
  } else if (xAOD::EgammaHelpers::isFwdElectron(&eg)) {
    if (!m_mvaFwdElectron.empty()) {
      mvaE = m_mvaFwdElectron->getEnergy(cluster, &eg, gei);
    } else {
      ATH_MSG_FATAL("Trying to calibrate a forward electron, but disabled");
      return StatusCode::FAILURE;
    }
  } else if (xAOD::EgammaHelpers::isPhoton(&eg)) {
    const xAOD::Photon* ph = static_cast<const xAOD::Photon*>(&eg);
    const bool is_conv_calib = isConvCalib(*ph);
    if (is_conv_calib) {
      if (!m_mvaConvertedPhoton.empty()) {
        mvaE = m_mvaConvertedPhoton->getEnergy(cluster, &eg, gei);
      } else {
        ATH_MSG_FATAL("Trying to calibrate a converted photon, but disabled");
        return StatusCode::FAILURE;
      }
    } else {
      if (!m_mvaUnconvertedPhoton.empty()) {
        mvaE = m_mvaUnconvertedPhoton->getEnergy(cluster, &eg, gei);
      } else {
        ATH_MSG_FATAL("Trying to calibrate an unconverted photon, but disabled");
        return StatusCode::FAILURE;
      }
    }
  } else {
    ATH_MSG_FATAL("Egamma object is of unsupported type");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG( "Calculated MVA calibrated energy = " << mvaE );
  return StatusCode::SUCCESS;
}

StatusCode egammaTransformerSvc::getEnergy(const xAOD::CaloCluster& cluster,
                                   const xAOD::EgammaParameters::EgammaType egType,
                                   double& mvaE,
				   const egammaMVACalib::GlobalEventInfo& gei) const
{

  ATH_MSG_DEBUG("calling egammaTransformerSvc::getEnergy with cluster and egType (" << egType <<")");

  mvaE = 0.0;
  switch (egType) {
  case xAOD::EgammaParameters::electron:
    if (!m_mvaElectron.empty()) {
      mvaE = m_mvaElectron->getEnergy(cluster,nullptr, gei);
    } else {
      ATH_MSG_FATAL("Trying to calibrate an electron, but disabled");
      return StatusCode::FAILURE;
    }
    break;
  case xAOD::EgammaParameters::forwardelectron:
    if (!m_mvaFwdElectron.empty()) {
      mvaE = m_mvaFwdElectron->getEnergy(cluster,nullptr, gei);
    } else {
      ATH_MSG_FATAL("Trying to calibrate a forward electron, but disabled");
      return StatusCode::FAILURE;
    }
    break;
  case xAOD::EgammaParameters::convertedPhoton:
  case xAOD::EgammaParameters::unconvertedPhoton:
    // treat converted photons like unconverted photons since don't have access to vertex
    if (!m_mvaUnconvertedPhoton.empty()) {
      mvaE = m_mvaUnconvertedPhoton->getEnergy(cluster,nullptr, gei);
    } else {
      ATH_MSG_FATAL("Trying to calibrate an unconverted photon, but disabled");
      return StatusCode::FAILURE;
    }
    break;
  default:
    ATH_MSG_FATAL("Egamma object " << egType << " is of unsupported type");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG( "Calculated MVA calibrated energy = " << mvaE );
  return StatusCode::SUCCESS;
}


StatusCode egammaTransformerSvc::execute(xAOD::CaloCluster& cluster,
                                 const xAOD::Egamma& eg,
				 const egammaMVACalib::GlobalEventInfo& gei) const
{
  double mvaE = 0.;

  ATH_CHECK(getEnergy(cluster, eg, mvaE, gei));

  if (mvaE > eg.m()) {
    cluster.setCalE(mvaE);
  }
  else {
    ATH_MSG_DEBUG("MVA energy (" << mvaE << ") < particle mass ("
                  << eg.m() << "), setting e = cluster energy (" << cluster.e() << ")");
    cluster.setCalE(cluster.e());
  }
  return StatusCode::SUCCESS;
}

StatusCode egammaTransformerSvc::execute(xAOD::CaloCluster& cluster,
                                 const xAOD::EgammaParameters::EgammaType egType,
				 const egammaMVACalib::GlobalEventInfo& gei) const
{

  double mvaE = 0.;

  ATH_CHECK(getEnergy(cluster, egType, mvaE, gei));

  if (mvaE > 0) {
    cluster.setCalE(mvaE);
  }
  else {
    ATH_MSG_DEBUG("MVA energy (" << mvaE << ") < 0, setting e = cluster energy ("
		  << cluster.e() << ")");
    cluster.setCalE(cluster.e());
  }
  return StatusCode::SUCCESS;
}
