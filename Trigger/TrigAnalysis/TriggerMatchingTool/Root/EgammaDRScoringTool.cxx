/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#include "TriggerMatchingTool/EgammaDRScoringTool.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "xAODBase/IParticle.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODEgamma/Egamma.h"
#include "AthContainers/ConstAccessor.h"
#include <sstream>

// Anonymous namespace for helper functions
namespace {
    float dr(const xAOD::CaloCluster &lhs, const xAOD::CaloCluster &rhs)
    {
        return xAOD::P4Helpers::deltaR(lhs.etaBE(2), lhs.phiBE(2), rhs.etaBE(2), rhs.phiBE(2));
    }
    float dr(const xAOD::IParticle &online, const xAOD::CaloCluster &offline, bool useDecor = true)
    {
        static const SG::ConstAccessor<float> cl_eta2Acc ("cl_eta2");
        static const SG::ConstAccessor<float> cl_phi2Acc ("cl_phi2");
        double lhs_eta = useDecor ? cl_eta2Acc(online) : online.eta();
        double lhs_phi = useDecor ? cl_phi2Acc(online) : online.phi();
        double rhs_eta = offline.etaBE(2);
        double rhs_phi = offline.phiBE(2);

        return xAOD::P4Helpers::deltaR(lhs_eta, lhs_phi, rhs_eta, rhs_phi);
    }
}

namespace Trig {
    EgammaDRScoringTool::EgammaDRScoringTool(const std::string &name) :
        asg::AsgTool(name)
    {
        declareProperty("UseClusterDecorator", m_useClusterDecorator = true,
        "Use the decorated cluster values instead of the link to the cluster.");
    }

    StatusCode EgammaDRScoringTool::initialize()
    {
        return StatusCode::SUCCESS;
    }

    float EgammaDRScoringTool::score(
            const xAOD::IParticle &online,
            const xAOD::IParticle &offline) const
    {
        switch(online.type())
        {
            case xAOD::Type::CaloCluster:
                return dr(
                        dynamic_cast<const xAOD::CaloCluster &>(online),
                        dynamic_cast<const xAOD::CaloCluster &>(offline));
            case xAOD::Type::Electron:
                if (m_useClusterDecorator)
                {
                    return dr(online,
                             *dynamic_cast<const xAOD::Egamma &>(offline).caloCluster(), true);
                } else {
                    return dr(
                            *dynamic_cast<const xAOD::Egamma &>(online).caloCluster(),
                            *dynamic_cast<const xAOD::Egamma &>(offline).caloCluster());
                }
            case xAOD::Type::Photon:
                return dr(
                        *dynamic_cast<const xAOD::Egamma &>(online).caloCluster(),
                        *dynamic_cast<const xAOD::Egamma &>(offline).caloCluster());
            default:
            {
                std::string msg =  "Not an egamma type: " + std::to_string(online.type());
                throw std::runtime_error(msg);
            }
        }
      
    }
} //> end namespace Trig
