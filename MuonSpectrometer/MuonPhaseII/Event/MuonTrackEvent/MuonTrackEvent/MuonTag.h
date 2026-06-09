/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKEVENT_MUONTAG_H
#define MUONTRACKEVENT_MUONTAG_H


#include "xAODMuon/Muon.h"
#include "MuonTrackEvent/HitSummary.h"
#include "AthContainers/DataVector.h"

#include <span>
#include <optional>
#include <variant>
#include <unordered_map>
#include <memory>

namespace MuonR4 {
    /** @brief Baseline EDM object to gather all relevant information about a reconstructed
     *         muon candidate which can be turned into an analysis object */
    class MuonTag{

    public:
        /** @brief  Default constructor*/
        MuonTag() = default;
        /** @brief Setup the muon authors */
        using Author = xAOD::Muon::Author;
        /** @brief Recylce the Parameter definition enum from the Muon */
        using ParamDef = xAOD::Muon::ParamDef;
        /** @brief Extra parameters are either floats or ints  */
        using ParamData_t = std::variant<float, int>;
        /** @brief Returns the segments associated with the tag */
        const std::vector<const xAOD::MuonSegment*>& segments() const;
        /** @brief Returns the id track candidate from which the tag was built */
        const xAOD::TrackParticle* idTrack() const;
        /** @brief Returns the combined track candidate from which the tag was built */
        const xAOD::TrackParticle* cbTrack() const;
        /** @brief Returns the ms track candidate from which the tag was built  */
        const xAOD::TrackParticle* msTrack() const;
        /** @brief Return the primary track particle */
        const xAOD::TrackParticle* primaryTrack() const;
        /** @brief Copy the parameters from the Muon tag to the output xAOD muon */
        void copyParameters(xAOD::Muon& muon) const;
        /** @brief Returns the object's author */
        Author author() const;
        
        /** @brief Retrieve an additional parameter. Template specification
         *         can either be the ParamData_t or the types held by the 
         *         variant. If the parameter is not stored or the data type
         *         does not match a nullopt is returned
         * @param par: The parameter to retrieve */
        template <typename T>
        std::optional<T> parameter(const ParamDef par) const;

        /** @brief Sets the inner detector track particle
         *  @param idTrack: Pointer to the ID track particle */
        void setIdTrack(const xAOD::TrackParticle* idTrack);
        /** @brief Sets the combined track particle
         *  @param cbTrack: Pointer to the combined track particle */
        void setCbTrack(const xAOD::TrackParticle* cbTrack);
        /** @brief Sets the ms track particle 
         *  @param msTrack Pointer to the MS track particle */
        void setMsTrack(const xAOD::TrackParticle* msTrack);
        /** @brief Sets the segments associated with this tag */
        void setSegments(const std::span<const xAOD::MuonSegment* const> segs);
        /** @brief Set a parameter to be decorated to the final muon
         *  @param par: Enum encoding which quantity is represented by the data
         *  @param data: The actual data held */
        void setParameter(const ParamDef par, ParamData_t data);
        /** @brief Set the muon's author */
        void setAuthor(const Author author);
        /** @brief Set the muon track summary  */
        void setSummary(HitSummary&& summary);
        /** @brief Returns the pointer to the hit summary */
        const HitSummary* summary() const;
    private:
        using ParamMap_t = std::unordered_map<ParamDef, ParamData_t>;
        /** @brief Storeage of extra parameters */
        ParamMap_t m_params{};
        /** @brief List of associated segments */
        std::vector<const xAOD::MuonSegment*> m_segments{};
        /** @brief Pointer to the muon track summary */
        std::unique_ptr<HitSummary> m_summary{};
        /** @brief The Pointer to the ID track particle */
        const xAOD::TrackParticle* m_idTrack{nullptr};
        /** @brief The pointer to the MS track particle */
        const xAOD::TrackParticle* m_msTrack{nullptr};
        /** @brief The pointer to the combiend track particle */
        const xAOD::TrackParticle* m_cbTrack{nullptr};
        /** @brief the tag's author */
        Author m_author{Author::unknown};
    };

    using MuonTagContainer = DataVector<MuonTag>;
}

CLASS_DEF( MuonR4::MuonTagContainer , 1314634447 , 1 )

#include "MuonTrackEvent/MuonTag.icc"

#endif