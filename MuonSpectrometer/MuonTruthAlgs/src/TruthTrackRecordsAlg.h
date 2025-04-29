/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

  * @brief This alg receives the container of truth muons and decorates them 
  *        with track records (snapshot) of the same in different positions
  *        within the detector. Decorations (for each detector position)
  *        include: muon records in that layer; a flag saying if a record has
  *        been recorded in that layer; muon extrapolation from the 
  *        previous layer; extrapolation's covariance matrix; and a flag 
  *        representing the exrapolation outcome.
*/
#ifndef MUONTRUTHALGS_TruthTrackRecordsAlg_H
#define MUONTRUTHALGS_TruthTrackRecordsAlg_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "MuonTruthAlgs/DecorUtils.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "TrackRecord/TrackRecordCollection.h"

namespace Muon {

    class TruthTrackRecordsAlg : public AthReentrantAlgorithm {
    public:

        // Constructor with parameters:
        using AthReentrantAlgorithm::AthReentrantAlgorithm;

        // Basic algorithm methods:
        virtual StatusCode initialize() override;
        virtual StatusCode execute(const EventContext& ctx) const override;

    private:
        /** @brief Struct object encapsulating all write-decorators. Each decorArray represents a property, and its
        *          elements represents that property evaluated at different detector positions. 
        *          List of properties: x,y,x,px,py,pz are position and momentum in a given layer; matchedDecor is a 
        *          flag saying if a record has been recorded in that layer; ex,ey,ez,epx,epy,epz are extrapolated position
        *          and momentum; ecov is a vector encapsulating the covariance matrix from extrapolation; eis is a flag 
        *          representing if the extrapolation has been successful.
        *          We can decouple declaration and population of this object because vectors can be initialized empty.        
        */
        using WriteDecorArray_f = DecorHandlePtrVec_t<xAOD::TruthParticleContainer, float>;
        using WriteDecorArray_b = DecorHandlePtrVec_t<xAOD::TruthParticleContainer, char>;
        using WriteDecorArray_fvec = DecorHandlePtrVec_t<xAOD::TruthParticleContainer, std::vector<float>>;
        
        struct SummaryDecors{
            WriteDecorArray_f xDecor{};
            WriteDecorArray_f yDecor{};
            WriteDecorArray_f zDecor{};
            WriteDecorArray_f pxDecor {};
            WriteDecorArray_f pyDecor {};
            WriteDecorArray_f pzDecor {};
            WriteDecorArray_b matchedDecor {};
            WriteDecorArray_f exDecor {};
            WriteDecorArray_f eyDecor {};
            WriteDecorArray_f ezDecor {};
            WriteDecorArray_f epxDecor {};
            WriteDecorArray_f epyDecor {};
            WriteDecorArray_f epzDecor {};
            WriteDecorArray_fvec ecovDecor {};
            WriteDecorArray_b eisDecor {};
        };
        /** @brief addTrackRecords is the actual function that decorates muons with track records. Firstly, if production vertex is available,
        *          we save position and momentum at production. Then we loop over snapshot positions (e.g. calo entry level) and, in 
        *          each position, we loop over track records. If the record is descendand of the muon, we save position and momentum in 
        *          that position, as well as the tracking volume and record name (= snapshot position). Then, by means of another loop 
        *          over all records of a truth muon, we extrapolate each record to the next shapshot position. Specifically, 
        *          we save the extrapolated position and momentum, the extrapolation's covariance matrix, and a flag representing the
        *          extrapolation outcome.
        ** @param  ctx: tells in which event we are
        ** @param  truthParticle: a pointer to the truth muon
        ** @param  myDecors: reference to the decorator summary object
        */
        StatusCode addTrackRecords(const EventContext& ctx, 
                                   const xAOD::TruthParticle& truthParticle,
                                   SummaryDecors& myDecors) const;
        
        /** @brief This function fills the arrays of write-decorator keys and initializes them. Specifically, the key that will be written
        *          at i-th position is <property_name>_<detector_position(i)>, where property_name = writeKey, and detector_position(i) is 
        *          retrieved from (= is an element of) m_trackRecords
        **  @param writeKey: reference to the (empty) array of write-decor keys that is going to be populated
        **  @param keyName: reference to the property name (e.g. x, x_extr, ...)
        */
        template <typename T> 
        StatusCode fillWriteDecorator(SG::WriteDecorHandleKeyArray<xAOD::TruthParticleContainer, T>& writeKey, const std::string& keyName) const;

        /** @brief Key of the container of truth muons that will be decorated
        */
        SG::ReadHandleKey<xAOD::TruthParticleContainer> m_muonTruth{this, "muonTruth", "MuonTruthParticles"};

        /** @brief Keys of the containers of track records in different detector positions 
        */
        SG::ReadHandleKeyArray<TrackRecordCollection> m_trackRecords{
            this, "TrackRecordCollectionNames", {"CaloEntryLayer", "MuonEntryLayer", "MuonExitLayer"}};
        
        /** @brief Declaration of the key arrays, which are all initialized as empty (they will be filled up by @fillWriteDecorator).
        *          Each decorKeyArray, which is an array of keys, represents a property, and its elements represents that property 
        *          evaluated at different detector positions. List of properties: x,y,x,px,py,pz are position and momentum in a given 
        *          layer; matchedDecor is a flag saying if a record has been recorded in that layer; ex,ey,ez,epx,epy,epz are extrapolated
        *          position and momentum; ecov is a vector encapsulating the covariance matrix from extrapolation; eis is a flag representing 
        *          if the extrapolation has been successful.    
        */
        using DecorKey_t = SG::WriteDecorHandleKeyArray<xAOD::TruthParticleContainer>;
        DecorKey_t m_muonSnapShotX{this, "SnapShotX", { }, "Position decoration..."};
        DecorKey_t m_muonSnapShotY{this, "SnapShotY", { }, "Position decoration..."};
        DecorKey_t m_muonSnapShotZ{this, "SnapShotZ", { }, "Position decoration..."};
        DecorKey_t m_muonSnapShotPx{this, "SnapShotPx", { }, "Momentum decoration..."};
        DecorKey_t m_muonSnapShotPy{this, "SnapShotPy", { }, "Momentum decoration..."};
        DecorKey_t m_muonSnapShotPz{this, "SnapShotPz", { }, "Momentum decoration..."};
        DecorKey_t m_muonSnapShotMtc{this, "SnapShotMtc", { }, "is matched decoration..."};
        DecorKey_t m_muonSnapShotEX{this, "SnapShotEX", { }, "Position decoration..."};
        DecorKey_t m_muonSnapShotEY{this, "SnapShotEY", { }, "Position decoration..."};
        DecorKey_t m_muonSnapShotEZ{this, "SnapShotEZ", { }, "Position decoration..."};
        DecorKey_t m_muonSnapShotEPx{this, "SnapShotEPx", { }, "Momentum decoration..."};
        DecorKey_t m_muonSnapShotEPy{this, "SnapShotEPy", { }, "Momentum decoration..."};
        DecorKey_t m_muonSnapShotEPz{this, "SnapShotEPz", { }, "Momentum decoration..."};
        DecorKey_t m_muonSnapShotEcov{this, "SnapShotEcov", { }, "Covariance decoration"};
        DecorKey_t m_muonSnapShotEis{this, "SnapShotEis", { }, "Flag decoration whether the extrapolation worked"};

        /** @brief  Extrapolation tool handle */
        ToolHandle<Trk::IExtrapolator> m_extrapolator{this, "Extrapolator", "Trk::Extrapolator/AtlasExtrapolator"};

    };

}  // namespace Muon
#endif