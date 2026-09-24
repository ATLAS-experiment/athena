/*
 Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef XAOD_STANDALONE
#include <FourMomUtils/xAODP4Helpers.h>
#include <IsolationSelection/IsoVariableHelper.h>
#include <IsolationSelection/IsolationWP.h>
#include <IsolationSelection/TestMacroHelpers.h>
#include <xAODEgamma/EgammaxAODHelpers.h>
#include <xAODEgamma/Electron.h>
#include <xAODMuon/Muon.h>
#include <xAODPrimitives/IsolationHelpers.h>
#include <xAODPrimitives/tools/getIsolationAccessor.h>
#include <xAODPrimitives/tools/getIsolationCorrectionAccessor.h>
namespace CP {

    static const FloatAccessor acc_assocClustEta{IsolationCloseByCorrectionTool::caloDecors()[0]};
    static const FloatAccessor acc_assocClustPhi{IsolationCloseByCorrectionTool::caloDecors()[1]};
    static const FloatAccessor acc_assocClustEne{IsolationCloseByCorrectionTool::caloDecors()[2]};
    static const CharAccessor acc_assocCaloIsDec{IsolationCloseByCorrectionTool::caloDecors()[3]};

    static const FloatAccessor acc_assocPflowEta{IsolationCloseByCorrectionTool::pflowDecors()[0]};
    static const FloatAccessor acc_assocPflowPhi{IsolationCloseByCorrectionTool::pflowDecors()[1]};
    static const FloatAccessor acc_assocPflowEne{IsolationCloseByCorrectionTool::pflowDecors()[2]};
    static const CharAccessor acc_assocPflowIsDec{IsolationCloseByCorrectionTool::pflowDecors()[3]};

    //############################################################################
    //                      IsoCorrectionTestHelper
    //############################################################################
    bool IsoCorrectionTestHelper::init() { return true; }
    bool IsoCorrectionTestHelper::fill(const EventContext&) { return true; }
    void IsoCorrectionTestHelper::SetFlowElements(const PflowSet& flows) {m_flows = flows;}
    IsoCorrectionTestHelper::IsoCorrectionTestHelper(MuonVal::MuonTesterTree& outTree, const std::string& ContainerName,
                                                     const std::vector<std::unique_ptr<IsolationWP>>& WPs) :
        MuonTesterBranch(outTree, ContainerName),
        m_cont_name{ContainerName} {
        // Retrieve the isolation accessors directly from the WP
        if (!WPs.empty()){
          const auto & W = WPs.front(); // Assume only 1 WP
            for (const auto& C : W->conditions()) {
                for (unsigned int t = 0; t < C->num_types(); ++t) {
                    const IsoType iso_type = C->type(t);
                    bool add = std::ranges::none_of(m_iso_branches, 
                      [iso_type](const IsolationBranches& known) {
                        return known.Accessor->isotype() == iso_type;
                    });
                    if (add) m_iso_branches.emplace_back(*this, iso_type, "");
                }
            }
        }
    }
    void IsoCorrectionTestHelper::SetClusters(const ClusterSet& clusters) { m_clusters = clusters; }
    void IsoCorrectionTestHelper::SetSelectionDecorator(const std::string& acc) {
        m_acc_used_for_corr = std::make_optional<CharAccessor>(acc);
    }
    void IsoCorrectionTestHelper::SetIsolationDecorator(const std::string& acc) {
        m_acc_passDefault = std::make_optional<CharAccessor>(acc);
    }
    void IsoCorrectionTestHelper::SetUpdatedIsoDecorator(const std::string& acc) {
        m_acc_passCorrected = std::make_optional<CharAccessor>(acc);
    }
    void IsoCorrectionTestHelper::SetBackupPreFix(const std::string& prefix) {
        for (IsolationBranches& branch : m_iso_branches) {
            branch.Accessor = std::make_unique<IsoVariableHelper>(branch.Accessor->isotype(), prefix);
        }
    }

    StatusCode IsoCorrectionTestHelper::Fill(const xAOD::IParticleContainer* Particles) {
        if (!Particles) {
            Error("IsoCorrectionTestHelper::Fill()", "No particles given");
            return StatusCode::FAILURE;
        }

        for (const xAOD::IParticle* object : *Particles) {
            if (m_acc_used_for_corr != std::nullopt && (!(*m_acc_used_for_corr).isAvailable(*object) || !(*m_acc_used_for_corr)(*object)))
                continue;
            m_pt.push_back(object->pt());
            m_eta.push_back(object->eta());
            m_phi.push_back(object->phi());
            m_e.push_back(object->e());
            m_Q.push_back(Charge(object));
            for (IsolationBranches& branch : m_iso_branches) {
                if (!FillIsolationBranches(object, branch.Accessor, branch.original_cones, branch.corrected_cones).isSuccess()) {
                    Error("IsoCorrectionTestHelper()", "Failed to fill isolation");
                    return StatusCode::FAILURE;
                }
            }
            if (m_acc_passDefault != std::nullopt && !(*m_acc_passDefault).isAvailable(*object)) {
                Error("IsoCorrectionTestHelper()", "It has not been stored whether the particle passes the default isolation");
                return StatusCode::FAILURE;
            } else
                m_orig_passIso.push_back(m_acc_passDefault != std::nullopt ? (*m_acc_passDefault)(*object) : false);

            if (m_acc_passCorrected != std::nullopt && !(*m_acc_passCorrected).isAvailable(*object)) {
                Error("IsoCorrectionTestHelper()", "It has not been stored whether the particle passes the corrected isolation.");
                return StatusCode::FAILURE;
            } else
                m_corr_passIso.push_back(m_acc_passCorrected != std::nullopt ? (*m_acc_passCorrected)(*object) : false);
            /// Fill the momenta of the associated tracks and clusters
            if (object->type() != xAOD::Type::ObjectType::Electron && object->type() != xAOD::Type::ObjectType::Muon) continue;

            const xAOD::TrackParticle* assoc_track = nullptr;
            CaloClusterPtr assoc_cluster{};
            FlowElementPtr assoc_flow{};
            if (object->type() == xAOD::Type::ObjectType::Muon) {
                const xAOD::Muon* mu = static_cast<const xAOD::Muon*>(object);
                assoc_track = mu->trackParticle(xAOD::Muon::TrackParticleType::InnerDetectorTrackParticle);
                if (m_clusters.empty())
                    assoc_cluster = mu->cluster();
                else {
                    assoc_cluster = (*std::min_element(m_clusters.begin(), m_clusters.end(),[mu](const CaloClusterPtr& a, const
                    CaloClusterPtr& b){
                        return xAOD::P4Helpers::deltaR2(a, mu) < xAOD::P4Helpers::deltaR2(b, mu);
                    }));
                }
            } else if (object->type() == xAOD::Type::ObjectType::Electron) {
                const xAOD::Electron* el = static_cast<const xAOD::Electron*>(object);
                assoc_track = xAOD::EgammaHelpers::getOriginalTrackParticle(el);
                assoc_cluster = el->caloCluster(0);
            }
            if (!m_flows.empty()) {
                assoc_flow = (*std::min_element(m_flows.begin(), m_flows.end(),[object](const FlowElementPtr& a, const
                    FlowElementPtr& b){
                        return xAOD::P4Helpers::deltaR2(a, object) < xAOD::P4Helpers::deltaR2(b, object);
                    }));
            }
            m_assoc_track_pt.push_back(assoc_track ? assoc_track->pt() : FLT_MAX);

            if (acc_assocCaloIsDec.isAvailable(*object) && acc_assocCaloIsDec(*object)) {
                m_assoc_cluster_et.push_back(acc_assocClustEne(*object));
                m_assoc_cluster_eta.push_back(acc_assocClustEta(*object));
                m_assoc_cluster_phi.push_back(acc_assocClustPhi(*object));
            } else if (assoc_cluster) {
                m_assoc_cluster_et.push_back(IsolationCloseByCorrectionTool::clusterEtMinusTile(assoc_cluster));
                m_assoc_cluster_eta.push_back(assoc_cluster->eta());
                m_assoc_cluster_phi.push_back(assoc_cluster->phi());
            } else {
                m_assoc_cluster_et += FLT_MAX;
                m_assoc_cluster_eta += FLT_MAX;
                m_assoc_cluster_phi += FLT_MAX;
            }            
            if (acc_assocPflowIsDec.isAvailable(*object) && acc_assocPflowIsDec(*object)) {
                m_assoc_pflow_et.push_back( acc_assocPflowEne(*object));
                m_assoc_pflow_eta.push_back(acc_assocPflowEta(*object));
                m_assoc_pflow_phi.push_back(acc_assocPflowPhi(*object));
            } else if (assoc_flow) {
                m_assoc_pflow_et.push_back(assoc_flow->e() * assoc_flow.weight);
                m_assoc_pflow_eta.push_back(assoc_flow->eta());
                m_assoc_pflow_phi.push_back(assoc_flow->phi());
            } else {
                m_assoc_pflow_et += FLT_MAX;
                m_assoc_pflow_eta += FLT_MAX;
                m_assoc_pflow_phi += FLT_MAX;
            }        
        }
        return StatusCode::SUCCESS;
    }
    float IsoCorrectionTestHelper::Charge(const xAOD::IParticle* P) const {
        static const FloatAccessor acc_charge("charge");
        if (!acc_charge.isAvailable(*P))
            return 0;
        else
            return acc_charge(*P);
    }

    StatusCode IsoCorrectionTestHelper::FillIsolationBranches(const xAOD::IParticle* P, const IsoHelperPtr& Acc,
                                                              MuonVal::VectorBranch<float>& Original, MuonVal::VectorBranch<float>& Corrected) {
        if (!Acc) return StatusCode::SUCCESS;
        float IsoValue{-1.};
        if (Acc->getOriginalIsolation(P, IsoValue).code() != CorrectionCode::Ok) return StatusCode::FAILURE;
        Original.push_back(IsoValue);
        if (Acc->getIsolation(P, IsoValue).code() != CorrectionCode::Ok) return StatusCode::FAILURE;
        Corrected.push_back(IsoValue);
        return StatusCode::SUCCESS;
    }
}  // namespace CP
#endif
