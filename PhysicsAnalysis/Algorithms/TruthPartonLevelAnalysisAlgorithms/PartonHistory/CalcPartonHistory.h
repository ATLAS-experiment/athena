/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>
/// @author Steffen Korn <steffen.korn@cern.ch>

#ifndef PARTONS_CALCPARTONHISTORY_H
#define PARTONS_CALCPARTONHISTORY_H

// system include(s):
#include <memory>
#include <vector>

// Framework include(s):
#include <xAODEventInfo/EventInfo.h>

#include "AsgTools/AsgTool.h"
#include "AthContainers/DataVector.h"
#include "PartonHistory/PartonHistoryUtils.h"
#include "PartonHistory/PartonSchemeConfig.h"
#include "VectorHelpers/DecoratorHelpers.h"
#include "xAODTruth/TruthParticleContainer.h"

namespace CP {
using ROOT::Math::PtEtaPhiMVector;

class CalcPartonHistory : public asg::AsgTool {

 public:
  explicit CalcPartonHistory(
      const std::string& name,
      const std::vector<std::string>& truthCollections = {"TruthTop"});
  virtual ~CalcPartonHistory() {};

  CalcPartonHistory(const CalcPartonHistory& rhs) = delete;
  CalcPartonHistory(CalcPartonHistory&& rhs) = delete;
  CalcPartonHistory& operator=(const CalcPartonHistory& rhs) = delete;

  void AddToParticleMap(const xAOD::TruthParticle* particle,
                        const std::string& key);
  bool ExistsInMap(const std::string& key) const;
  bool ExistsInKey(const std::string& key,
                   const xAOD::TruthParticle* particle) const;
  bool Retrievep4(const std::string& key, PtEtaPhiMVector& p4);
  bool Retrievep4(const std::string& key, PtEtaPhiMVector& p4, const int& idx);
  bool Retrievep4Gamma(PtEtaPhiMVector& p4, int& parentpdgId);
  bool RetrievepdgId(const std::string& key, std::vector<int>& pdgIds);
  bool RetrievepdgId(const std::string& key, int& pdgId);
  bool RetrievepdgId(const std::string& key, int& pdgId, const int& idx);
  bool RetrieveParticleInfo(const std::string& prefix,
                            std::vector<const xAOD::TruthParticle*>& particles);
  bool RetrieveParticleInfo(const std::string& prefix,
                            PtEtaPhiMVector& particle, int& pdgId);
  bool RetrieveParticleInfo(const std::string& prefix,
                            PtEtaPhiMVector& particle, int& pdgId,
                            const int& idx);
  bool RetrieveParticleInfo(const std::string& prefix,
                            const std::string& alt_prefix,
                            PtEtaPhiMVector& particle, int& pdgId);
  bool RetrieveParticleInfo(const std::string& prefix,
                            std::vector<PtEtaPhiMVector>& particles,
                            std::vector<int>& pdgIds);

  void Initialize4TopDecorators();
  void InitializeTopDecorators();
  void InitializeAntiTopDecorators();
  void InitializeBottomDecorators();
  void InitializeVectorBottomDecorators();
  void InitializeAntiBottomDecorators();
  void InitializeVectorAntiBottomDecorators();
  void InitializeCharmDecorators();
  void InitializeVectorCharmDecorators();
  void InitializeAntiCharmDecorators();
  void InitializeVectorAntiCharmDecorators();
  void InitializeTtbarDecorators();
  void InitializeHiggsDecorators();
  void InitializePhotonDecorators();
  void InitializeZDecorators(int nZs = 1, bool extend = false);
  void InitializeWDecorators(int nWs = 1);

  std::string GetParticleType(const xAOD::TruthParticle* particle);

  void TraceParticle(
      const xAOD::TruthParticle* particle,
      std::vector<const xAOD::TruthParticle*>& currentPath,
      std::vector<std::vector<const xAOD::TruthParticle*>>& allPaths);
  void TraceParticles(const xAOD::TruthParticleContainer* truthParticles);

  bool handleFSR(const xAOD::TruthParticle* particle, const std::string& newKey,
                 std::string& key);
  bool handleDecay(const xAOD::TruthParticle* particle, std::string& key,
                   int decayID);
  void handleSameAsParent(const xAOD::TruthParticle* particle,
                          std::string& key);
  void handleDefault(const xAOD::TruthParticle* particle,
                     const std::string& newKey, std::string& key);

  void FillParticleMap(
      std::vector<std::vector<const xAOD::TruthParticle*>>& allPaths);

  // Generic history fillers
  void FillGenericPartonHistory(const std::string& retrievalstring,
                                const std::string& decorationstring,
                                const int idx);
  void FillGenericPartonHistory(
      const std::vector<std::string>& retrievalStrings,
      const std::string& decorationstring, const int idx);
  void FillGenericPartonHistory(
      const std::vector<std::string>& retrievalStrings,
      const std::string& decorationstring);
  void FillGenericVectorPartonHistory(const std::string& retrievalstring,
                                      const std::string& decorationstring);

  // Specialised history fillers
  void FillGammaPartonHistory(const std::string& parent);
  void FillZPartonHistory(const std::string& parent, int nZs = 1,
                          const std::string& mode = "resonant");
  void FillZtautauPartonHistory(const std::string& parent, int nZs = 1,
                                const std::string& mode = "resonant");
  void FillWPartonHistory(const std::string& parent, int nWs = 1,
                          const std::string& mode = "resonant");
  void FillTopPartonHistory();
  void FillAntiTopPartonHistory();
  void FillHiggsPartonHistory(const std::string& mode);
  void FillTtbarPartonHistory();

  // Helpers for non-resonant / off-shell reconstruction.
  void setHiggs(const std::string& fsr);
  void setW(const std::string& fsr, int nWs);
  bool getW(const std::string& str_lep, const std::string& str_nu,
            PtEtaPhiMVector& p1, int& pdgId1, PtEtaPhiMVector& p2, int& pdgId2);
  void setZ(const std::string& fsr, int nZs);
  void setZtautau(const std::string& fsr, int nZs);
  bool getZ(const std::string& str_lep1, const std::string& str_lep2,
            PtEtaPhiMVector& p1, int& pdgId1, PtEtaPhiMVector& p2, int& pdgId2);
  bool getZFromTaus(const std::string& fsr, PtEtaPhiMVector& Zdecay1,
                    int& Zdecay1_pdgId, PtEtaPhiMVector& Zdecay2,
                    int& Zdecay2_pdgId, PtEtaPhiMVector& Zdecay1_decay1,
                    int& Zdecay1_decay1_pdgId, PtEtaPhiMVector& Zdecay1_decay2,
                    int& Zdecay1_decay2_pdgId, PtEtaPhiMVector& Zdecay1_decay3,
                    int& Zdecay1_decay3_pdgId, PtEtaPhiMVector& Zdecay2_decay1,
                    int& Zdecay2_decay1_pdgId, PtEtaPhiMVector& Zdecay2_decay2,
                    int& Zdecay2_decay2_pdgId, PtEtaPhiMVector& Zdecay2_decay3,
                    int& Zdecay2_decay3_pdgId);

  /// Configure this instance with a scheme configuration.
  /// Must be called before initialize().
  void configure(const PartonSchemeConfig& config);

  virtual StatusCode initialize();
  virtual StatusCode execute();

 protected:
  std::map<std::string, std::vector<const xAOD::TruthParticle*>> m_particleMap;
  PartonDecorator m_dec;

  const std::vector<std::string> m_truthCollections;
  std::string
      m_prefix;  ///< prefix applied to all decorator and m_particleMap names

  // this method is used to perform the linking of various TRUTH3 particle
  // containers for Partons to work
  virtual StatusCode linkTruthContainers(
      const xAOD::TruthParticleContainer*& tp);

  // this is the method that runs the actual parton history reconstruction
  virtual StatusCode runHistorySaver(
      const xAOD::TruthParticleContainer* truthParticles,
      const xAOD::EventInfo* ttbarPartonHistory);

  virtual void initializeDecorators();

  PartonSchemeConfig m_config;  ///< scheme configuration set via configure()
  bool m_configured = false;    ///< true after configure() has been called

  /** used to build container from multiple collections
   *   in DAOD_PHYS we don't have the TruthParticles collection, so we have to
   * build a TruthParticleContainer (named out_contName) by merging several
   * collections; this is stored in the evtStore this method has to use some
   * tricks, like the helper m_tempParticles ConstDataVector, due to the desing
   * of DataVector, see
   * https://twiki.cern.ch/twiki/bin/view/AtlasComputing/DataVector
   */
  StatusCode buildContainerFromMultipleCollections(
      const std::vector<std::string>& collections,
      const std::string& out_contName);

  /** currently in DAOD_PHYS TruthTop have links to Ws from the TruthBoson
   * collection, which have no link to their decay products; we have therefore
   * to associate the W from the TruthBoson collections to those in the
   * TruthBosonsWithDecayParticles collection. This method will use the helper
   * method decorateCollectionWithLinksToAnotherCollection to decorate bosons in
   * the TruthBoson collection with "CustomLinkedTruthBosonWithDecayParticles",
   * which is a link to the same bosons in the TruthBosonsWithDecayParticles
   * collection
   */
  StatusCode linkBosonCollections();

  /// helper method to handle retriveing the truth particle linked in the
  /// decoration of another particle
  const xAOD::TruthParticle* getTruthParticleLinkedFromDecoration(
      const xAOD::TruthParticle* part, const std::string& decorationName);

 private:
  /**helper method currently used in DAOD_PHYS to link particles from a given
   * collection to the same particles included in another collection; needed
   * because particles may be duplicated in different collections, but their
   * navigation links may only be there in some of them...
   */
  StatusCode decorateCollectionWithLinksToAnotherCollection(
      const std::string& collectionToDecorate,
      const std::string& collectionToLink, const std::string& nameOfDecoration);
};

}  // namespace CP

#endif
