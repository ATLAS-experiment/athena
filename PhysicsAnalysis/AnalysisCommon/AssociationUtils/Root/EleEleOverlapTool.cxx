/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// System includes
#include <typeinfo>
#include <exception>

// Framework includes
#include "AthContainers/ConstDataVector.h"
#include "FourMomUtils/xAODP4Helpers.h"

// EDM includes
#include "xAODEgamma/EgammaDefs.h"

// Local includes
#include "AssociationUtils/EleEleOverlapTool.h"

namespace
{

  /// Check for dummy float values of variables.
  /// Assuming -999 is the only value of interest for now.
  bool isDummyVal(float x)
  {
    const float epsilon = 1e-5;
    const float dummyVal = -999;
    return std::abs(x - dummyVal) < epsilon;
  }

  // Define simple exception for Dummy/missing values
  class DummyValError : public std::exception
  {};

}

namespace ORUtils
{

  //---------------------------------------------------------------------------
  // Constructor
  //---------------------------------------------------------------------------
  EleEleOverlapTool::EleEleOverlapTool(const std::string& name)
    : BaseOverlapTool(name),
      m_useTrackMatch(true),
      m_useClusterMatch(false),
      m_clusterDeltaEta(3*0.025),
      m_clusterDeltaPhi(5*0.025)
  {
    declareProperty("UseTrackMatch", m_useTrackMatch,
                    "Match electrons by shared track");
    declareProperty("UseClusterMatch", m_useClusterMatch,
                    "Match electrons by cluster proximity");
    declareProperty("ClusterDeltaEta", m_clusterDeltaEta,
                    "Cluster matching delta eta");
    declareProperty("ClusterDeltaPhi", m_clusterDeltaPhi,
                    "Cluster matching delta phi");
  }

  //---------------------------------------------------------------------------
  // Initialize
  //---------------------------------------------------------------------------
  StatusCode EleEleOverlapTool::initializeDerived()
  {
    ATH_MSG_DEBUG("UseTrackMatch " << m_useTrackMatch <<
                  " UseClusterMatch " << m_useClusterMatch <<
                  " ClusterDeltaEta " << m_clusterDeltaEta <<
                  " ClusterDeltaPhi " << m_clusterDeltaPhi);

    // Sanity check
    if(!m_useTrackMatch && !m_useClusterMatch) {
      ATH_MSG_ERROR("You must enable at least one: UseTrackMatch or UseClusterMatch");
      return StatusCode::FAILURE;
    }
    if (m_useClusterMatch)
    {
      // Initialize the cluster accessors
      resetAccessor (m_accessors->m_clusterContainerAcc, *m_accessors, "egammaClusters");
      resetAccessor (m_accessors->m_caloClusterAcc, *m_accessors, "caloClusterLinks");
      m_accessors->m_etaBEAcc.emplace (*m_accessors);
      m_accessors->m_phiBEAcc.emplace (*m_accessors);
    }
    if (m_useTrackMatch)
    {
      // Initialize the track accessor
      resetAccessor (m_accessors->m_track0Acc, *m_accessors, "GSFTrackParticles");
      resetAccessor (m_accessors->m_trackAcc, *m_accessors, "trackParticleLinks");
    }

    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Identify overlaps
  //---------------------------------------------------------------------------
  StatusCode EleEleOverlapTool::
  findOverlaps(columnar::Particle1Range cont1,
               columnar::Particle2Range cont2,
               columnar::EventContextId /*eventContext*/) const
  {
    if constexpr (columnar::ColumnarModeDefault::isXAOD)
    {
      // I require that the two containers are the same so that I can
      // arbitrarily pick one of them to use.
      if(&cont1.getXAODObject() != &cont2.getXAODObject()) {
        ATH_MSG_ERROR("This tool expects both electron containers to be the " <<
                      "same for now");
        return StatusCode::FAILURE;
      }
    }
    // Check the container type
    ATH_CHECK( checkForXAODContainer<xAOD::ElectronContainer>(cont1, "Container is not of type ElectronContainer!") );
    // Call the type-specific method
    ATH_CHECK( internalFindOverlaps(cont1) );
    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Identify overlaps
  //---------------------------------------------------------------------------
  StatusCode EleEleOverlapTool::
  internalFindOverlaps(columnar::Particle1Range electrons) const
  {
    ATH_MSG_DEBUG("Removing overlapping electrons");

    // Initialize output decorations if necessary
    initializeDecorations(electrons);

    // TODO: consider adding cluster-based matching also

    // Loop over surviving electron pairs
    for(const auto el1 : electrons) {
      if(!isSurvivingObject(el1)) continue;
      for(const auto el2 : electrons) {
        if(el1 == el2) continue;
        if(!isSurvivingObject(el2)) continue;

        // Perform the match and decide whether to reject el1
        try {
          if(electronsMatch(el1, el2) && rejectFirst(el1, el2)) {
            ATH_CHECK( handleOverlap(el1, el2) );
          }
        }
        catch(const DummyValError& e) {
          ATH_MSG_ERROR("Cluster 2nd sampling eta/phi values are -999. " <<
                        "It seems you are missing the neccesary variables to "
                        "do the requested electron-electron cluster matching");
          return StatusCode::FAILURE;
        }
      }
    }

    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Apply the ele-ele matching criteria
  //---------------------------------------------------------------------------
  bool EleEleOverlapTool::
  electronsMatch(columnar::Particle1Id el1, columnar::Particle1Id el2) const
  {
    auto& acc = *m_accessors;
    // Look for a shared track
    if(m_useTrackMatch)
    {
      auto trk1 = el1(acc.m_trackAcc);
      auto trk2 = el2(acc.m_trackAcc);
      if(trk1.size() > 0 && trk2.size() > 0 && trk1[0] == trk2[0])
        return true;
    }

    // Look for overlapping clusters
    if(m_useClusterMatch) {
      auto clus1 = el1(acc.m_caloClusterAcc)[0].value();
      auto clus2 = el2(acc.m_caloClusterAcc)[0].value();
      using xAOD::P4Helpers::deltaPhi;

      // We use coordinates from 2nd sampling
      const unsigned layer = 2;
      const float eta1 = clus1(acc.m_etaBEAcc.value(), layer);
      const float eta2 = clus2(acc.m_etaBEAcc.value(), layer);
      const float phi1 = clus1(acc.m_phiBEAcc.value(), layer);
      const float phi2 = clus2(acc.m_phiBEAcc.value(), layer);

      // Check validity of the eta/phi (no dummy -999 values)
      if(isDummyVal(eta1) || isDummyVal(eta2) ||
         isDummyVal(phi1) || isDummyVal(phi2)) {
        throw DummyValError();
      }

      const float dEta = eta1 - eta2;
      const float dPhi = deltaPhi(phi1, phi2);

      if( std::abs(dEta) < m_clusterDeltaEta &&
          std::abs(dPhi) < m_clusterDeltaPhi )
      {
        return true;
      }
    }

    return false;
  }

  //---------------------------------------------------------------------------
  // Decide whether to reject the first electron compared to the 2nd.
  // This function assumes a matching criteria has already been applied.
  //---------------------------------------------------------------------------
  bool EleEleOverlapTool::
  rejectFirst(columnar::Particle1Id el1, columnar::Particle1Id el2) const
  {
    auto& acc = *m_accessors;
    // TODO: consider incorporating track-match information in the priority
    // selection.

    // Always reject author "Ambiguous" when compared to author "Electron"
    if(el1(acc.m_authorAcc) == xAOD::EgammaParameters::AuthorAmbiguous &&
       el2(acc.m_authorAcc) == xAOD::EgammaParameters::AuthorElectron)
      return true;
    if(el1(acc.m_authorAcc) == xAOD::EgammaParameters::AuthorElectron &&
       el2(acc.m_authorAcc) == xAOD::EgammaParameters::AuthorAmbiguous)
      return false;

    // Reject the softer electron
    if(el1(acc.m_ptAcc) < el2(acc.m_ptAcc)) return true;
    return false;
  }

} // namespace ORUtils
