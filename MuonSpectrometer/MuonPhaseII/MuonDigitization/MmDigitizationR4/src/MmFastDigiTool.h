/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MM_DIGITIZATIONR4_MMFASTDIGITOOL_H
#define MM_DIGITIZATIONR4_MMFASTDIGITOOL_H


#include "MuonDigitizationR4/MuonDigitizationTool.h"
#include "MuonDigitContainer/MmDigitContainer.h"

#include "MuonCondData/DigitEffiData.h"
#include "MuonCondData/NswErrorCalibData.h"

namespace MuonR4{
    /***
     *  @brief: Smearing of hits stemming from muons using the best knowledge uncertainties
     */
    class MmFastDigiTool final: public MuonDigitizationTool {
        public:
            MmFastDigiTool(const std::string& type, const std::string& name, const IInterface* pIID);

            StatusCode initialize() override final;
            StatusCode finalize() override final;
        protected:
            StatusCode digitize(const EventContext& ctx,
                                const TimedHits& hitsToDigit,
                                xAOD::MuonSimHitContainer* sdoContainer) const override final; 
        
 
        private: 
            using DigiCache = OutDigitCache_t<MmDigitCollection>;
            SG::WriteHandleKey<MmDigitContainer> m_writeKey{this, "OutputObjectName", "MM_DIGITS"};

            SG::ReadCondHandleKey<Muon::DigitEffiData> m_effiDataKey{this, "EffiDataKey", "MmDigitEff",
                                                                    "Efficiency constants of the individual Rpc gasGaps"};
            
            SG::ReadCondHandleKey<NswErrorCalibData> m_uncertCalibKey{this, "ErrorCalibKey", "NswUncertData",
                                                                     "Key of the parametrized NSW uncertainties"};

            
            mutable std::array<std::atomic<unsigned>, 8> m_allHits ATLAS_THREAD_SAFE{};
            mutable std::array<std::atomic<unsigned>, 8> m_acceptedHits ATLAS_THREAD_SAFE{};

            // Dead time between two hits in the same channel
            Gaudi::Property<double> m_deadTime{this, "deadTime", 300. * Gaudi::Units::ns};

            Gaudi::Property<bool> m_digitizeMuonOnly{this, "ProcessTrueMuonsOnly", false, 
                                                     "If set to true hit with pdgId != 13 are skipped"};
            Gaudi::Property<double> m_energyThreshold{this, "EnergyThreshold", 50, "Minimal energy to process an electron sim hit"};
    };
}
#endif