/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
// Decorates a jet with summary information about contained truth vertices
// See TruthVertexDecoratorAlg.h for more information


#ifndef PARTICLEJETTOOLS_JETTRUTHVERTEXSUMMARYDECORATORALG_H
#define PARTICLEJETTOOLS_JETTRUTHVERTEXSUMMARYDECORATORALG_H
#include "ParticleJetTools/FatVertex.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "xAODBase/IParticleContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthVertexContainer.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"
#include "StoreGate/ReadDecorHandleKey.h"

#include "xAODJet/JetContainer.h"

namespace ParticleJetTools{
class JetTruthVertexSummaryDecoratorAlg: public AthReentrantAlgorithm
{
public:

  JetTruthVertexSummaryDecoratorAlg(const std::string& name, ISvcLocator* loc);
  virtual StatusCode initialize () override;
  virtual StatusCode execute (const EventContext&) const override;

  private:

  Gaudi::Property<float> m_drThreshold{
    this, "drThreshold", 0.4, "Vertices are matched to jets with this dR parameter"
  };
  Gaudi::Property<bool> m_onlyID{
    this, "onlyID", false,
    "If true, only consider particles who decay/interact before the end of the ID"
  };
  // https://atlas.cern/updates/experiment-briefing/inner-detector-alignment
  Gaudi::Property<float> m_lxyIDThreshold{
    this, "lxyIDThreshold", 1082.0, "Under this Lxy (and z, see below) will be considered ID"
  };
  // https://www.semanticscholar.org/paper/Study-of-the-material-of-the-ATLAS-inner-detector-2-Collaboration/6ef67c4105099f0667fe4691135ebbb63b9ac5cc/figure/0
  // Fig1
  Gaudi::Property<float> m_zIDThreshold{
    this, "zIDThreshold", 2710.0, "Under this z (and Lxy, see above) will be considered ID"
  };

  SG::ReadHandleKey< xAOD::TruthParticleContainer > m_TruthContainerKey {
    this, "truthContainer", "TruthParticles",
    "Key for the input truth particle collection"};
  SG::ReadHandleKey< xAOD::JetContainer > m_JetContainerKey {
    this, "jetContainer", "AntiKt4EMPFlowJets",
    "Key for the input jet collection"};
  SG::ReadDecorHandleKey< xAOD::TruthParticleContainer > m_vertexValid{
    this, "ftagTPValid", "ftagTPValid",
    "Is this truth particle physical"};
  SG::ReadDecorHandleKey< xAOD::TruthParticleContainer > m_vertexID{
    this, "ftagTPDecayVertexID", "ftagTPDecayVertexID",
    "ID of the vertex associated with this TP"};
  SG::ReadDecorHandleKey< xAOD::TruthParticleContainer > m_vertexSimpleDecayType {
      this, "ftagTPDecaySimpleVertexType", "ftagTPDecaySimpleVertexType",
      "Exclusive label for this truth particles decay type"};
  SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_decoNumBVerts{
    this, "ftagJetNumBVertices", "ftagJetNumBVertices",
    "Number of b vertices in jet"
  };
  SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_decoNumCVerts{
    this, "ftagJetNumCVertices", "ftagJetNumCVertices",
    "Number of c vertices in jet"
  };
  SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_decoNumTauVerts{
    this, "ftagJetNumTauVertices", "ftagJetNumTauVertices",
    "Number of tau vertices in jet"
  };
  SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_decoNumStrangeVerts{
    this, "ftagJetNumStrangeVertices", "ftagJetNumStrangeVertices",
    "Number of strange vertices in jet"
  };
  SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_decoNumPionVerts{
    this, "ftagJetNumPionVertices", "ftagJetNumPionVertices",
    "Number of pion vertices in jet"
  };
  SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_decoNumMaterialIntVerts{
    this, "ftagJetNumMaterialIntVertices", "ftagJetNumMaterialIntVertices",
    "Number of material interaction vertices in jet"
  };
  SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_decoNumOtherVerts{
    this, "ftagJetNumOtherVertices", "ftagJetNumOtherVertices",
    "Number of other vertices in jet"
  };
  SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_decoNumVerts{
    this, "ftagJetNumVertices", "ftagJetNumVertices",
    "Number of vertices in jet"
  };

};
}
#endif // PARTICLEJETTOOLS_JETTRUTHVERTEXSUMMARYDECORATORALG_H
