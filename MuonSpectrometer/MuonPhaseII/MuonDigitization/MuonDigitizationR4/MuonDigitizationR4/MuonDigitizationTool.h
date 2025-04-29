/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONDIGITIZATIONR4_MUONDIGITIZATIONTOOL_H
#define MUONDIGITIZATIONR4_MUONDIGITIZATIONTOOL_H

#include "PileUpTools/PileUpMergeSvc.h"
#include "PileUpTools/PileUpToolBase.h"


#include "CLHEP/Random/RandomEngine.h"
#include "AthenaKernel/IAthRNGSvc.h"


#include "xAODMuonSimHit/MuonSimHitContainer.h"
#include "xAODMuonSimHit/MuonSimHitAuxContainer.h"

#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "HitManagement/TimedHitPtr.h"


namespace std{
    template<> class remove_pointer<TimedHitPtr<xAOD::MuonSimHit>>{
        public:
          using type = xAOD::MuonSimHit;
    };
      template<> class remove_pointer<TimedHitPtr<const xAOD::MuonSimHit>>{
        public:
          using type = xAOD::MuonSimHit;
    };
}

namespace MuonR4 {
     /** @brief Barebone implementation of the I/O infrastructure for all MuonDigitizationTools.
      *         
     */
     class MuonDigitizationTool: public PileUpToolBase {
        public:
            using PileUpToolBase::PileUpToolBase;

            StatusCode initialize() override;


            StatusCode processBunchXing(int bunchXing, 
                                        SubEventIterator bSubEvents, 
                                        SubEventIterator eSubEvents) override final;

            StatusCode mergeEvent(const EventContext& ctx) override final;

            /** When being run from PileUpToolsAlgs, this method is called at the start of the subevts loop. Not able to access SubEvents */
            StatusCode prepareEvent(const EventContext& ctx, 
                                    const unsigned int /*nInputEvents*/) override final;

  
            /** alternative interface which uses the PileUpMergeSvc to obtain
            all the required SubEvents. */
            StatusCode processAllSubEvents(const EventContext& ctx) override final;
            /** Reentrant version of the digitization tool */
            StatusCode processAllSubEvents(const EventContext& ctx) const;
        
        protected:
            CLHEP::HepRandomEngine* getRandomEngine(const EventContext&ctx) const;
            
            using TimedHit = TimedHitPtr<xAOD::MuonSimHit>;
            using TimedHits = std::vector<TimedHitPtr<xAOD::MuonSimHit>>;

            /** @brief Digitize the time ordered hits and write them to the digit format specific for the 
             *         detector technology. A new MuonSimHitContainer pointer is parsed to also create the MuonSDO. 
             *         If a new SDO should be added to the container plese use the addSDO() method as defined below.         
            */
            virtual StatusCode digitize(const EventContext& ctx,
                                        const TimedHits& hitsToDigit,
                                        xAOD::MuonSimHitContainer* sdoContainer) const = 0;
            
            /** @brief Adds the timed simHit to the output SDO container. The hit may be rejected if it's originating from
             *         pile-up and the pile-up truth skimming strategy is applied
             */
            xAOD::MuonSimHit* addSDO(const TimedHit& hit, xAOD::MuonSimHitContainer* sdoContainer) const;

            /** @brief Returns the reference to the ActsGeometryContext needed to fetch global positions from the Readout geometry*/
            const ActsGeometryContext& getGeoCtx(const EventContext& ctx) const;

            /** @brief DigitContainers are sorted by DigitCollections which are the ensemble of all hits in a given
             *         MuonChamber. To fill the final DigitContainer thread-safely, the DigitCollections shall be cached
             *         pre cached in a OutDigitCache_t vector which is later moved to the final DigitContainer
            */
            template <class DetType> using OutDigitCache_t = std::vector<std::unique_ptr<DetType>>;
            
            /** @brief Helper function that provides fetches the proper DigitCollection from the DigitCache for a given hit identifier
             *         If the Collection is fetched for the first time, it's inserted into the cache first.
            */
            template <class DigitColl> 
                      DigitColl* fetchCollection(const Identifier& hitId,
                                                 OutDigitCache_t<DigitColl>& digitCache) const;
            /** @brief Helper function to move the collected digits into the final DigitContainer. The function needs the maximal
             *         size of the container in advance which is provided by calling the module_hash_max() function of the corresponding
             *         MuonIdHelper.
            */
            template <class DigitCont, class DigitColl> 
                    StatusCode writeDigitContainer(const EventContext& ctx,
                                                   const SG::WriteHandleKey<DigitCont>& key,
                                                   OutDigitCache_t<DigitColl>&& digitCache,
                                                   unsigned int hashMax) const;

            /** @brief Returns the global time of the hit which is the sum of eventTime & individual hit time */
            static double hitTime(const TimedHit& hit);
            using DeadTimeMap = std::unordered_map<Identifier, double>;
            /** @brief Returns whether the new digit is within the dead time window
             *  @brief channelId: Identifier of the fired channel
             *  @brief hitTime: Current hit time
             *  @brief deadTimeWindow: Dead time to pass before the next hit may be accepted
             *  @brief deadTimeMap: Map storing the last hit times from each Identifier */
            static bool passDeadTime(const Identifier& channelId,
                                     const double hitTime, 
                                     const double deadTimeWindow,
                                     DeadTimeMap& deadTimeMap);

            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};

            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", 
                                                                "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
        private:
            using PileUpHits = PileUpMergeSvc::TimedList<xAOD::MuonSimHitContainer>::type;
            
            /** Translates the PileUpHits into the timed hits format */
            StatusCode fillTimedHits(PileUpHits&& hitColl, TimedHits& timedHits) const;
            
            SG::ReadHandleKey<xAOD::MuonSimHitContainer> m_simHitKey{this, "SimHitKey", ""};

            SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", 
                                                               "Geometry context"};

            ServiceHandle<PileUpMergeSvc> m_mergeSvc{this, "PileUpMergeSvc", "PileUpMergeSvc", ""};

            Gaudi::Property<std::string> m_streamName{this, "StreamName", ""};

            ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc", ""};  // Random number service

            SG::WriteHandleKey<xAOD::MuonSimHitContainer> m_sdoKey{this, "OutputSDOName", ""};

            Gaudi::Property<bool> m_onlyUseContainerName{this, "OnlyUseContainerName", false,
                                                         "Don't use the ReadHandleKey directly. Just extract the container name from it."};

            Gaudi::Property<bool> m_includePileUpTruth{this, "IncludePileUpTruth", true, "Include pile-up truth info"};

            std::string m_inputObjectName{""};

            TimedHits m_timedHits{};
            /// Create a local copy of the sim hits to ensure overlayed hits across the events remain valid
            using SimHitLocalCopy = std::pair<std::unique_ptr<xAOD::MuonSimHitContainer>,
                                              std::unique_ptr<xAOD::MuonSimHitAuxContainer>>;
            std::vector<SimHitLocalCopy> m_simHits{};


};

}
#include "MuonDigitizationR4/MuonDigitizationTool.icc"
#endif

