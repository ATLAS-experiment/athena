
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <cstdint>
#include <fstream>
#include <stdexcept>

#include "ActsClusterComparisonAlg.h"

#include "AthAllocators/DataPool.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "InDetReadoutGeometry/SiDetectorDesign.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "ReadoutGeometryBase/SiCellId.h"
#include "SCT_ReadoutGeometry/SCT_BarrelModuleSideDesign.h"
#include "SCT_ReadoutGeometry/SCT_ForwardModuleSideDesign.h"
#include "SCT_ReadoutGeometry/SCT_ModuleSideDesign.h"
#include "SCT_ReadoutGeometry/StripStereoAnnulusDesign.h"
#include "StoreGate/ReadHandle.h"
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"
#include "xAODInDetMeasurement/SpacePoint.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"
#include "xAODMeasurementBase/MeasurementDefs.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"


namespace ActsTrk {

StatusCode ActsClusterComparisonAlg::initialize()
{
    ATH_MSG_INFO("ActsClusterComparisonAlg::initialize");

    ATH_CHECK(detStore()->retrieve(m_pixelManager, m_pixelManagerKey));
    ATH_CHECK(detStore()->retrieve(m_stripManager, m_stripManagerKey));

    ATH_CHECK(detStore()->retrieve(m_stripID, "SCT_ID"));

    ATH_CHECK(m_stripLorentzAngleTool.retrieve());
    ATH_CHECK(m_pixelLorentzAngleTool.retrieve());

    ATH_CHECK(m_monitoredPixelClustersKey.initialize());
    ATH_CHECK(m_monitoredStripClustersKey.initialize());

    ATH_CHECK(m_referencePixelClustersKey.initialize());
    ATH_CHECK(m_referenceStripClustersKey.initialize());

    ATH_CHECK(m_referenceSpacepointsKey.initialize(m_checkSpacepoints));
    ATH_CHECK(m_monitoredSpacepointsKey.initialize(m_checkSpacepoints));

    ATH_MSG_INFO("ActsClusterComparisonAlg::initialize complete");
    return StatusCode::SUCCESS;
}

StatusCode ActsClusterComparisonAlg::execute(const EventContext& ctx) const
{


    // Create per-event data structures
    std::unordered_map<const xAOD::PixelCluster*, const xAOD::PixelCluster*>
            pixel_cluster_matches;

    std::unordered_map<const xAOD::StripCluster*, const xAOD::StripCluster*>
            strip_cluster_matches;

    
    ATH_CHECK(validateClusters(ctx, pixel_cluster_matches, strip_cluster_matches));

    if (m_checkSpacepoints)
        ATH_CHECK(validatePixelSpacepoints(ctx, pixel_cluster_matches));

    return StatusCode::SUCCESS;
}

// Match pixel clusters
void  ActsClusterComparisonAlg::matchPixelClusters(
    std::vector<const xAOD::PixelCluster*>& monitored_list,
    std::vector<const xAOD::PixelCluster*>& reference_list,
    const std::string& module_id,
    std::vector<
        std::pair<const xAOD::PixelCluster*, const xAOD::PixelCluster*>>& pairs) const
{

    // Extract RDO sets
    std::vector<std::set<Identifier>> monitored_rdo_sets;
    std::vector<std::set<Identifier>> reference_rdo_sets;

    for (const auto* c : monitored_list) {
        const auto& rdoListRange = c->rdoList();
        std::vector<Identifier> rdoList(rdoListRange.begin(), rdoListRange.end());
        monitored_rdo_sets.emplace_back(rdoList.begin(), rdoList.end());
    }

    for (const auto* c : reference_list) {
        const auto& rdoListRange = c->rdoList();
        std::vector<Identifier> rdoList(rdoListRange.begin(), rdoListRange.end());
        reference_rdo_sets.emplace_back(rdoList.begin(), rdoList.end());
    }

    // Match clusters
    std::vector<std::pair<int, int>> matched_pairs;
    std::vector<int> unmatched_monitored;
    std::set<int> unmatched_reference;

    for (size_t j = 0; j < reference_rdo_sets.size(); ++j) {
        unmatched_reference.insert(j);
    }

    for (size_t i = 0; i < monitored_rdo_sets.size(); ++i) {

        bool found_match = false;
        for (size_t j = 0; j < reference_rdo_sets.size(); ++j) {
            if (monitored_rdo_sets[i] == reference_rdo_sets[j]) {
                matched_pairs.emplace_back(i, j);
                pairs.emplace_back(monitored_list.at(i), reference_list.at(j));
                found_match = true;
                unmatched_reference.erase(j);
                break;
            }
        }
        if (!found_match) {
            unmatched_monitored.push_back(i);
        }
    }

    // Report mismatches
    if (!unmatched_monitored.empty() || !unmatched_reference.empty()) {
        ATH_MSG_DEBUG("[ERROR] Module " << module_id
                  << ": cluster mismatch detected!");
        if (!unmatched_monitored.empty()) {
            ATH_MSG_DEBUG("  Unmatched monitored clusters ("
                      << unmatched_monitored.size() << "):");
            for (int i : unmatched_monitored) {
                ATH_MSG_DEBUG("    - #" << i);
                const xAOD::PixelCluster* monitored_cluster = monitored_list.at(i);
                const auto& monitored_cov = monitored_cluster->localCovariance<1>();

                ATH_MSG_DEBUG("Detailed print of cluster: ");

                ATH_MSG_DEBUG("Local position: ");
                ATH_MSG_DEBUG(
                    "  Monitored: ("
                    << monitored_cluster->localPosition<2>()[Trk::locX] << ", "
                    << monitored_cluster->localPosition<2>()[Trk::locY] << ")");
                
                ATH_MSG_DEBUG("Cluster cov: ");
                ATH_MSG_DEBUG("  Monitored: (" << monitored_cov(0, 0) << ", "
                                            << monitored_cov(1, 1) << ")");
            }
            m_pix_unmatched_mon+= unmatched_monitored.size();
        }
        if (!unmatched_reference.empty()) {
            ATH_MSG_DEBUG("  Unmatched reference clusters (" << unmatched_reference.size()
                      << "):");
            for (int j : unmatched_reference) {
                ATH_MSG_DEBUG("    - #" << j);
                const xAOD::PixelCluster* reference_cluster = reference_list.at(j);
                const auto& reference_cov = reference_cluster->localCovariance<1>();

                ATH_MSG_DEBUG("Detailed print of cluster: ");
                
                ATH_MSG_DEBUG("Local position: ");
                
                ATH_MSG_DEBUG("  Reference: ("
                            << reference_cluster->localPosition<2>()[Trk::locX] << ", "
                            << reference_cluster->localPosition<2>()[Trk::locY]
                            << ")");
                
                ATH_MSG_DEBUG("Cluster cov: ");
                
                ATH_MSG_DEBUG("  Reference: (" << reference_cov(0, 0) << ", "
                                        << reference_cov(1, 1) << ")");
            }
            m_pix_unmatched_ref+= unmatched_reference.size();
        }
        ATH_MSG_DEBUG("------------------------------------------------------------");
    }
}

// Match strip clusters
void  ActsClusterComparisonAlg::matchStripClusters(
    std::vector<const xAOD::StripCluster*>& monitored_list,
    std::vector<const xAOD::StripCluster*>& reference_list,
    const std::string& module_id,
    std::vector<
        std::pair<const xAOD::StripCluster*, const xAOD::StripCluster*>>& pairs) const
{

    // Extract RDO sets
    std::vector<std::set<Identifier>> monitored_rdo_sets;
    std::vector<std::set<Identifier>> reference_rdo_sets;

    for (const auto* c : monitored_list) {
        const auto& rdoListRange = c->rdoList();
        std::vector<Identifier> rdoList(rdoListRange.begin(), rdoListRange.end());
        monitored_rdo_sets.emplace_back(rdoList.begin(), rdoList.end());
    }

    for (const auto* c : reference_list) {
        const auto& rdoListRange = c->rdoList();
        std::vector<Identifier> rdoList(rdoListRange.begin(), rdoListRange.end());
        reference_rdo_sets.emplace_back(rdoList.begin(), rdoList.end());
    }

    // Match clusters
    std::vector<std::pair<int, int>> matched_pairs;
    std::vector<int> unmatched_monitored;
    std::set<int> unmatched_reference;

    for (size_t j = 0; j < reference_rdo_sets.size(); ++j) {
        unmatched_reference.insert(j);
    }

    for (size_t i = 0; i < monitored_rdo_sets.size(); ++i) {
        bool found_match = false;
        for (size_t j = 0; j < reference_rdo_sets.size(); ++j) {
            if (monitored_rdo_sets[i] == reference_rdo_sets[j]) {
                matched_pairs.emplace_back(i, j);
                pairs.emplace_back(monitored_list.at(i), reference_list.at(j));
                found_match = true;
                unmatched_reference.erase(j);
                break;
            }
        }
        if (!found_match) {
            unmatched_monitored.push_back(i);
        }
    }

    // Report mismatches
    if (!unmatched_monitored.empty() || !unmatched_reference.empty()) {
        ATH_MSG_DEBUG("[ERROR] Module " << module_id
                  << ": cluster mismatch detected!");
        if (!unmatched_monitored.empty()) {
            ATH_MSG_DEBUG("  Unmatched monitored clusters ("
                      << unmatched_monitored.size() << "):");
            for (int i : unmatched_monitored) {
                ATH_MSG_DEBUG("    - #" << i);
                const xAOD::StripCluster* monitored_cluster = monitored_list.at(i);
                const auto& monitored_cov = monitored_cluster->localCovariance<1>();

                ATH_MSG_DEBUG("Detailed print of cluster: ");

                ATH_MSG_DEBUG("Local position: ");
                ATH_MSG_DEBUG(
                    "  Monitored: ("
                    << monitored_cluster->localPosition<1>()[Trk::locX] << ")");
                
                ATH_MSG_DEBUG("Cluster cov: ");
                ATH_MSG_DEBUG("  Monitored: (" << monitored_cov(0, 0) << ")");
            }
            m_strip_unmatched_mon+= unmatched_monitored.size();
        }
        if (!unmatched_reference.empty()) {
            ATH_MSG_DEBUG("  Unmatched reference clusters (" << unmatched_reference.size()
                      << "):");
            for (int j : unmatched_reference) {
                ATH_MSG_DEBUG("    - #" << j);
                const xAOD::StripCluster* reference_cluster = reference_list.at(j);
                const auto& reference_cov = reference_cluster->localCovariance<1>();

                ATH_MSG_DEBUG("Detailed print of cluster: ");
                
                ATH_MSG_DEBUG("Local position: ");
                
                ATH_MSG_DEBUG("  Reference: ("
                            << reference_cluster->localPosition<1>()[Trk::locX] << ")");
                
                ATH_MSG_DEBUG("Cluster cov: ");
                
                ATH_MSG_DEBUG("  Reference: (" << reference_cov(0, 0) << ")");
            }
            m_strip_unmatched_ref+= unmatched_reference.size();
        }
        ATH_MSG_DEBUG("------------------------------------------------------------");
    }
}

StatusCode ActsClusterComparisonAlg::validateClusters(
    const EventContext& eventContext, std::unordered_map<const xAOD::PixelCluster*, const xAOD::PixelCluster*>& pixel_cluster_matches, std::unordered_map<const xAOD::StripCluster*, const xAOD::StripCluster*>& strip_cluster_matches) const
{

    ATH_MSG_DEBUG(
        "============================================================");
    // retrieve the clusters in form of xAOD containers
    ATH_MSG_INFO("Reading monitored clusters: " << m_monitoredPixelClustersKey.key()
                                             << " and "
                                             << m_monitoredStripClustersKey.key());
    ATH_MSG_INFO("Reading reference clusters: " << m_referencePixelClustersKey.key()
                                           << " and "
                                           << m_referenceStripClustersKey.key());
    ATH_MSG_DEBUG(
        "============================================================");

    SG::ReadHandle<xAOD::PixelClusterContainer> monitoredPixelClustersHandle =
        SG::makeHandle(m_monitoredPixelClustersKey, eventContext);
    ATH_CHECK(monitoredPixelClustersHandle.isValid());
    const xAOD::PixelClusterContainer* monitoredPixelClusters =
        monitoredPixelClustersHandle.cptr();

    SG::ReadHandle<xAOD::StripClusterContainer> monitoredStripClustersHandle =
        SG::makeHandle(m_monitoredStripClustersKey, eventContext);
    ATH_CHECK(monitoredStripClustersHandle.isValid());
    const xAOD::StripClusterContainer* monitoredStripClusters =
        monitoredStripClustersHandle.cptr();

    SG::ReadHandle<xAOD::PixelClusterContainer> referencePixelClustersHandle =
        SG::makeHandle(m_referencePixelClustersKey, eventContext);
    ATH_CHECK(referencePixelClustersHandle.isValid());
    const xAOD::PixelClusterContainer* referencePixelClusters =
        referencePixelClustersHandle.cptr();

    SG::ReadHandle<xAOD::StripClusterContainer> referenceStripClustersHandle =
        SG::makeHandle(m_referenceStripClustersKey, eventContext);
    ATH_CHECK(referenceStripClustersHandle.isValid());
    const xAOD::StripClusterContainer* referenceStripClusters =
        referenceStripClustersHandle.cptr();

    size_t t_n_pixel = monitoredPixelClusters->size();
    size_t t_n_strip = monitoredStripClusters->size();
    size_t a_n_pixel = referencePixelClusters->size();
    size_t a_n_strip = referenceStripClusters->size();

    ATH_MSG_DEBUG("  Monitored/reference pixel clusters " << t_n_pixel << " / "
                                                  << a_n_pixel);
    ATH_MSG_DEBUG("  Monitored/reference strip clusters " << t_n_strip << " / "
                                                  << a_n_strip);

    if (t_n_pixel != a_n_pixel) {
        ATH_MSG_DEBUG("[ERROR] mismatched pixel cluster numbers found!");
        ATH_MSG_DEBUG("  Monitored/reference clusters " << t_n_pixel << " / "
                                                << a_n_pixel);
    }
    if (t_n_strip != a_n_strip) {
        ATH_MSG_DEBUG("[ERROR] mismatched strip cluster numbers found!");
        ATH_MSG_DEBUG("  Monitored/reference clusters " << t_n_strip << " / "
                                                << a_n_strip);
    }

    // Group by module
    std::map<std::string, std::vector<const xAOD::PixelCluster*>>
        monitored_pixel_map, reference_pixel_map;
    std::map<std::string, std::vector<const xAOD::StripCluster*>>
        monitored_strip_map, reference_strip_map;

    for (const auto* c : *monitoredPixelClusters) {
        monitored_pixel_map[std::to_string(c->identifierHash())].push_back(c);
    }
    for (const auto* c : *monitoredStripClusters) {
        monitored_strip_map[std::to_string(c->identifierHash())].push_back(c);
    }
    for (const auto* c : *referencePixelClusters) {
        reference_pixel_map[std::to_string(c->identifierHash())].push_back(c);
    }
    for (const auto* c : *referenceStripClusters) {
        reference_strip_map[std::to_string(c->identifierHash())].push_back(c);
    }

    // Collect all module IDs
    std::set<std::string> pixel_modules, strip_modules;
    for (const auto& [key, _] : monitored_pixel_map)
        pixel_modules.insert(key);
    for (const auto& [key, _] : reference_pixel_map)
        pixel_modules.insert(key);
    for (const auto& [key, _] : monitored_strip_map)
        strip_modules.insert(key);
    for (const auto& [key, _] : reference_strip_map)
        strip_modules.insert(key);

    int pixel_unequal = 0, strip_unequal = 0;
    int matched_pixel = 0, matched_strip = 0;
    int pixel_pos_diff_0p5sig = 0;
    int pixel_pos_diff_0p25sig = 0;
    int pixel_pos_diff_1sig = 0;
    int strip_pos_diff_0p5sig = 0;
    int strip_pos_diff_0p25sig = 0;
    int strip_pos_diff_1sig = 0;

    ATH_MSG_DEBUG("Pixel/Strip modules " << pixel_modules.size() << " / "
                                           << strip_modules.size());
    ATH_MSG_DEBUG("Pixel cluster validation: ");
    ATH_MSG_DEBUG(
        "============================================================");

    // Process pixel modules
    for (const auto& hid : pixel_modules) {
        auto& tpixel = monitored_pixel_map[hid];
        auto& apixel = reference_pixel_map[hid];

        if (tpixel.empty() && apixel.empty())
            continue;

        if (tpixel.size() != apixel.size()) {
            ATH_MSG_DEBUG("[ERROR] Pixel Module " << hid
                      << ": mismatched clusters found!");
            ATH_MSG_DEBUG("  Reference found " << apixel.size()
                      << " and monitored found " << tpixel.size() << " clusters!");
            pixel_unequal++;
            continue;
        }

        std::vector<
            std::pair<const xAOD::PixelCluster*, const xAOD::PixelCluster*>>
            pixel_pairs;
        matchPixelClusters(tpixel, apixel, hid, pixel_pairs);

        matched_pixel += pixel_pairs.size();

        // Add matched pairs and check position differences
        for (const auto& pair : pixel_pairs) {
            const xAOD::PixelCluster* monitored_cluster = pair.first;
            const xAOD::PixelCluster* reference_cluster = pair.second;

            (pixel_cluster_matches)[pair.first] = pair.second;

            // Calculate local position difference
            double l_dx = monitored_cluster->localPosition<2>()[Trk::locX] -
                          reference_cluster->localPosition<2>()[Trk::locX];
            double l_dy = monitored_cluster->localPosition<2>()[Trk::locY] -
                          reference_cluster->localPosition<2>()[Trk::locY];
            double l_pos_diff = std::sqrt(l_dx * l_dx + l_dy * l_dy);

            // Calculate global position difference
            float g_dx = (monitored_cluster->globalPosition()).x() -
                         (reference_cluster->globalPosition()).x();
            float g_dy = (monitored_cluster->globalPosition()).y() -
                         (reference_cluster->globalPosition()).y();
            float g_dz = (monitored_cluster->globalPosition()).z() -
                         (reference_cluster->globalPosition()).z();
            float g_pos_diff =
                std::sqrt(g_dx * g_dx + g_dy * g_dy + g_dz * g_dz);

            // Calculate error difference
            Eigen::Matrix<float, 2, 2> monitored_cov =
                monitored_cluster->localCovariance<2>();
            Eigen::Matrix<float, 2, 2> reference_cov =
                reference_cluster->localCovariance<2>();

            const InDetDD::SiDetectorElement* monitored_element =
                m_pixelManager->getDetectorElement(
                    monitored_cluster->identifierHash());
            const InDetDD::PixelModuleDesign& design =
                static_cast<const InDetDD::PixelModuleDesign&>(
                    monitored_element->design());

            const Identifier monitored_Pixel_ModuleID =
                monitored_element->identify();
            double monitored_lorentz_shift =
                m_pixelLorentzAngleTool->getLorentzShift(
                    monitored_element->identifyHash(), eventContext);
        

            if (std::abs(l_dx / (std::sqrt(monitored_cov(0, 0)))) > 0.25 ||
                std::abs(l_dy / (std::sqrt(monitored_cov(1, 1)))) > 0.25) {
                pixel_pos_diff_0p25sig++;

                ATH_MSG_DEBUG("Detailed print of cluster discrepancy: ");
                ATH_MSG_DEBUG("On module: " << monitored_Pixel_ModuleID);
                ATH_MSG_DEBUG("Lorentz shift: " << std::fixed << std::setprecision(9) << monitored_lorentz_shift);

                ATH_MSG_DEBUG("Local position: ");
                ATH_MSG_DEBUG(
                    "  Monitored: ("
                    << monitored_cluster->localPosition<2>()[Trk::locX] << ", "
                    << monitored_cluster->localPosition<2>()[Trk::locY] << ")");
                ATH_MSG_DEBUG("  Reference: ("
                            << reference_cluster->localPosition<2>()[Trk::locX] << ", "
                            << reference_cluster->localPosition<2>()[Trk::locY]
                            << ")");
                ATH_MSG_DEBUG("  Δx = " << l_dx << ", Δy = " << l_dy
                                        << ", Δr = " << l_pos_diff);
                ATH_MSG_DEBUG("Cluster cov: ");
                ATH_MSG_DEBUG("  Monitored: (" << monitored_cov(0, 0) << ", "
                                            << monitored_cov(1, 1) << ")");
                ATH_MSG_DEBUG("  Reference: (" << reference_cov(0, 0) << ", "
                                        << reference_cov(1, 1) << ")");
                ATH_MSG_DEBUG("  Δx = " << monitored_cov(0, 0) - reference_cov(0, 0)
                                        << ", Δy = "
                                        << monitored_cov(1, 1) - reference_cov(1, 1));


                const auto& rdoListRange = monitored_cluster->rdoList();
                std::vector<Identifier> monitored_rdoList(
                    rdoListRange.begin(), rdoListRange.end());
                for (auto rdoIter : monitored_rdoList) {
                    const InDetDD::SiCellId& chargeCellId =
                        monitored_element->cellIdFromIdentifier(rdoIter);
                    std::array<InDetDD::PixelDiodeTree::CellIndexType, 2>
                        diode_idx = InDetDD::PixelDiodeTree::makeCellIndex(
                            chargeCellId.phiIndex(), chargeCellId.etaIndex());

                    InDetDD::PixelDiodeTree::DiodeProxyWithPosition si_param(
                        design.diodeProxyFromIdxCachePosition(diode_idx));
                    ATH_MSG_DEBUG("hit id for this cell: "
                                  << chargeCellId
                                  << ", position: " << si_param.position()[0]
                                  << ", " << si_param.position()[1]);
                }

                // Calculate width difference
                ATH_MSG_DEBUG("Cluster width: ");
                ATH_MSG_DEBUG("  Monitored phi/eta channels, eta width: ("
                            << monitored_cluster->channelsInPhi() << ", "
                            << monitored_cluster->channelsInEta() << ", "
                            << monitored_cluster->widthInEta() << ")");
                ATH_MSG_DEBUG("  Reference phi/eta channels, eta width: ("
                            << reference_cluster->channelsInPhi() << ", "
                            << reference_cluster->channelsInEta() << ", "
                            << reference_cluster->widthInEta() << ")");

                ATH_MSG_DEBUG(" Δphi = " << monitored_cluster->channelsInPhi() -
                                                reference_cluster->channelsInPhi()
                                        << ",  Δeta = "
                                        << monitored_cluster->channelsInEta() -
                                                reference_cluster->channelsInEta()
                                        << ", Δwidth = "
                                        << monitored_cluster->widthInEta() -
                                                reference_cluster->widthInEta());
            }
            if (std::abs(l_dx / (std::sqrt(monitored_cov(0, 0)))) > 0.5 ||
                std::abs(l_dy / (std::sqrt(monitored_cov(1, 1)))) > 0.5) {
                pixel_pos_diff_0p5sig++;
            }
            if (std::abs(l_dx / (std::sqrt(monitored_cov(0, 0)))) > 1 ||
                std::abs(l_dy / (std::sqrt(monitored_cov(1, 1)))) > 1) {
                pixel_pos_diff_1sig++;
            }


            ATH_MSG_VERBOSE("Global position: ");
            ATH_MSG_VERBOSE("  Monitored: ("
                          << (monitored_cluster->globalPosition()).x() << ", "
                          << (monitored_cluster->globalPosition()).y() << ", "
                          << (monitored_cluster->globalPosition()).z() << ")");
            ATH_MSG_VERBOSE("  Reference: ("
                          << (reference_cluster->globalPosition()).x() << ", "
                          << (reference_cluster->globalPosition()).y() << ", "
                          << (reference_cluster->globalPosition()).z() << ")");
            ATH_MSG_VERBOSE("  Δx = " << g_dx << ", Δy = " << g_dy << ", Δz = "
                                    << g_dz << ", Δr = " << g_pos_diff);

        }
    }

    ATH_MSG_DEBUG(
        "============================================================");
    ATH_MSG_DEBUG("Strip cluster validation: ");
        ATH_MSG_DEBUG(
        "============================================================");
    // Process strip modules
    for (const auto& hid : strip_modules) {
        auto& tstrip = monitored_strip_map[hid];
        auto& astrip = reference_strip_map[hid];

        if (tstrip.empty() && astrip.empty())
            continue;

        if (tstrip.size() != astrip.size()) {
            ATH_MSG_DEBUG("[ERROR] Strip Module "
                          << hid << ": mismatched clusters found!");
            ATH_MSG_DEBUG("  Reference found " << astrip.size()
                                          << " and monitored found "
                                          << tstrip.size() << " clusters!");
            strip_unequal++;
            continue;
        }

        std::vector<
            std::pair<const xAOD::StripCluster*, const xAOD::StripCluster*>>
            strip_pairs;
        matchStripClusters(tstrip, astrip, hid, strip_pairs);

        matched_strip += strip_pairs.size();

        // Add matched pairs and check position differences
        for (const auto& pair : strip_pairs) {
            const xAOD::StripCluster* monitored_cluster = pair.first;
            const xAOD::StripCluster* reference_cluster = pair.second;

            (strip_cluster_matches)[pair.first] = pair.second;

            // Calculate position difference
            double pos_diff = monitored_cluster->localPosition<1>()[Trk::locX] -
                              reference_cluster->localPosition<1>()[Trk::locX];


            // Calculate error difference
            Eigen::Matrix<float, 1, 1> monitored_cov =
                monitored_cluster->localCovariance<1>();
            Eigen::Matrix<float, 1, 1> reference_cov =
                reference_cluster->localCovariance<1>();

            const InDetDD::SiDetectorElement* monitored_element =
                m_stripManager->getDetectorElement(monitored_cluster->identifierHash());


            if (std::abs(pos_diff / (std::sqrt(monitored_cov(0, 0)))) > 0.25) {
                strip_pos_diff_0p25sig++;

                int side = m_stripID->side(monitored_element->identify());
                const Identifier strip_moduleID = m_stripID->module_id(monitored_element->identify());
                const IdentifierHash Strip_ModuleHash = m_stripID->wafer_hash(strip_moduleID);
                double monitored_lorentz_shift =
                    m_stripLorentzAngleTool->getLorentzShift(Strip_ModuleHash + side, eventContext);

                ATH_MSG_DEBUG("Detailed print of cluster discrepancy: ");
                ATH_MSG_DEBUG("On module: " << strip_moduleID << ", side: " << m_stripID->side(monitored_element->identify()));
                ATH_MSG_DEBUG("Lorentz shift: " << std::fixed << std::setprecision(9) << monitored_lorentz_shift);

                ATH_MSG_DEBUG("Local position: ");
                ATH_MSG_DEBUG(
                    "  Monitored: ("
                    << monitored_cluster->localPosition<1>()[Trk::locX] << ")");
                ATH_MSG_DEBUG("  Reference: ("
                            << reference_cluster->localPosition<1>()[Trk::locX] <<  ")");
                ATH_MSG_DEBUG("  Δx = " << pos_diff );
                ATH_MSG_DEBUG("Cluster cov: ");
                ATH_MSG_DEBUG("  Monitored: (" << monitored_cov(0, 0) << ")");
                ATH_MSG_DEBUG("  Reference: (" << reference_cov(0, 0) << ")");
                ATH_MSG_DEBUG("  Δx = " << monitored_cov(0, 0) - reference_cov(0, 0));

                const auto& rdoListRange = monitored_cluster->rdoList();
                std::vector<Identifier> monitored_rdoList(
                    rdoListRange.begin(), rdoListRange.end());

                if (monitored_element->isBarrel()) {
                    const InDetDD::SCT_BarrelModuleSideDesign* s_design = (static_cast<const InDetDD::SCT_BarrelModuleSideDesign*>(&monitored_element->design()));

                    for (auto rdoIter : monitored_rdoList) {
                        const InDetDD::SiCellId& chargeCellId =
                            monitored_element->cellIdFromIdentifier(rdoIter);
                        InDetDD::SiLocalPosition si_pos =
                            s_design->localPositionOfCell(chargeCellId);
                        Amg::Vector2D loc_pos(si_pos.xPhi(), si_pos.xEta());
                        ATH_MSG_DEBUG("hit id for this cell: "
                                    << chargeCellId
                                    << ", position: " << loc_pos[0]
                                    << ", " << loc_pos[1]);
                    }

                }else{

                    const InDetDD::StripStereoAnnulusDesign* annulus_design = (static_cast<const InDetDD::StripStereoAnnulusDesign*>(&monitored_element->design()));

                    for (auto rdoIter : monitored_rdoList) {
                        const InDetDD::SiCellId& chargeCellId =
                            monitored_element->cellIdFromIdentifier(rdoIter);

                        InDetDD::SiLocalPosition si_pos =
                            annulus_design->localPositionOfCell(chargeCellId);
                        Amg::Vector2D loc_pos(si_pos.xPhi(), si_pos.xEta());
                        ATH_MSG_DEBUG("hit id for this cell: "
                                    << chargeCellId
                                    << ", posiiton: " << loc_pos[0]
                                    << ", " << loc_pos[1]);
                    }

                }

                // Calculate width difference
                ATH_MSG_DEBUG("Cluster width: ");
                ATH_MSG_DEBUG("  Monitored phi channels: ("
                            << monitored_cluster->channelsInPhi() << ")");
                ATH_MSG_DEBUG("  Reference phi channels: ("
                            << reference_cluster->channelsInPhi() << ")");

                ATH_MSG_DEBUG(" Δphi = " << monitored_cluster->channelsInPhi() -
                                            reference_cluster->channelsInPhi());

            }
            if (std::abs(pos_diff / (std::sqrt(monitored_cov(0, 0)))) > 0.5) {
                strip_pos_diff_0p5sig++;
            }
            if (std::abs(pos_diff / (std::sqrt(monitored_cov(0, 0)))) > 1) {
                strip_pos_diff_1sig++;
            }

        }
    }

    ATH_MSG_DEBUG(
        "============================================================");

    // Print statistics
    ATH_MSG_DEBUG(
        "============================================================");
    ATH_MSG_DEBUG("PIXEL CLUSTER MATCHING STATISTICS: ");
    ATH_MSG_DEBUG("  Total matched clusters: " << matched_pixel);
    ATH_MSG_DEBUG("  Clusters with pos diff > 1 sigma: "
                  << pixel_pos_diff_1sig << " ("
                  << (matched_pixel > 0
                          ? 100.0 * pixel_pos_diff_1sig / matched_pixel
                          : 0.0)
                  << "%)");
    ATH_MSG_DEBUG("  Clusters with pos diff > 0.5 sigma: "
                  << pixel_pos_diff_0p5sig << " ("
                  << (matched_pixel > 0
                          ? 100.0 * pixel_pos_diff_0p5sig / matched_pixel
                          : 0.0)
                  << "%)");
    ATH_MSG_DEBUG("  Clusters with pos diff > 0.25 sigma: "
                  << pixel_pos_diff_0p25sig << " ("
                  << (matched_pixel > 0
                          ? 100.0 * pixel_pos_diff_0p25sig / matched_pixel
                          : 0.0)
                  << "%)");
    ATH_MSG_DEBUG(
        "============================================================");
    ATH_MSG_DEBUG(
        "============================================================");
    ATH_MSG_DEBUG("STRIP CLUSTER MATCHING STATISTICS:");
    ATH_MSG_DEBUG("  Total matched clusters: " << matched_strip);
    ATH_MSG_DEBUG("  Clusters with pos diff > 1 sigma: "
                  << strip_pos_diff_1sig << " ("
                  << (matched_strip > 0
                          ? 100.0 * strip_pos_diff_1sig / matched_strip
                          : 0.0)
                  << "%)");
    ATH_MSG_DEBUG("  Clusters with pos diff > 0.5 sigma: "
                  << strip_pos_diff_0p5sig << " ("
                  << (matched_strip > 0
                          ? 100.0 * strip_pos_diff_0p5sig / matched_strip
                          : 0.0)
                  << "%)");
    ATH_MSG_DEBUG("  Clusters with pos diff > 0.25 sigma: "
                  << strip_pos_diff_0p25sig << " ("
                  << (matched_strip > 0
                          ? 100.0 * strip_pos_diff_0p25sig / matched_strip
                          : 0.0)
                  << "%)");
    ATH_MSG_DEBUG(
        "============================================================");

    m_pixel_unequal += pixel_unequal;
    m_strip_unequal += strip_unequal;
    m_matched_pixel += matched_pixel;
    m_matched_strip += matched_strip;
    m_pixel_pos_diff_1sig += pixel_pos_diff_1sig;
    m_pixel_pos_diff_0p5sig += pixel_pos_diff_0p5sig;
    m_pixel_pos_diff_0p25sig += pixel_pos_diff_0p25sig;
    m_strip_pos_diff_1sig += strip_pos_diff_1sig;
    m_strip_pos_diff_0p5sig += strip_pos_diff_0p5sig;
    m_strip_pos_diff_0p25sig += strip_pos_diff_0p25sig;

    return StatusCode::SUCCESS;
}

StatusCode ActsClusterComparisonAlg::validatePixelSpacepoints(
    const EventContext& eventContext, std::unordered_map<const xAOD::PixelCluster*, const xAOD::PixelCluster*>& pixel_cluster_matches) const
{

    // retrieve the spacepoints in form of xAOD containers
    ATH_MSG_INFO(
        "Reading monitored spacepoints: " << m_monitoredSpacepointsKey.key());
    ATH_MSG_INFO("Reading reference spacepoints: " << m_referenceSpacepointsKey.key());

    SG::ReadHandle<xAOD::SpacePointContainer> monitoredSpacepointsHandle =
        SG::makeHandle(m_monitoredSpacepointsKey, eventContext);
    ATH_CHECK(monitoredSpacepointsHandle.isValid());
    const xAOD::SpacePointContainer* monitoredSpacepoints =
        monitoredSpacepointsHandle.cptr();

    SG::ReadHandle<xAOD::SpacePointContainer> referenceSpacepointsHandle =
        SG::makeHandle(m_referenceSpacepointsKey, eventContext);
    ATH_CHECK(referenceSpacepointsHandle.isValid());
    const xAOD::SpacePointContainer* referenceSpacepoints =
        referenceSpacepointsHandle.cptr();

    size_t t_n_sp = monitoredSpacepoints->size();
    size_t a_n_sp = referenceSpacepoints->size();

    ATH_MSG_DEBUG(" Monitored/reference spacepoints " << t_n_sp << " / " << a_n_sp);

    if (t_n_sp != a_n_sp) {
        ATH_MSG_DEBUG("[ERROR] mismatched spacepoint numbers found!");
        ATH_MSG_DEBUG("  Monitored/reference spacepoints " << t_n_sp << " / "
                                                   << a_n_sp);
    }

    // Now match spacepoints based on their constituent clusters
    int matched_sp = 0;
    int unmatched_monitored_sp = 0;
    int unmatched_reference_sp = 0;
    int sp_global_pos_diff_1mm = 0;
    int sp_global_pos_diff_5mm = 0;
    int sp_variance_r_diff = 0;
    int sp_variance_z_diff = 0;

    std::vector<std::pair<const xAOD::SpacePoint*, const xAOD::SpacePoint*>>
        sp_matches;
    std::set<const xAOD::SpacePoint*> matched_reference_sp;

    for (const auto* monitored_sp : *monitoredSpacepoints) {

        const auto& monitored_measurements = monitored_sp->measurements();

        // Find matching ACTS spacepoint by comparing cluster content
        bool found_match = false;
        for (const auto* reference_sp : *referenceSpacepoints) {

            const auto& reference_measurements = reference_sp->measurements();

            // Check if measurements sizes match
            if (monitored_measurements.size() != reference_measurements.size())
                continue;

            // Check if all clusters match
            bool all_clusters_match = true;
            for (size_t i = 0; i < monitored_measurements.size(); ++i) {
                const xAOD::UncalibratedMeasurement* monitored_meas =
                    monitored_measurements[i];
                const xAOD::UncalibratedMeasurement* reference_meas =
                    reference_measurements[i];

                // Try pixel cluster matching
                auto monitored_pixel =
                    dynamic_cast<const xAOD::PixelCluster*>(monitored_meas);
                auto reference_pixel =
                    dynamic_cast<const xAOD::PixelCluster*>(reference_meas);

                if (monitored_pixel && reference_pixel) {
                    auto it = pixel_cluster_matches.find(monitored_pixel);
                    if (it == pixel_cluster_matches.end() ||
                        it->second != reference_pixel) {
                        all_clusters_match = false;
                        break;
                    }
                } else {
                    // Only pixel spacepoints are supported for now
                    ATH_MSG_WARNING("Non-pixel measurement in spacepoint comparison — "
                                    "strip spacepoints are not yet handled, skipping pair");
                    all_clusters_match = false;
                    break;
                }
            }

            if (all_clusters_match) {

                // Found matching spacepoint pair
                matched_sp++;
                sp_matches.push_back(std::make_pair(monitored_sp, reference_sp));
                matched_reference_sp.insert(reference_sp);
                found_match = true;

                break;
            }
        }

        if (!found_match) {
            unmatched_monitored_sp++;
        }
    }

    for (auto& sp_pair : sp_matches) {

        const xAOD::SpacePoint* monitored_sp = sp_pair.first;
        const xAOD::SpacePoint* reference_sp = sp_pair.second;

        double dx = monitored_sp->x() - reference_sp->x();
        double dy = monitored_sp->y() - reference_sp->y();
        double dz = monitored_sp->z() - reference_sp->z();
        double pos_diff = std::sqrt(dx * dx + dy * dy + dz * dz);

        // Calculate global position difference
        ATH_MSG_DEBUG("Spacepoint global position: ");
        ATH_MSG_DEBUG("  Monitored: (" << monitored_sp->x() << ", " << monitored_sp->y()
                                    << ", " << monitored_sp->z() << ")");
        ATH_MSG_DEBUG("  Reference: (" << reference_sp->x() << ", " << reference_sp->y()
                                  << ", " << reference_sp->z() << ")");
        ATH_MSG_DEBUG("  Δx = " << dx << ", Δy = " << dy << ", Δz = " << dz
                                << ", Δr = " << pos_diff);

        if (pos_diff > 1.0)
            sp_global_pos_diff_1mm++;
        if (pos_diff > 5.0)
            sp_global_pos_diff_5mm++;

        // Calculate covariance difference
        ATH_MSG_DEBUG("Spacepoint cov and radius: ");
        ATH_MSG_DEBUG("  Monitored r/z and radius: ("
                      << monitored_sp->varianceR() << ", "
                      << monitored_sp->varianceZ() << ", " << monitored_sp->radius()
                      << ")");
        ATH_MSG_DEBUG("  Reference r/z and radius: (" << reference_sp->varianceR() << ", "
                                                 << reference_sp->varianceZ() << ", "
                                                 << reference_sp->radius() << ")");
        ATH_MSG_DEBUG("  Δcov_r = "
                      << monitored_sp->varianceR() - reference_sp->varianceR()
                      << ", Δcov_z = "
                      << monitored_sp->varianceZ() - reference_sp->varianceZ()
                      << ", Δradius = "
                      << monitored_sp->radius() - reference_sp->radius());

        if (reference_sp->varianceR() > 0 &&
            std::abs(monitored_sp->varianceR() - reference_sp->varianceR()) /
                reference_sp->varianceR() > 0.1)
            sp_variance_r_diff++;
        if (reference_sp->varianceZ() > 0 &&
            std::abs(monitored_sp->varianceZ() - reference_sp->varianceZ()) /
                reference_sp->varianceZ() > 0.1)
            sp_variance_z_diff++;

    }

    unmatched_reference_sp = referenceSpacepoints->size() - matched_reference_sp.size();

    // Print statistics
    ATH_MSG_INFO(
        "============================================================");
    ATH_MSG_INFO("SPACEPOINT MATCHING STATISTICS:");
    ATH_MSG_INFO("  Total monitored spacepoints: " << t_n_sp);
    ATH_MSG_INFO("  Total reference spacepoints: " << a_n_sp);
    ATH_MSG_INFO("  Matched spacepoints: " << matched_sp);
    ATH_MSG_INFO("  Unmatched monitored spacepoints: " << unmatched_monitored_sp);
    ATH_MSG_INFO("  Unmatched reference spacepoints: " << unmatched_reference_sp);
    ATH_MSG_INFO(
        "============================================================");
    ATH_MSG_INFO("SPACEPOINT POSITION COMPARISON:");
    ATH_MSG_INFO(
        "  Spacepoints with global pos diff > 1 mm: "
        << sp_global_pos_diff_1mm << " ("
        << (matched_sp > 0 ? 100.0 * sp_global_pos_diff_1mm / matched_sp : 0.0)
        << "%)");
    ATH_MSG_INFO(
        "  Spacepoints with global pos diff > 5 mm: "
        << sp_global_pos_diff_5mm << " ("
        << (matched_sp > 0 ? 100.0 * sp_global_pos_diff_5mm / matched_sp : 0.0)
        << "%)");
    ATH_MSG_INFO(
        "============================================================");
    ATH_MSG_INFO("SPACEPOINT VARIANCE COMPARISON:");
    ATH_MSG_INFO(
        "  Spacepoints with >10% variance R difference: "
        << sp_variance_r_diff << " ("
        << (matched_sp > 0 ? 100.0 * sp_variance_r_diff / matched_sp : 0.0)
        << "%)");
    ATH_MSG_INFO(
        "  Spacepoints with >10% variance Z difference: "
        << sp_variance_z_diff << " ("
        << (matched_sp > 0 ? 100.0 * sp_variance_z_diff / matched_sp : 0.0)
        << "%)");
    ATH_MSG_INFO(
        "============================================================");


    m_nMonSp += t_n_sp;
    m_nRefSp += a_n_sp;
    m_nMatchedSp += matched_sp;
    m_nUnmatchedMonSp += unmatched_monitored_sp;
    m_nUnmatchedRefSp += unmatched_reference_sp;
    m_nSpPosDiff1mm += sp_global_pos_diff_1mm;
    m_nSpPosDiff5mm += sp_global_pos_diff_5mm;
    m_nSpVarRDiff += sp_variance_r_diff;
    m_nSpVarZDiff += sp_variance_z_diff;

    return StatusCode::SUCCESS;
}

StatusCode ActsClusterComparisonAlg::finalize()
{

    ATH_MSG_INFO("Validation summary: ");

    // Print cluster statistics
    ATH_MSG_INFO(
        "============================================================");
    ATH_MSG_INFO("PIXEL CLUSTER MATCHING STATISTICS: ");
    ATH_MSG_INFO("  Total unmatched clusters mon/ref: " << m_pix_unmatched_mon << " / " << m_pix_unmatched_ref);
    ATH_MSG_INFO("  Total matched clusters: " << m_matched_pixel);
    ATH_MSG_INFO("  Clusters with pos diff > 1 sigma: "
                 << m_pixel_pos_diff_1sig << " ("
                 << (m_matched_pixel.value() > 0
                         ? 100.0 * m_pixel_pos_diff_1sig.value() / m_matched_pixel.value()
                         : 0.0)
                 << "%)");
    ATH_MSG_INFO("  Clusters with pos diff > 0.5 sigma: "
                 << m_pixel_pos_diff_0p5sig << " ("
                 << (m_matched_pixel.value() > 0
                         ? 100.0 * m_pixel_pos_diff_0p5sig.value() / m_matched_pixel.value()
                         : 0.0)
                 << "%)");
    ATH_MSG_INFO("  Clusters with pos diff > 0.25 sigma: "
                 << m_pixel_pos_diff_0p25sig << " ("
                 << (m_matched_pixel.value() > 0
                         ? 100.0 * m_pixel_pos_diff_0p25sig.value() / m_matched_pixel.value()
                         : 0.0)
                 << "%)");
    ATH_MSG_INFO(
        "============================================================");
    ATH_MSG_INFO(
        "============================================================");
    ATH_MSG_INFO("STRIP CLUSTER MATCHING STATISTICS:");
    ATH_MSG_INFO("  Total unmatched clusters mon/ref: " << m_strip_unmatched_mon << " / " << m_strip_unmatched_ref);
    ATH_MSG_INFO("  Total matched clusters: " << m_matched_strip);
    ATH_MSG_INFO("  Clusters with pos diff > 1 sigma: "
                 << m_strip_pos_diff_1sig << " ("
                 << (m_matched_strip.value() > 0
                         ? 100.0 * m_strip_pos_diff_1sig.value() / m_matched_strip.value()
                         : 0.0)
                 << "%)");
    ATH_MSG_INFO("  Clusters with pos diff > 0.5 sigma: "
                 << m_strip_pos_diff_0p5sig << " ("
                 << (m_matched_strip.value() > 0
                         ? 100.0 * m_strip_pos_diff_0p5sig.value() / m_matched_strip.value()
                         : 0.0)
                 << "%)");
    ATH_MSG_INFO("  Clusters with pos diff > 0.25 sigma: "
                 << m_strip_pos_diff_0p25sig << " ("
                 << (m_matched_strip.value() > 0
                         ? 100.0 * m_strip_pos_diff_0p25sig.value() / m_matched_strip.value()
                         : 0.0)
                 << "%)");
    ATH_MSG_INFO(
        "============================================================");

    if(m_checkSpacepoints){
        ATH_MSG_INFO("============================================================");
        ATH_MSG_INFO("PIXEL SPACEPOINT MATCHING STATISTICS:");
        ATH_MSG_INFO("  Total monitored spacepoints: " << m_nMonSp);
        ATH_MSG_INFO("  Total reference spacepoints: " << m_nRefSp);
        ATH_MSG_INFO("  Matched spacepoints: " << m_nMatchedSp);
        ATH_MSG_INFO("  Unmatched monitored spacepoints: " << m_nUnmatchedMonSp);
        ATH_MSG_INFO("  Unmatched reference spacepoints: " << m_nUnmatchedRefSp);
        ATH_MSG_INFO("============================================================");
        ATH_MSG_INFO("SPACEPOINT POSITION COMPARISON:");
        ATH_MSG_INFO("  Spacepoints with global pos diff > 1 mm: "
                    << m_nSpPosDiff1mm << " ("
                    << (m_nMatchedSp.value() > 0 ? 100.0 * m_nSpPosDiff1mm.value() / m_nMatchedSp.value() : 0.0)
                    << "%)");
        ATH_MSG_INFO("  Spacepoints with global pos diff > 5 mm: "
                    << m_nSpPosDiff5mm << " ("
                    << (m_nMatchedSp.value() > 0 ? 100.0 * m_nSpPosDiff5mm.value() / m_nMatchedSp.value() : 0.0)
                    << "%)");
        ATH_MSG_INFO("============================================================");
        ATH_MSG_INFO("SPACEPOINT VARIANCE COMPARISON:");
        ATH_MSG_INFO("  Spacepoints with >10% variance R difference: "
                    << m_nSpVarRDiff << " ("
                    << (m_nMatchedSp.value() > 0 ? 100.0 * m_nSpVarRDiff.value() / m_nMatchedSp.value() : 0.0)
                    << "%)");
        ATH_MSG_INFO("  Spacepoints with >10% variance Z difference: "
                    << m_nSpVarZDiff << " ("
                    << (m_nMatchedSp.value() > 0 ? 100.0 * m_nSpVarZDiff.value() / m_nMatchedSp.value() : 0.0)
                    << "%)");
        ATH_MSG_INFO("============================================================");
    }

    return StatusCode::SUCCESS;
}

} // namespace ActsTrk
