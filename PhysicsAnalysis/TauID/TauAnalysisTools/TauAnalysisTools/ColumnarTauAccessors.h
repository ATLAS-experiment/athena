/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef TAUANALYSISTOOLS_TAU_JET_HELPERS_H
#define TAUANALYSISTOOLS_TAU_JET_HELPERS_H

#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarCore/LinkColumn.h>
#include <ColumnarJet/JetDef.h>
#include <ColumnarTau/TauJetDef.h>
#include <ColumnarTruth/TruthParticleDef.h>
#include <ColumnarModeDefault/ColumnarModeDefault.h>
#include <TauAnalysisTools/Enums.h>
#include <TruthUtils/HepMCHelpers.h>

namespace TauAnalysisTools
{
  /// @file accessor for variables that have calculations in @ref xAOD::TauJet
  ///
  /// Essentially this just copies out the relevant parts of the xAOD
  /// class and makes them look like stand-alone accessors.  The name
  /// of each class is derived from the member function in the xAOD
  /// class.


  /// @brief accessor for the truth particle type of a tau
  ///
  /// Note that this has three CIs: the taus, the associated truth
  /// particles, and the associated truth jets. Particularly the last
  /// is potentially problematic, as the tool may also want to connect
  /// to a detector level jet container, and those need to be given
  /// separate CIs.
  template<columnar::ContainerIdConcept CITau   = columnar::TauJetDef,
           columnar::ContainerIdConcept CITruth = columnar::TruthParticleDef,
           columnar::ContainerIdConcept CIJet   = columnar::JetDef,
           typename CM = CMode>
  class TruthParticleTypeAccessor final
  {
    columnar::ColumnAccessor<CITau,   columnar::OptObjectId<CITruth, CM>, CM> m_truthParticleLinkAcc;
    columnar::ColumnAccessor<CITau,   columnar::OptObjectId<CIJet,   CM>, CM> m_truthJetLinkAcc;
    columnar::ColumnAccessor<CITruth, int,                                CM> m_pdgIdAcc;
    columnar::ColumnAccessor<CITruth, char,                               CM> m_isHadronicTauAcc;

  public:

    TruthParticleTypeAccessor (columnar::ColumnarTool<CM>& columnarTool)
      : m_truthParticleLinkAcc (columnarTool, "truthParticleLink"),
        m_truthJetLinkAcc      (columnarTool, "truthJetLink"),
        m_pdgIdAcc             (columnarTool, "pdgId"),
        m_isHadronicTauAcc     (columnarTool, "IsHadronicTau")
    {}

    [[nodiscard]] TruthMatchedParticleType
    operator () (columnar::ObjectId<CITau, CM> tau) const
    {
      const columnar::OptObjectId<CITruth, CM> truthParticle = m_truthParticleLinkAcc(tau);
      if (truthParticle.has_value())
      {
        const int pdgId = m_pdgIdAcc(truthParticle.value());
        if (MC::isTau(pdgId))
          return static_cast<bool>(m_isHadronicTauAcc(truthParticle.value()))
                 ? TruthHadronicTau : TruthLeptonicTau;
        if (MC::isMuon(pdgId))     return TruthMuon;
        if (MC::isElectron(pdgId)) return TruthElectron;
      }
      if (m_truthJetLinkAcc(tau).has_value())
        return TruthJet;
      return Unknown;
    }
  };
}

#endif
