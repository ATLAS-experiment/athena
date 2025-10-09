/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TGC_DIGITIZATIONR4_TGCFASTDIGITOOL_H
#define TGC_DIGITIZATIONR4_TGCFASTDIGITOOL_H


#include "MuonDigitizationR4/MuonDigitizationTool.h"
#include "MuonDigitContainer/TgcDigitContainer.h"
#include "MuonCondData/DigitEffiData.h"

namespace MuonR4{
    class TgcFastDigiTool final: public MuonDigitizationTool {
        public:
            using MuonDigitizationTool::MuonDigitizationTool;

            StatusCode initialize() override final;
            StatusCode finalize() override final;
        protected:
            StatusCode digitize(const EventContext& ctx,
                                const TimedHits& hitsToDigit,
                                xAOD::MuonSimHitContainer* sdoContainer) const override final; 
        private:
            /** @brief Digitize the wire hit by smearing the truth hit position according to the
             *         wire group pitch and then assigning the Identifier to it
             *         If an efficiency conditions object is scheduled, hits may additionally rejected
             *         based on simple random number drawing
             *  @param ctx: Context of the current event to access the digitEffi store
             *  @param timedHit: Truth hit to digitize
             *  @param efficiencyMap: Pointer to the gasGap efficency look-up table
             *  @param outColl: Digit collection to push the final digit into
             *  @param rndEngine: Random engine used for smearing & efficiency evaluation
             *  @param deadTimes: Reference to the last digitized times in order to apply the dead time model */
            bool digitizeWireHit(const EventContext& ctx,
                                 const TimedHit& timedHit,
                                 const Muon::DigitEffiData* efficiencyMap,
                                 TgcDigitCollection& outColl,
                                 CLHEP::HepRandomEngine* rndEngine,
                                 DeadTimeMap& deadTimes) const;
            
            /** @brief Digitize the strip hit by smearing the truth hit position according to the
             *         wire group pitch and then assigning the Identifier to it.
             *         If an efficiency conditions object is scheduled, hits may additionally rejected
             *         based on simple random number drawing.
             *  @param ctx: Context of the current event to access the digitEffi store
             *  @param timedHit: Truth hit to digitize
             *  @param efficiencyMap: Pointer to the gasGap efficency look-up table
             *  @param outColl: Digit collection to push the final digit into
             *  @param rndEngine: Random engine used for smearing & efficiency evaluation
             *  @param deadTimes: Reference to the last digitized times in order to apply the dead time model */
            bool digitizeStripHit(const EventContext& ctx,
                                  const TimedHit& timedHit,
                                  const Muon::DigitEffiData* efficiencyMap,
                                  TgcDigitCollection& outColl,
                                  CLHEP::HepRandomEngine* rndEngine,
                                  DeadTimeMap& deadTimes) const;
            
            /** @brief: Associates the global bcIdTag to the digit
             *  @param ctx: EventContext
             *  @param timedHit: Hit to calculate the time from
             */
            int associateBCIdTag(const EventContext& ctx,
                                 const TimedHit& timedHit) const;

            using DigiCache = OutDigitCache_t<TgcDigitCollection>;

            SG::WriteHandleKey<TgcDigitContainer> m_writeKey{this, "OutputObjectName", "TGC_DIGITS"};

            SG::ReadCondHandleKey<Muon::DigitEffiData> m_effiDataKey{this, "EffiDataKey", "TgcDigitEff",
                                                                    "Efficiency constants of the individual Rpc gasGaps"};

            mutable std::array<std::atomic<unsigned>, 2> m_allHits ATLAS_THREAD_SAFE{};
            mutable std::array<std::atomic<unsigned>, 2> m_acceptedHits ATLAS_THREAD_SAFE{};

            Gaudi::Property<double> m_deadTime{this, "deadTime", 100.*Gaudi::Units::nanosecond};

            Gaudi::Property<bool> m_digitizeMuonOnly{this, "ProcessTrueMuonsOnly", false, 
                                                     "If set to true hit with pdgId != 13 are skipped"};


    };
}
#endif