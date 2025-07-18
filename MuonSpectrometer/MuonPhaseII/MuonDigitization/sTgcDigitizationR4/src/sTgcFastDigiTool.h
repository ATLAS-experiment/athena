/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef STGC_DIGITIZATIONR4_STGCFASTDIGITOOL_H
#define STGC_DIGITIZATIONR4_STGCFASTDIGITOOL_H


#include "MuonDigitizationR4/MuonDigitizationTool.h"
#include "MuonDigitContainer/sTgcDigitContainer.h"
#include "MuonCondData/DigitEffiData.h"
#include "MuonCondData/NswErrorCalibData.h"

namespace MuonR4{
    class sTgcFastDigiTool final: public MuonDigitizationTool {
        public:
            sTgcFastDigiTool(const std::string& type, const std::string& name, const IInterface* pIID);

            StatusCode initialize() override final;
            StatusCode finalize() override final;
        protected:
            StatusCode digitize(const EventContext& ctx,
                                const TimedHits& hitsToDigit,
                                xAOD::MuonSimHitContainer* sdoContainer) const override final; 
        private:            
            
            bool digitizeStrip(const EventContext& ctx,
                               const TimedHit& timedHit,
                               const NswErrorCalibData* errorCalibDB,
                               const Muon::DigitEffiData* effiData,
                               CLHEP::HepRandomEngine* rndEngine,
                               sTgcDigitCollection& outCollection) const;
            
            bool digitizeWire(const EventContext& ctx,
                              const TimedHit& timedHit,
                              const Muon::DigitEffiData* effiData,
                              CLHEP::HepRandomEngine* rndEngine,
                              sTgcDigitCollection& outCollection) const;

            bool digitizePad(const EventContext& ctx,
                             const TimedHit& timedHit,
                             const Muon::DigitEffiData* effiData,
                             CLHEP::HepRandomEngine* rndEngine,
                             sTgcDigitCollection& outCollection) const;

            /** @brief: Associates the global bcIdTag to the digit
             *  @param ctx: EventContext
             *  @param timedHit: Hit to calculate the time from
             */
            int associateBCIdTag(const EventContext& ctx,
                                 const TimedHit& timedHit) const;



            using DigiCache = OutDigitCache_t<sTgcDigitCollection>;
            SG::WriteHandleKey<sTgcDigitContainer> m_writeKey{this, "OutputObjectName", "sTGC_DIGITS"};

            SG::ReadCondHandleKey<Muon::DigitEffiData> m_effiDataKey{this, "EffiDataKey", "sTgcDigitEff",
                                                                    "Efficiency constants of the individual sTgc gasGaps"};


            SG::ReadCondHandleKey<NswErrorCalibData> m_uncertCalibKey{this, "ErrorCalibKey", "NswUncertData",
                                                                      "Key of the parametrized NSW uncertainties"};

            mutable std::array<std::atomic<unsigned>, 3> m_allHits ATLAS_THREAD_SAFE{};
            mutable std::array<std::atomic<unsigned>, 3> m_acceptedHits ATLAS_THREAD_SAFE{};

            Gaudi::Property<bool> m_digitizeStrip{this, "doStrips", true, "Digitize strip hits"};
            Gaudi::Property<bool> m_digitizeWire{this, "doWires", true, "Digitize wire hits"};
            Gaudi::Property<bool> m_digitizePads{this, "doPads", false, "Digitize pad hits"};

            Gaudi::Property<double> m_deadTime{this, "deadTime", 300. * Gaudi::Units::ns};

            Gaudi::Property<bool> m_digitizeMuonOnly{this, "ProcessTrueMuonsOnly", false, 
                                                     "If set to true hit with pdgId != 13 are skipped"};
            Gaudi::Property<double> m_energyDepositThreshold{this,"energyDepositThreshold",300.0*CLHEP::eV};
            Gaudi::Property<double> m_limitElectronKineticEnergy{this,"limitElectronKineticEnergy",5.0*CLHEP::MeV};

    };
}
#endif