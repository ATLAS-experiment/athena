/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
   */

#include <array>
#include <vector>

#include "FPGATrackSimSpacePointsTool.h"
#include "FPGATrackSimObjects/FPGATrackSimCluster.h"
#include "FPGATrackSimObjects/FPGATrackSimLogicalEventInputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "TH1.h"

FPGATrackSimSpacePointsTool::FPGATrackSimSpacePointsTool(const std::string &algname, const std::string &name, const IInterface *ifc)
    : base_class(algname, name, ifc)
{
    declareInterface<FPGATrackSimSpacePointsToolI>(this);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode FPGATrackSimSpacePointsTool::initialize()
{
    ATH_MSG_INFO("Using spacepoints default tool");
    if (m_duplicate) ATH_MSG_INFO("Duplicating spacepoint to layer on the other side of the stave");
    if (m_filter) ATH_MSG_INFO("Filtering out incomplete spacepoints");
    if (m_filterClose) ATH_MSG_INFO("Filtering out single hits close to spacepoints");

    m_spacepts_per_hit = new TH1I("spacepts_per_hit","",20,0,20);

    return StatusCode::SUCCESS;
}

StatusCode FPGATrackSimSpacePointsTool::finalize()
{
    ATH_MSG_INFO("Spacepoints report");
    ATH_MSG_INFO("-----------------------------------------");
    ATH_MSG_INFO("Strip Hits Input " << m_inputhits);
    ATH_MSG_INFO("Spacepoints Ouput " << m_spacepts);
    ATH_MSG_INFO("Filtered Hits " << m_filteredhits);
    ATH_MSG_INFO("Adjacent Eta Module SPs " << m_adjacent_eta_sp);
    ATH_MSG_INFO("Adjacent Phi Module SPs " << m_adjacent_phi_sp);
    ATH_MSG_INFO("Diagonal Module SPs " << m_diagonal_sp);
    ATH_MSG_INFO("Spacepoints per hit " << m_spacepts_per_hit->GetMean());
    ATH_MSG_INFO("-----------------------------------------");
    return StatusCode::SUCCESS;
}

StatusCode FPGATrackSimSpacePointsTool::DoSpacePoints(FPGATrackSimLogicalEventInputHeader &header, std::vector<FPGATrackSimCluster> &spacepoints)
{
    for (int i = 0; i < header.nTowers(); ++i)
    {
        FPGATrackSimTowerInputHeader &tower = *header.getTower(i);
        std::vector<FPGATrackSimHit> hits = tower.hits();
        if (hits.empty()) continue;

        m_inputhits += hits.size();
        ATH_CHECK(fillMaps(hits));
        ATH_CHECK(makeSpacePoints(tower,spacepoints));
    }

    return StatusCode::SUCCESS;
}


StatusCode FPGATrackSimSpacePointsTool::fillMaps(std::vector<FPGATrackSimHit>& hits)
{
    m_map.clear();
    m_pixel.clear();

    int strip_hits = 0;
    for (auto hit : hits) {

        if (hit.isPixel()) {
            // just save them so we can put them back
            m_pixel.push_back(hit);
            ATH_MSG_DEBUG("Pixel hit z = " << hit.getZ() << ", r = " << hit.getR() << ", phi = " << hit.getGPhi());
            continue;
        }
        strip_hits += 1;

        std::vector<int> module_desc(4);

        module_desc[0] = (int)hit.getDetectorZone();
        module_desc[1] = hit.getPhysLayer()/2; //div 2 gives layer as part of pair
        unsigned side = hit.getPhysLayer()%2; // even/odd layer
        module_desc[2] = hit.getPhiModule();
        module_desc[3] = hit.getEtaModule();

        if (side==0)
            m_map[module_desc].first.push_back(hit);
        else
            m_map[module_desc].second.push_back(hit);
    }

    ATH_MSG_DEBUG("Running spacepoints default tool over " << strip_hits << " strip hits.");

    return StatusCode::SUCCESS;
}

StatusCode FPGATrackSimSpacePointsTool::makeSpacePoints(FPGATrackSimTowerInputHeader &tower, std::vector<FPGATrackSimCluster> &spacepoints)
{

    // delete the old clusters, this will be replaced by the spacepoints
    tower.clearHits();

    for (auto entry : m_map)
    {
        // std::vector<int> const & module_desc = entry.first;
        std::vector<FPGATrackSimHit>& hits_inner = entry.second.first;
        std::vector<FPGATrackSimHit>& hits_outer = entry.second.second;

        for (auto hit_in : hits_inner) {
            int startsize =  spacepoints.size();
            bool foundPair = searchForMatch(hit_in,hits_outer,tower,spacepoints);

            if (!foundPair && !m_sameModulesOnly) {
                // search in +1 eta direction
                std::vector<int> nextmod = entry.first;
                nextmod[3]+=1; // increment eta module counter
                auto entry2 = m_map.find(nextmod);
                if (entry2!=m_map.end()) {
                    foundPair = searchForMatch(hit_in,entry2->second.second,tower,spacepoints);
                    if (foundPair) m_adjacent_eta_sp++;
                }
            }

            // Also seek in +1 phi direction.
            if (!foundPair && !m_sameModulesOnly) {
                // search in +1 eta direction
                std::vector<int> nextphimod = entry.first;
                nextphimod[2]+=1; // increment phi module counter
                auto entry3 = m_map.find(nextphimod);
                if (entry3!=m_map.end()) {
                    foundPair = searchForMatch(hit_in,entry3->second.second,tower,spacepoints);
                    if (foundPair) m_adjacent_phi_sp++;
                }
            }

            // If all else fails, seek in +1 eta, +1 phi direction.
            if (!foundPair && !m_sameModulesOnly) {
                // search in +1 eta direction
                std::vector<int> next2mod = entry.first;
                next2mod[3]+=1; // increment eta module counter
                next2mod[2]+=1; // increment phi module counter
                auto entry4 = m_map.find(next2mod);
                if (entry4!=m_map.end()) {
                    foundPair = searchForMatch(hit_in,entry4->second.second,tower,spacepoints);
                    if (foundPair) m_diagonal_sp++;
                }
            }

            if (!foundPair) {
                if (m_filter) m_filteredhits++;
                else {
                    tower.addHit(hit_in); // add unpaired hit_in
                    ATH_MSG_DEBUG("Unpaired hit z = " << hit_in.getZ() << ", r = " << hit_in.getR() << ", phi = " << hit_in.getGPhi() << ", phi module = " << hit_in.getPhiModule() << ", eta module = " << hit_in.getEtaModule());
                }
            }

            m_spacepts_per_hit->Fill(spacepoints.size()-startsize);
        }

        // add unpaired outer hits TODO use the hit type instead of another loop
        for (const FPGATrackSimHit& hit_out : hits_outer) {
            bool foundPair=false;
            for (const FPGATrackSimHit& hit_in : hits_inner) {
                if (abs(hit_in.getGPhi()-hit_out.getGPhi()) < m_phiwindow) {
                    foundPair=true;
                    break;
                }
            }

            // search in -1 eta direction
            std::vector<int> nextmod = entry.first;
            nextmod[3]-=1; // increment eta module counter
            auto entry2 = m_map.find(nextmod);
            if (entry2!=m_map.end()) {
                for (auto hit_in : entry2->second.first) {
                    if (abs(hit_in.getGPhi()-hit_out.getGPhi()) < m_phiwindow) {
                        foundPair=true;
                        m_adjacent_eta_sp++;
                        break;
                    }
                }
            }

            // Also seek in -1 phi direction.
            if (!foundPair) {
                // search in +1 eta direction
                std::vector<int> nextphimod = entry.first;
                nextphimod[2]-=1; // increment phi module counter
                auto entry3 = m_map.find(nextphimod);
                if (entry3!=m_map.end()) {
                    for (auto hit_in : entry3->second.first) {
                        if (abs(hit_in.getGPhi()-hit_out.getGPhi()) < m_phiwindow) {
                            foundPair=true;
                            m_adjacent_phi_sp++;
                            break;
                        }
                    }
                }
            }

            // If all else fails, seek in -1 eta, -1 phi direction.
            if (!foundPair) {
                // search in +1 eta direction
                std::vector<int> next2mod = entry.first;
                next2mod[3]-=1; // increment eta module counter
                next2mod[2]-=1; // increment phi module counter
                auto entry4 = m_map.find(next2mod);
                if (entry4!=m_map.end()) {
                    for (auto hit_in : entry4->second.first) {
                        if (abs(hit_in.getGPhi()-hit_out.getGPhi()) < m_phiwindow) {
                            foundPair=true;
                            m_diagonal_sp++;
                            break;
                        }
                    }
                }
            }

            if (!foundPair) {
                if (m_filter) m_filteredhits++;
                else {
                    tower.addHit(hit_out);
                    ATH_MSG_DEBUG("Unpaired hit z = " << hit_out.getZ() << ", r = " << hit_out.getR() << ", phi = " << hit_out.getGPhi() << ", phi module = " << hit_out.getPhiModule() << ", eta module = " << hit_out.getEtaModule());
                }
            }
        }
    }

    // add back the pixles
    for (const FPGATrackSimHit& hit: m_pixel) tower.addHit(hit);

    m_inputhits -= m_pixel.size(); // so count is just input strips
    m_spacepts += spacepoints.size();


    return StatusCode::SUCCESS;
}


bool FPGATrackSimSpacePointsTool::searchForMatch(FPGATrackSimHit& hit_in,std::vector<FPGATrackSimHit>& hits_outer,FPGATrackSimTowerInputHeader &tower, std::vector<FPGATrackSimCluster> &spacepoints)
{
    bool foundPair = false;
    for (const FPGATrackSimHit& hit_out : hits_outer)
    {
        // Too far apart to be from same track
        if (abs(hit_in.getGPhi()-hit_out.getGPhi()) < m_phiwindow) {
            addSpacePoints(hit_in,hit_out,tower,spacepoints);
            foundPair=true;
        }
    }
    return foundPair;
}

void FPGATrackSimSpacePointsTool::addSpacePoints(FPGATrackSimHit hit_in, FPGATrackSimHit hit_out ,FPGATrackSimTowerInputHeader &tower, std::vector<FPGATrackSimCluster> &spacepoints)
{
    // Make a spacepoint
    //------------------
    float x,y,z;
    calcPosition(hit_in, hit_out, x, y, z);

    float phi_window = abs(hit_in.getGPhi()-hit_out.getGPhi());

    // Let's print out the coords of the spacepoints we identify.
    float r = TMath::Sqrt(x*x + y*y);
    ATH_MSG_DEBUG("Spacepoint x = " << x << ", y = " << y << ", z = " << z << ", r = " << r);
    ATH_MSG_DEBUG("    Paired hit z = " << hit_in.getZ() << ", r = " << hit_in.getR() << ", phi = " << hit_in.getGPhi() << ", phi module = " << hit_in.getPhiModule() << ", eta module = " << hit_in.getEtaModule());
    ATH_MSG_DEBUG("    Paired hit z = " << hit_out.getZ() << ", r = " << hit_out.getR() << ", phi = " << hit_out.getGPhi() << ", phi module = " << hit_out.getPhiModule() << ", eta module = " << hit_out.getEtaModule());

    // We need to merge the truth information for the hits together in order
    // to use them in matrix generation.

    const FPGATrackSimMultiTruth truth_in = hit_in.getTruth();
    const FPGATrackSimMultiTruth truth_out = hit_out.getTruth();

    FPGATrackSimMultiTruth new_truth;
    new_truth.add(truth_in);
    new_truth.add(truth_out);
    //hit_in.setTruth(new_truth);

    //hit_in.setX(x);
    //hit_in.setY(y);
    //hit_in.setZ(z);

    // TODO this is probably no longer necessary, we should just overwrite the local coordinates
    // of both hit, I think?
    //hit_in.setPairedHit(hit_in);

    // This function now does everything (I think) including storing the correct
    // local coordinates for later retrieval.
    // Maybe it should return a new FPGATrackSimHit rather than modifying in place.
    hit_in.makeSpacepoint(x, y, z, phi_window, hit_out, new_truth);
    hit_in.setCluster2ID(hit_out.getCluster1ID());

    if (m_reduceCoordPrecision)
        reduceGlobalCoordPrecision(hit_in);

    // abusing hit type 'guessed' to be able to indentify it as spacepoint later on
    // Guessed is ambiguous with an actual guessed hit-- there is a spacepoint type which
    // should be used instead.
    //    hit_in.setHitType(HitType::guessed);
    //    hit_in.setHitType(HitType::spacepoint);
    tower.addHit(hit_in);

    if (m_duplicate) {
        hit_out.makeSpacepoint(x, y, z, phi_window, hit_in, new_truth);
        hit_out.setCluster2ID(hit_in.getCluster1ID());

        if (m_reduceCoordPrecision)
            reduceGlobalCoordPrecision(hit_out);

        tower.addHit(hit_out);
    }

    // push back a copy for monitoring
    FPGATrackSimCluster sp;
    sp.setClusterEquiv(hit_in);
    sp.push_backHitList(hit_in);
    sp.push_backHitList(hit_out);
    spacepoints.push_back(sp);

}



void FPGATrackSimSpacePointsTool::calcPosition(FPGATrackSimHit &hit_in, FPGATrackSimHit &hit_out, float &x, float &y, float &z)
{
    float phi_sp = (hit_in.getGPhi() + hit_out.getGPhi()) / 2.0;
    float r_sp = (hit_in.getR() + hit_out.getR()) / 2.0;;
    float z_sp = (hit_in.getZ() + hit_out.getZ()) / 2.0;;

    float delta_phi_local = (hit_in.getGPhi() - hit_out.getGPhi()) * r_sp; // distance in phi dir in mm

    if (hit_in.isBarrel())
    {
        static const float stereo_angle = 0.026; // barrel
        z_sp += delta_phi_local/tan(stereo_angle)/2.0;
    }
    else
    {
        static const float stereo_angle = 0.020; // endcap
        r_sp += delta_phi_local/tan(stereo_angle)/2.0;

        // don't let r_sp out of range of edge of module TODO fix hardcode
        // Allow for the bounds check to be disabled, since it may mess up a linear fit.
        if (m_boundsCheck) {
            float r_bounds[19] = {394.0, 415.5, 442.0, 472.4, 498.85, 521.45, 547.05, 566.65, 591.0, 621.8, 654.7, 683.9, 710.2, 739.4, 784.2, 838.8, 887.6, 937.7, 967.8};

            if (hit_in.getEtaModule()==hit_in.getEtaModule()) {
                float r_limited = std::max(r_sp,r_bounds[hit_in.getEtaModule()]);
                r_limited = std::min(r_sp,r_bounds[hit_in.getEtaModule()+1]);
                if (r_sp!=r_limited) {
                    ATH_MSG_WARNING("Spacepoint location not in module boundary: r_sp=" << r_sp
                            << " not in [" << r_bounds[hit_in.getEtaModule()] << "," << r_bounds[hit_in.getEtaModule()+1] << "]");
                    r_sp=r_limited;
                }
            } else if (hit_in.getEtaModule()==hit_in.getEtaModule()+1) {
                float window = 3; //mm, max possible distance between hits from same track with reasonable incline to module
                float r_limited = std::max(r_sp,r_bounds[hit_in.getEtaModule()+1]-window);
                r_limited = std::min(r_sp,r_bounds[hit_in.getEtaModule()+1]+window);
                if (r_sp!=r_limited) {
                    ATH_MSG_WARNING("Crossing spacepoint location too far from module boundary: r_sp=" << r_sp
                            << " not in [" << r_bounds[hit_in.getEtaModule()+1]-window << "," << r_bounds[hit_in.getEtaModule()+1]+window  << "]");
                    r_sp=r_limited;
                }
            }
        }
    }

    // insert new values
    x = r_sp*cos(phi_sp);
    y = r_sp*sin(phi_sp);
    z = z_sp;
}

void FPGATrackSimSpacePointsTool::reduceGlobalCoordPrecision(FPGATrackSimHit &hit) const {
    float pos[3] = { hit.getR(), hit.getGPhi(), hit.getZ() };

    pos[0] = std::trunc(pos[0] / m_coordRPrecision) * m_coordRPrecision;
    pos[1] = std::trunc(pos[1] / m_coordPhiPrecision) * m_coordPhiPrecision;
    pos[2] = std::trunc(pos[2] / m_coordZPrecision) * m_coordZPrecision;

    hit.setX(pos[0] * std::cos(pos[1]));
    hit.setY(pos[0] * std::sin(pos[1]));
    hit.setZ(pos[2]);
}
