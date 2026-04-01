/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloCornerRingsBuilder.h"

#include "AthenaKernel/errorcheck.h"

// Cell includes:
#include "CaloUtils/CaloCellList.h"
#include "CaloGeoHelpers/CaloSampling.h"
#include "CaloEvent/CaloCellContainer.h"

// RingSet includes:
#include "xAODCaloRings/RingSetContainer.h"

// CaloRings includes:
#include "xAODCaloRings/CaloRingsContainer.h"

// Ringer Conf include:
#include "xAODCaloRings/RingSetConf.h"

// Other xAOD includes:
#include "xAODBase/IParticle.h"
#include "xAODCaloEvent/CaloCluster.h"

// STL
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <sstream>
#include <span>

namespace Ringer
{

    CaloCornerRingsBuilder::CaloCornerRingsBuilder(const std::string &type,
                                                   const std::string &name,
                                                   const ::IInterface *parent)
        : CaloRingsBuilder(type, name, parent)
    {

        declareInterface<ICaloRingsBuilder>(this);
    }

    CaloCornerRingsBuilder::~CaloCornerRingsBuilder() = default;

    StatusCode CaloCornerRingsBuilder::execute(const xAOD::CaloCluster &cluster,
                                               ElementLink<xAOD::CaloRingsContainer> &clRings)
    {
        double et(0.);
        const double eta2 = std::fabs(cluster.etaBE(2));
        const double energy = cluster.e();
        if (eta2 < 999.)
        {
            const double cosheta = std::cosh(eta2);
            et = (cosheta != 0.) ? energy / cosheta : 0.;
        }
        if (et > m_minEnergy)
        {
            // This now correctly calls the CaloCornerRingsBuilder::executeTemp
            return executeTemp(cluster, clRings);
        }
        else
        {
            ATH_MSG_DEBUG("Skipping cluster with low energy (" << et << " MeV) .");
            return StatusCode::SUCCESS;
        }
    }

    StatusCode CaloCornerRingsBuilder::execute(const xAOD::IParticle &particle,
                                               ElementLink<xAOD::CaloRingsContainer> &clRings)
    {
        double et = particle.pt();
        if (et > m_minEnergy)
        {
            // This now correctly calls the CaloCornerRingsBuilder::executeTemp
            return executeTemp(particle, clRings);
        }
        else
        {
            ATH_MSG_DEBUG("Skipping particle with low energy (" << et << " MeV) .");
            return StatusCode::SUCCESS;
        }
    }

    template <typename T>
    StatusCode CaloCornerRingsBuilder::executeTemp(
        const T &input,
        ElementLink<xAOD::CaloRingsContainer> &clRingsLink)
    {

        ATH_MSG_DEBUG("Entering executeTemp with input eta=" << input.eta()
                                                             << " phi=" << input.phi());

        xAOD::CaloRings *clRings = new xAOD::CaloRings();
        m_crCont->push_back(clRings);

        clRingsLink.toContainedElement(*m_crCont, clRings);

        if (!m_useShowShapeBarycenter)
        {
            m_lastValidSeed = AtlasGeoPoint(input.eta(), input.phi());
            ATH_MSG_DEBUG("Using input as seed eta=" << input.eta()
                                                     << " phi=" << input.phi());
        }

        for (const xAOD::RingSetConf::RawConf &rawConf : m_rsRawConfCol)
        {

            xAOD::RingSet *rs = new xAOD::RingSet(rawConf.nRings);
            m_rsCont->push_back(rs);

            unsigned int nSubRings = rawConf.nRings / 5;

            ATH_MSG_DEBUG("Creating RingSet with total rings=" << rawConf.nRings
                                                               << " subrings=" << nSubRings);

            std::vector<AtlasGeoPoint> seeds;
            seeds.reserve(5);

            AtlasGeoPoint centralSeed;

            CHECK(getRingSetSeed(rawConf, input, centralSeed));

            ATH_MSG_DEBUG("Central seed eta=" << centralSeed.eta()
                                              << " phi=" << centralSeed.phi());

            seeds.push_back(centralSeed);

            CHECK(getCornerRingsSeeds(rawConf, centralSeed, seeds));

            for (size_t i = 0; i < seeds.size(); ++i)
            {
                const AtlasGeoPoint &seed = seeds[i];
                const unsigned int offset = i * nSubRings;

                CHECK(buildRingSet(rawConf, seed, rs, offset, nSubRings));
            }

            ElementLink<xAOD::RingSetContainer> rsEL(rs, *m_rsCont);
            clRings->addRingSetEL(rsEL);
        }

        if (msgLevel() <= MSG::DEBUG)
        {
            std::ostringstream str;
            clRings->print(str);
            ATH_MSG_DEBUG(str.str());
        }

        return StatusCode::SUCCESS;
    }

    StatusCode CaloCornerRingsBuilder::getCornerRingsSeeds(
        const xAOD::RingSetConf::RawConf &rawConf,
        const AtlasGeoPoint &seed,
        std::vector<AtlasGeoPoint> &cornerSeeds)
    {

        const float deltaEta = m_cornerShift * rawConf.etaWidth;
        const float deltaPhi = m_cornerShift * rawConf.phiWidth;

        ATH_MSG_DEBUG("Corner seed shifts: deltaEta=" << deltaEta
                                                      << " deltaPhi=" << deltaPhi);

        AtlasGeoPoint top_right, top_left, bottom_right, bottom_left;

        top_right.setEta(seed.eta() + deltaEta);
        top_right.setPhi(seed.phi() + deltaPhi);

        top_left.setEta(seed.eta() + deltaEta);
        top_left.setPhi(seed.phi() - deltaPhi);

        bottom_right.setEta(seed.eta() - deltaEta);
        bottom_right.setPhi(seed.phi() + deltaPhi);

        bottom_left.setEta(seed.eta() - deltaEta);
        bottom_left.setPhi(seed.phi() - deltaPhi);

        ATH_MSG_DEBUG("Corner seeds:");
        ATH_MSG_DEBUG("top_right eta=" << top_right.eta() << " phi=" << top_right.phi());
        ATH_MSG_DEBUG("top_left eta=" << top_left.eta() << " phi=" << top_left.phi());
        ATH_MSG_DEBUG("bottom_right eta=" << bottom_right.eta() << " phi=" << bottom_right.phi());
        ATH_MSG_DEBUG("bottom_left eta=" << bottom_left.eta() << " phi=" << bottom_left.phi());

        cornerSeeds.push_back(top_right);
        cornerSeeds.push_back(top_left);
        cornerSeeds.push_back(bottom_left);
        cornerSeeds.push_back(bottom_right);

        return StatusCode::SUCCESS;
    }

    StatusCode CaloCornerRingsBuilder::buildRingSet(
        const xAOD::RingSetConf::RawConf &rawConf,
        const AtlasGeoPoint &seed,
        xAOD::RingSet *rs,
        const unsigned int offset,
        const unsigned int nSubRings)
    {

        SG::ReadHandle<CaloCellContainer> cellsCont(m_cellsContName);
        ATH_CHECK(cellsCont.isValid());

        SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloMgrKey};
        ATH_CHECK(caloMgrHandle.isValid());

        const CaloDetDescrManager *caloMgr = *caloMgrHandle;
        CaloCellList cells(caloMgr, cellsCont.ptr());

        for (const int layer : rawConf.layers)
        {

            ATH_MSG_DEBUG("Selecting cells in layer " << layer);

            cells.select(seed.eta(), seed.phi(), m_cellMaxDEtaDist, m_cellMaxDPhiDist, layer);

            for (const CaloCell *cell : cells)
            {

                unsigned int ringNumber(0);

                const float deltaEta = std::abs((cell->eta() - seed.eta())) / rawConf.etaWidth;
                const float deltaPhi = std::abs(CaloPhiRange::diff(cell->phi(), seed.phi())) / rawConf.phiWidth;

                const float deltaGreater = std::max(deltaEta, deltaPhi);

                ringNumber = static_cast<unsigned int>(std::floor(deltaGreater + .5));

                if (ringNumber < nSubRings)
                {
                    ringNumber += offset;

                    float energyToAdd = 0;

                    if (m_doTransverseEnergy)
                    {
                        energyToAdd = cell->energy() / std::cosh(cell->eta());
                    }
                    else
                    {
                        energyToAdd = cell->energy();
                    }

                    rs->at(ringNumber) += energyToAdd;

                    ATH_MSG_DEBUG("New ring energy=" << rs->at(ringNumber));
                }
            }
        }

        return StatusCode::SUCCESS;
    }
    // =====================================================================================

} // namespace Ringer