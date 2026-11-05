/*
  Copyright (C) 2002-2026 CERN
  for the benefit of the ATLAS collaboration
*/
#ifndef PRDTESTERR4_STGCPRDVARIABLES_H
#define PRDTESTERR4_STGCPRDVARIABLES_H

#include "MuonPRDTestR4/TesterModuleBase.h"
#include "MuonTesterTree/IdentifierBranch.h"
#include "MuonTesterTree/TwoVectorBranch.h"
#include "xAODMuonPrepData/sTgcStripContainer.h"
#include "xAODMuonPrepData/sTgcWireContainer.h"
#include "xAODMuonPrepData/sTgcPadContainer.h"

#include <cstdint>
#include <string>

namespace MuonValR4 {

/** One tester module reading all three Phase-II sTGC PRD collections.
 *  ROOT output is separated into StgcStrip_*, StgcWire_*, StgcPad_*.
 *  The constituent strip branches are booked only when selected.
 */

class StgcPrdVariables final : public TesterModuleBase {
public:
     /**
     * @param tree Tester tree that receives the output branches.
     * @param stripKey StoreGate key for the sTGC strip-cluster container.
     * @param wireKey StoreGate key for the sTGC wire-hit container.
     * @param padKey StoreGate key for the sTGC pad-hit container.
     * @param writeStripNumbers Whether to write the per-cluster strip numbers.
     * @param writeStripCharges Whether to write the per-strip charges.
     * @param writeStripTimes Whether to write the per-strip times.
     * @param msgLvl Message level used by the tester module.
     */
    StgcPrdVariables(MuonVal::MuonTesterTree& tree,
                     const std::string& stripKey,
                     const std::string& wireKey,
                     const std::string& padKey,
                     bool writeStripNumbers = true,
                     bool writeStripCharges = true,
                     bool writeStripTimes = true,
                     MSG::Level msgLvl = MSG::INFO);

    /** @brief Declare dependencies on the configured strip, wire, and pad containers. */
    bool declare_keys() override final;
    /**
     * @brief Read the sTGC containers and append selected measurements to the tree.
     * @param ctx Event context used to retrieve the containers and geometry.
     * @return True if all three containers are valid and processed; false otherwise.
     */
    bool fill(const EventContext& ctx) override final;
    /**
     * @brief Select a strip cluster for output.
     * @param prd Strip cluster to select.
     * @return Its index in the strip output, reusing the index if already selected.
     */
    unsigned int push_back( const xAOD::sTgcStripCluster& prd);
    /**
     * @brief Select a wire hit for output.
     * @param prd Wire hit to select.
     * @return Its index in the wire output, reusing the index if already selected.
     */
    unsigned int push_back( const xAOD::sTgcWireHit& prd);
    /**
     * @brief Select a pad hit for output.
     * @param prd Pad hit to select.
     * @return Its index in the pad output, reusing the index if already selected.
     */
    unsigned int push_back( const xAOD::sTgcPadHit& prd);
    /**
     * @brief Enable selection-based output. Only explicitly selected measurements and measurements from chambers * passed to dumpAllHitsInChamber() are written.
     */
    void enableSeededDump();
    /**
     * @brief Include all sTGC measurements from the specified chamber in the output.
     * @param chamberId Identifier of the chamber to include.
     */
    void dumpAllHitsInChamber( const Identifier& chamberId);


private:
    /** @brief Append a strip cluster's identifier and measurement data to the tree.
     * @param gctx Geometry context used to calculate the global position.
     * @param prd Strip cluster to write.
     */
    void dumpStrip(const ActsTrk::GeometryContext& gctx, const xAOD::sTgcStripCluster& prd);
    /**
     * @brief Append a wire hit's identifier and measurement data to the tree.
     * @param gctx Geometry context used to calculate the global position.
     * @param prd Wire hit to write.
     */
    void dumpWire(const ActsTrk::GeometryContext& gctx, const xAOD::sTgcWireHit& prd);
    /**
     * @brief Append a pad hit's identifier and measurement data to the tree.
     * @param gctx Geometry context used to calculate the global position.
     * @param prd Pad hit to write.
     */
    void dumpPad(const ActsTrk::GeometryContext& gctx, const xAOD::sTgcPadHit& prd);

    /** @brief Clear the per-event measurement and chamber selections. */
    void clearEventSelection();

    /** @brief StoreGate key for the sTGC strip-cluster container. */
    SG::ReadHandleKey<xAOD::sTgcStripContainer> m_stripKey{};
    /** @brief StoreGate key for the sTGC wire-hit container. */
    SG::ReadHandleKey<xAOD::sTgcWireContainer> m_wireKey{};
    /** @brief StoreGate key for the sTGC pad-hit container. */
    SG::ReadHandleKey<xAOD::sTgcPadContainer> m_padKey{};


    /** @brief Identifier branch for sTGC strip clusters. */
    MuonVal::sTgcIdentifierBranch m_stripId{parent(), "StgcStrip"};
    /** @brief Global-frame position of each sTGC strip cluster. */
    MuonVal::ThreeVectorBranch m_stripGlobal{parent(), "StgcStrip_globalPos"};
    /** @brief Measured local position of each sTGC strip cluster. */
    MuonVal::VectorBranch<float>& m_stripLocal{parent().newVector<float>("StgcStrip_localPos")};
    /** @brief Local measurement variance of each sTGC strip cluster. */
    MuonVal::VectorBranch<float>& m_stripCov{parent().newVector<float>("StgcStrip_localCov")};
    /** @brief Time recorded for each sTGC strip cluster. */
    MuonVal::VectorBranch<short>& m_stripTime{parent().newVector<short>("StgcStrip_time")};
    /** @brief Charge recorded for each sTGC strip cluster. */
    MuonVal::VectorBranch<int>& m_stripCharge{parent().newVector<int>("StgcStrip_charge")};
    /** @brief Reconstruction author code for each sTGC strip cluster. */
    MuonVal::VectorBranch<uint8_t>& m_stripAuthor{parent().newVector<uint8_t>("StgcStrip_author")};
    //MuonVal::VectorBranch<uint8_t>& m_stripQuality{parent().newVector<uint8_t>("StgcStrip_quality")};

    /** @brief Identifier branch for sTGC wire hits. */
    MuonVal::sTgcIdentifierBranch m_wireId{parent(), "StgcWire"};
    /** @brief Global-frame position of each sTGC wire hit. */
    MuonVal::ThreeVectorBranch m_wireGlobal{parent(), "StgcWire_globalPos"};
    /** @brief Measured local position of each wire hit, along local Y. */
    MuonVal::VectorBranch<float>& m_wireLocal{parent().newVector<float>("StgcWire_localPos")};
    /** @brief Local measurement variance of each sTGC wire hit. */
    MuonVal::VectorBranch<float>& m_wireCov{parent().newVector<float>("StgcWire_localCov")};
    /** @brief Time recorded for each sTGC wire hit. */
    MuonVal::VectorBranch<short>& m_wireTime{parent().newVector<short>("StgcWire_time")};
    /** @brief Charge recorded for each sTGC wire hit. */
    MuonVal::VectorBranch<int>& m_wireCharge{parent().newVector<int>("StgcWire_charge")};
    /** @brief Reconstruction author code for each sTGC wire hit. */
    MuonVal::VectorBranch<uint8_t>& m_wireAuthor{parent().newVector<uint8_t>("StgcWire_author")};

    /** @brief Identifier branch for sTGC pad hits. */
    MuonVal::sTgcIdentifierBranch m_padId{parent(), "StgcPad"};
    /** @brief Global-frame position of each sTGC pad hit. */
    MuonVal::ThreeVectorBranch m_padGlobal{parent(), "StgcPad_globalPos"};
    /** @brief Local X coordinate of each sTGC pad hit. */
    MuonVal::VectorBranch<float>& m_padLocalX{parent().newVector<float>("StgcPad_localPosX")};
    /** @brief Local Y coordinate of each sTGC pad hit. */
    MuonVal::VectorBranch<float>& m_padLocalY{parent().newVector<float>("StgcPad_localPosY")};
    /** @brief Local XX covariance component of each sTGC pad hit. */
    MuonVal::VectorBranch<float>& m_padCovXX{parent().newVector<float>("StgcPad_covXX")};
    /** @brief Local XY covariance component of each sTGC pad hit. */
    MuonVal::VectorBranch<float>& m_padCovXY{parent().newVector<float>("StgcPad_covXY")};
    /** @brief Local YY covariance component of each sTGC pad hit. */
    MuonVal::VectorBranch<float>& m_padCovYY{parent().newVector<float>("StgcPad_covYY")};
    /** @brief Time recorded for each sTGC pad hit. */
    MuonVal::VectorBranch<short>& m_padTime{parent().newVector<short>("StgcPad_time")};
    /** @brief Charge recorded for each sTGC pad hit. */
    MuonVal::VectorBranch<int>& m_padCharge{parent().newVector<int>("StgcPad_charge")};
    /** @brief Reconstruction author code for each sTGC pad hit. */
    MuonVal::VectorBranch<uint8_t>& m_padAuthor{parent().newVector<uint8_t>("StgcPad_author")};

    /** @brief Optional per-cluster lists of constituent strip numbers; null when not booked. */
    MuonVal::MatrixBranch<uint16_t>* m_stripNumbers{nullptr};
    /** @brief Optional per-cluster lists of constituent strip charges; null when not booked. */
    MuonVal::MatrixBranch<int>* m_stripCharges{nullptr};
    /** @brief Optional per-cluster lists of constituent strip times; null when not booked. */
    MuonVal::MatrixBranch<short>* m_stripTimes{nullptr};


    /** @brief Chamber identifiers whose measurements are included by the chamber filter. */
    std::unordered_set<Identifier> m_filteredChamb{};

    /** @brief Map from strip-cluster identifier to its output index. */
    std::unordered_map<Identifier, unsigned int> m_stripIdx{};
    /** @brief Map from wire-hit identifier to its output index. */
    std::unordered_map<Identifier, unsigned int> m_wireIdx{};
    /** @brief Map from pad-hit identifier to its output index. */
    std::unordered_map<Identifier, unsigned int> m_padIdx{};

    /** @brief Strip clusters explicitly selected for output in the current event. */
    std::vector<const xAOD::sTgcStripCluster*> m_seedStrips{};
    /** @brief Wire hits explicitly selected for output in the current event. */
    std::vector<const xAOD::sTgcWireHit*> m_seedWires{};
    /** @brief Pad hits explicitly selected for output in the current event. */
    std::vector<const xAOD::sTgcPadHit*> m_seedPads{};

    /** @brief Whether output is restricted to selected measurements and chambers. */
    bool m_applyFilter{false};
};

} // namespace MuonValR4
#endif
