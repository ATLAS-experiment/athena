/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
  Contact: Raphael Haberle <raphael.julien.haberle@cern.ch>
*/

#ifndef DERIVATIONFRAMEWORKEGAMMA_EGammaGSFCalo_H
#define DERIVATIONFRAMEWORKEGAMMA_EGammaGSFCalo_H

#include <atomic>

#include "AthenaBaseComps/AthAlgTool.h"

#include "DerivationFrameworkInterfaces/IAugmentationTool.h"

#include "TrkToolInterfaces/IExtendedTrackSummaryTool.h"
#include "TrkToolInterfaces/ITrackParticleCreatorTool.h"

#include "egammaInterfaces/IegammaTrkRefitterTool.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODEgamma/ElectronContainerFwd.h"

#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

using xAOD::EgammaHelpers::summaryValueInt;


namespace DerivationFramework {
class EGammaGSFCalo : public AthAlgTool, public IAugmentationTool {
 public:
  /** @brief Default constructor **/
  EGammaGSFCalo(const std::string& t, const std::string& n,
                const IInterface* p);
  /** @brief Destructor **/
  virtual ~EGammaGSFCalo();
  /** @brief initialize method **/
  virtual StatusCode initialize() override;
  /** @brief finalize method **/
  virtual StatusCode finalize() override;
  /** @brief addBranches method **/
  virtual StatusCode addBranches( const EventContext& ctx ) const override;

 private:
  SG::ReadHandleKey<xAOD::ElectronContainer> m_electronCollectionKey{
      this, "ElectronCollectionName", "Electrons", "Input electron collection"};

  SG::WriteHandleKey<xAOD::TrackParticleContainer> m_OutputTrkPartContainerKey{
      this, "OutputTrkPartContainerName", "GSFCaloContainer",
      "Output GSF+Calo container"};

  SG::WriteDecorHandleKey<xAOD::ElectronContainer> m_gsfCaloTrackLinkKey{
      this, "GSFCaloTrackLinkKey", "Electrons.gsfCaloTrackParticleLink",
      "Decoration linking electrons to refitted TrackParticles"};
  /** @brief The track refitter */
  ToolHandle<IegammaTrkRefitterTool> m_trkRefitTool{
      this, "TrackRefitTool", "ElectronRefitterTool", "Track refitter tool"};

  /** @brief Tool to create track particle */
  ToolHandle<Trk::ITrackParticleCreatorTool> m_particleCreatorTool{
      this, "TrackParticleCreatorTool", "TrackParticleCreatorTool",
      "TrackParticle creator tool"};

  PublicToolHandle<Trk::IExtendedTrackSummaryTool> m_trackSummaryTool{
      this, "TrackSummaryTool", "Trk::TrackSummaryTool/AtlasTrackSummaryTool"};

  /** @brief Minimum number of silicon hits on track before it is allowed to be
   * refitted **/
  Gaudi::Property<int> m_minNSiHits{this, "minNSiHits", 4,
                                    "Minimum number of silicon hits on track "
                                    "before it is allowed to be refitted"};
  Gaudi::Property<bool> m_doPix{this, "usePixel", true,
                                "Bool to use pixel hits"};
  Gaudi::Property<bool> m_doSCT{this, "useSCT", true, "Bool to use SCT hits"};
  Gaudi::Property<bool> m_doTRT{this, "useTRT", true, "Bool to use TRT hits"};

  Gaudi::Property<bool> m_doTruth{this, "useTruth", true,
                                  "Bool to add truth decorations"};

  Gaudi::Property<bool> m_isAOD{this, "isAOD", true, "AOD flag"};

  /** Counters **/
  mutable std::atomic<unsigned int> m_allElectrons = 0;
  mutable std::atomic<unsigned int> m_noTP = 0;
  mutable std::atomic<unsigned int> m_onlyTRT = 0;
  mutable std::atomic<unsigned int> m_noTrk = 0;
  mutable std::atomic<unsigned int> m_failedFits = 0;
  mutable std::atomic<unsigned int> m_successfulFits = 0;
  mutable std::atomic<unsigned int> m_successfulCopyInfos = 0;
  mutable std::atomic<unsigned int> m_noRefTP = 0;
  mutable std::atomic<unsigned int> m_allNewTP = 0;
  mutable std::atomic<unsigned int> m_tsos = 0;

  /** @brief Copy TrackParticle info from the original TP **/
  void copyInfo(const xAOD::TrackParticle& original,
                xAOD::TrackParticle& created, bool isRefitted) const;

  void copySummaryValue(const xAOD::TrackParticle& src,
                        xAOD::TrackParticle& dest,
                        const xAOD::SummaryType& information) const {
    uint8_t value = summaryValueInt(src, information, 0);
    dest.setSummaryValue(value, information);
  }
};
}  // namespace DerivationFramework

#endif  // DERIVATIONFRAMEWORKEGAMMA_EGammaGSFCalo_H
