/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef RPC_DIGITIZATIONR4_RPCFASTDIGITOOL_H
#define RPC_DIGITIZATIONR4_RPCFASTDIGITOOL_H


#include "MuonDigitizationR4/MuonDigitizationTool.h"
#include "MuonDigitContainer/RpcDigitContainer.h"
#include "MuonCondData/DigitEffiData.h"
#include "MuonReadoutGeometryR4/RpcReadoutElement.h"

#include "GaudiKernel/PhysicalConstants.h"

namespace MuonR4{
    class RpcFastDigiTool final: public MuonDigitizationTool {
        public:
            RpcFastDigiTool(const std::string& type, const std::string& name, const IInterface* pIID);

            StatusCode initialize() override final;
            StatusCode finalize() override final;
        protected:
            StatusCode digitize(const EventContext& ctx,
                                const TimedHits& hitsToDigit,
                                xAOD::MuonSimHitContainer* sdoContainer) const override final; 
        
 
        private:
            using EdgeSide = MuonGMR4::RpcReadoutElement::EdgeSide;
            int m_stIdxBIL{-1}; // Station name index of the BIL stations
            
            /**  @brief Digitize the sim hit as Rpc strip 1D hit.  
             *   @param gasGapId: Identifier of the associated gasGap
             *   @param measuresPhi: Process a phi or an eta strip
             *   @param designPtr: Pointer to the striplayout of the gasGap
             *   @param hitTime: Global time of the hit
             *   @param locPosOnStrip: Local position of the hit perpendicular to the strip plane
             *   @param effiMap: Pointer to an efficiency look-up table
             *   @param outContainer: DigitCollection to push the new digit into
             *   @param rndEngine:  Random engine used for smearing
             *   @param deadTimes: Reference to the last digitized times in order to apply the dead time model */
            bool digitizeHit(const Identifier& gasGapId,
                             const bool measuresPhi,
                             const MuonGMR4::RpcReadoutElement& reEle,
                             const double hitTime,
                             const Amg::Vector2D& locPos,
                             const Muon::DigitEffiData* effiMap,
                             RpcDigitCollection& outContainer,
                             CLHEP::HepRandomEngine* rndEngine,
                             DeadTimeMap& deadTimes) const;
            
            /**  @brief Digitize the sim hit as Rpc strip 2D hit.  
             *   @param gasGapId: Identifier of the associated gasGap
             *   @param designPtr: Pointer to the striplayout of the gasGap
             *   @param hitTime: Global time of the hit
             *   @param locPos: Local position of the hit inside the strip plane
             *   @param effiMap: Pointer to an efficiency look-up table
             *   @param outContainer: DigitCollection to push the new digit into
             *   @param rndEngine:  Random engine used for smearing
             *   @param deadTimes: Reference to the last digitized times in order to apply the dead time model */
            bool digitizeHitBI(const Identifier& gasGapId,
                               const MuonGMR4::RpcReadoutElement& reEle,
                               const double hitTime,
                               const Amg::Vector2D& locPos,
                               const Muon::DigitEffiData* effiMap,
                               RpcDigitCollection& outContainer,
                               CLHEP::HepRandomEngine* rndEngine,
                               DeadTimeMap& deadTimes) const;

            /** @brief Roll the time over threshold for each signal digit */
            static double timeOverThreshold(CLHEP::HepRandomEngine* rndmEngine) ;

            using DigiCache = OutDigitCache_t<RpcDigitCollection>;
            SG::WriteHandleKey<RpcDigitContainer> m_writeKey{this, "OutputObjectName", "RPC_DIGITS"};

            SG::ReadCondHandleKey<Muon::DigitEffiData> m_effiDataKey{this, "EffiDataKey", "RpcDigitEff",
                                                                    "Efficiency constants of the individual Rpc gasGaps"};

            mutable std::array<std::atomic<unsigned>, 2> m_allHits ATLAS_THREAD_SAFE{};
            mutable std::array<std::atomic<unsigned>, 2> m_acceptedHits ATLAS_THREAD_SAFE{};

            Gaudi::Property<double> m_propagationVelocity{this, "propSpeed", 0.5 * Gaudi::Units::c_light,
                                                         "Propagation speed of the signal inside the strip"}; // in mm/ns
    
            Gaudi::Property<double> m_stripTimeResolution{this, "timeResolution", 0.6 * Gaudi::Units::nanosecond,
                                                          "Estimated time resolution of the strip readout"};

            Gaudi::Property<double> m_deadTime{this, "deadTime", 100.*Gaudi::Units::nanosecond};

            Gaudi::Property<bool> m_digitizeMuonOnly{this, "ProcessTrueMuonsOnly", false, 
                                                     "If set to true hit with pdgId != 13 are skipped"};

    };
}
#endif