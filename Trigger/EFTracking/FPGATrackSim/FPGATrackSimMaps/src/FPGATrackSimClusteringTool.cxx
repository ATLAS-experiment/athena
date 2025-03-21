/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
   */

#include "FPGATrackSimClusteringTool.h"
#include "FPGATrackSimObjects/FPGATrackSimMultiTruth.h"
#include "FPGATrackSimObjects/FPGATrackSimConstants.h"
#include "CxxUtils/trapping_fp.h"
#include <algorithm>
#include <cmath>


namespace{
    //For deciding eta || phi columns in modules
    constexpr unsigned int ETA = 1;
    constexpr unsigned int PHI = 0;
}

FPGATrackSimClusteringTool::FPGATrackSimClusteringTool(const std::string& algname, const std::string &name, const IInterface *ifc) :
    base_class(algname, name, ifc)
{
}


StatusCode FPGATrackSimClusteringTool::DoClustering(FPGATrackSimLogicalEventInputHeader &header, std::vector<FPGATrackSimCluster> &clusters) const
{
    for (int i = 0; i<header.nTowers(); i++)
    {
        // Retreive the hits from the tower
        FPGATrackSimTowerInputHeader& tower = *header.getTower(i);
        std::vector<FPGATrackSimHit> hits = tower.hits();

        std::vector<std::vector<FPGATrackSimHit>> hitsPerModule;
        std::vector<FPGATrackSimCluster> towerClusters;

        if (m_reduceCoordPrecision) {
            for (auto &hit : hits)
                reduceGlobalCoordPrecision(hit);
        }

        splitAndSortHits(hits, hitsPerModule);
        SortedClustering(hitsPerModule, towerClusters);
        normaliseClusters(towerClusters);

        //remove the old hits from the tower...
        tower.clearHits();
        tower.reserveHits(towerClusters.size());
        clusters.clear();
        clusters.reserve(towerClusters.size());
        if(i > 1)
            ATH_MSG_WARNING("more than one tower, m_clusters is only going to contain those from the last one");

        unsigned cluster_count = 0;
        unsigned int pixelCounter =  0;
        unsigned int stripCounter = 0;
        for ( auto &cluster: towerClusters){
            if (m_reduceCoordPrecision)
                reduceGlobalCoordPrecision(cluster);

            FPGATrackSimHit cluster_as_FPGATrackSimhit = cluster.getClusterEquiv();
            cluster_as_FPGATrackSimhit.setHitType(HitType::clustered);
            cluster_as_FPGATrackSimhit.setParentageMask(cluster_count); // making use of unused m_parentageMask to keep track of cluster index

            if(cluster_as_FPGATrackSimhit.getDetType() == SiliconTech::pixel)
            {
              cluster_as_FPGATrackSimhit.setCluster1ID(pixelCounter);
              pixelCounter++;
            }
            else if(cluster_as_FPGATrackSimhit.getDetType() == SiliconTech::strip)
            {
              cluster_as_FPGATrackSimhit.setCluster1ID(stripCounter);
              stripCounter++;
            }
            tower.addHit(cluster_as_FPGATrackSimhit);

            //send back a copy for monitoring and to check when writing out hits in each road
            clusters.push_back(cluster);
            cluster_count++;
        }
    }
    ATH_MSG_DEBUG("Produced "<< clusters.size()<< " clusters");
    return StatusCode::SUCCESS;
}

//Attempt to implement clustering using FPGATrackSim objects.
void FPGATrackSimClusteringTool::SortedClustering(const std::vector<std::vector<FPGATrackSimHit> >& sorted_hits, std::vector<FPGATrackSimCluster> &clusters) const {
    std::vector<FPGATrackSimCluster> moduleClusters;
    //Loop over the sorted modules that we have
    for( auto& moduleHits:sorted_hits){
        //Make the clusters for this module
        Clustering(moduleHits, moduleClusters);
        //Put these clusters into the output list
        clusters.insert(clusters.end(), moduleClusters.begin(), moduleClusters.end());
        //Clear the vector or this will get messy
        moduleClusters.clear();
    }
}

void FPGATrackSimClusteringTool::Clustering(std::vector<FPGATrackSimHit> moduleHits, std::vector<FPGATrackSimCluster> &moduleClusters) const {
    std::vector<FPGATrackSimCluster> tempClusters;
    FPGATrackSimHit clusterEquiv;
    bool newCluster, newHit;

    //To hold the current cluster vars for comparison
    //loop over the hits that we have been passed for this module
    for( auto& hit: moduleHits){
        bool is_clustered_hit = false;

        //Loop over the clusters we have already made, check if this hit should be added to them?
        for( auto& cluster: tempClusters){
            if(hit.isPixel()){
                if (FPGATrackSimCLUSTERING::updatePixelCluster(cluster, hit, false, m_digitalClustering))
                    is_clustered_hit = true;
            }
            if(hit.isStrip()){
                if (FPGATrackSimCLUSTERING::updateStripCluster(cluster, hit, false, m_digitalClustering))
                    is_clustered_hit = true;
            }
        }

        //If it is the first hit or a not clustered hit, then start a new cluster and add it to the output vector
        if((is_clustered_hit==0) or (tempClusters.size()==0)){
            FPGATrackSimCluster cluster;
            if(hit.isPixel()){
                // No need to check the return code here
                FPGATrackSimCLUSTERING::updatePixelCluster(cluster, hit, true, m_digitalClustering);
            } else if(hit.isStrip()){
                FPGATrackSimCLUSTERING::updateStripCluster(cluster, hit, true, m_digitalClustering);
            }
            //Put this cluster into the output hits. Will update it in place.
            tempClusters.push_back(cluster);
        }
    }

    // Merge overlapping clusters
    for (auto& cluster : tempClusters) {
        newCluster = true;

        for (auto& finalCluster : moduleClusters) {
            int cPhi = cluster.getClusterEquiv().getPhiIndex();
            int cPhiWidth = cluster.getClusterEquiv().getPhiWidth();
            int cEta = cluster.getClusterEquiv().getEtaIndex();
            int cEtaWidth = cluster.getClusterEquiv().getEtaWidth();
            int fCPhi = finalCluster.getClusterEquiv().getPhiIndex();
            int fCPhiWidth = finalCluster.getClusterEquiv().getPhiWidth();
            int fCEta = finalCluster.getClusterEquiv().getEtaIndex();
            int fCEtaWidth = finalCluster.getClusterEquiv().getEtaWidth();

            // check for overlap in phi
            if ((fCPhi > cPhi + cPhiWidth - 1) ||
                    (cPhi > fCPhi + fCPhiWidth - 1))
                continue;

            // check for overlap in eta
            if ((fCEta > cEta + cEtaWidth - 1) ||
                    (cEta > fCEta + fCEtaWidth - 1))
                continue;

            // remaining clusters are overlapping, check if clusters share hits
            unsigned int sharedhits = 0;
            for (auto & hit : cluster.getHitList()) {
                newHit = true;
                for (auto & finalHit : finalCluster.getHitList()) {
                    if (hit.getEtaIndex() == finalHit.getEtaIndex() &&
                            hit.getPhiIndex() == finalHit.getPhiIndex())
                        newHit = false;
                }
                if (!newHit) {
                    sharedhits++;
                }
            }

            if (sharedhits == 0)
                continue;

            // Merge the clusters
            newCluster = false;

            clusterEquiv = finalCluster.getClusterEquiv();

            // set new phi & phi width
            if (cPhi < fCPhi) {
                clusterEquiv.setPhiIndex(cPhi);
                if (cPhi + cPhiWidth < fCPhi + fCPhiWidth)
                    clusterEquiv.setPhiWidth(fCPhiWidth + (fCPhi - cPhi));
                else
                    clusterEquiv.setPhiWidth(cPhiWidth);
            } else {
                clusterEquiv.setPhiIndex(fCPhi);
                if (!(cPhi + cPhiWidth < fCPhi + fCPhiWidth))
                    clusterEquiv.setPhiWidth(cPhiWidth + (cPhi - fCPhi));
                else
                    clusterEquiv.setPhiWidth(fCPhiWidth);
            }

            // set new eta & eta width
            if (cEta < fCEta) {
                clusterEquiv.setEtaIndex(cEta);
                if (cEta + cEtaWidth < fCEta + fCEtaWidth)
                    clusterEquiv.setEtaWidth(fCEtaWidth + (fCEta - cEta));
                else
                    clusterEquiv.setEtaWidth(cEtaWidth);
            } else {
                clusterEquiv.setEtaIndex(fCEta);
                if (!(cEta + cEtaWidth < fCEta + fCEtaWidth))
                    clusterEquiv.setEtaWidth(cEtaWidth + (cEta - fCEta));
                else
                    clusterEquiv.setEtaWidth(fCEtaWidth);
            }

            finalCluster.setClusterEquiv(clusterEquiv);

            for (auto & hit : cluster.getHitList()) {
                newHit = true;
                for (auto & finalHit : finalCluster.getHitList()) {
                    if (hit.getEtaIndex() == finalHit.getEtaIndex() &&
                            hit.getPhiIndex() == finalHit.getPhiIndex())
                        newHit = false;
                }
                if (newHit) {
                    CXXUTILS_TRAPPING_FP;
                    clusterEquiv = finalCluster.getClusterEquiv();
                    float xOld = clusterEquiv.getX();
                    float yOld = clusterEquiv.getY();
                    float zOld = clusterEquiv.getZ();
                    float xPhiOld = clusterEquiv.getPhiCoord();
                    float xEtaOld = clusterEquiv.getEtaCoord();
                    float cPhiOld = clusterEquiv.getCentroidPhiIndex();
                    float cEtaOld = clusterEquiv.getCentroidEtaIndex();
                    float xNew = hit.getX();
                    float yNew = hit.getY();
                    float zNew = hit.getZ();
                    float xPhiNew = hit.getPhiCoord();
                    float xEtaNew = hit.getEtaCoord();
                    float cPhiNew = hit.getPhiIndex() + 0.5;
                    float cEtaNew = hit.getEtaIndex() + 0.5;
                    int tot = clusterEquiv.getToT();
                    int totNew = hit.getToT();
                    if (m_digitalClustering) {
                        // n+1 because that is old + new now
                        int n = finalCluster.getHitList().size();
                        clusterEquiv.setX((xOld*n + xNew) / (n+1));
                        clusterEquiv.setY((yOld*n + yNew) / (n+1));
                        clusterEquiv.setZ((zOld*n + zNew) / (n+1));
                        clusterEquiv.setPhiCoord((xPhiOld*n + xPhiNew) / (n+1));
                        clusterEquiv.setEtaCoord((xEtaOld*n + xEtaNew) / (n+1));
                        clusterEquiv.setCentroidPhiIndex((cPhiOld*n + cPhiNew) / (n+1));
                        clusterEquiv.setCentroidEtaIndex((cEtaOld*n + cEtaNew) / (n+1));
                    } else {
                        clusterEquiv.setX((xOld*tot + xNew*totNew) / (tot+totNew));
                        clusterEquiv.setY((yOld*tot + yNew*totNew) / (tot+totNew));
                        clusterEquiv.setZ((zOld*tot + zNew*totNew) / (tot+totNew));
                        clusterEquiv.setPhiCoord((xPhiOld*tot + xPhiNew*totNew) / (tot+totNew));
                        clusterEquiv.setEtaCoord((xEtaOld*tot + xEtaNew*totNew) / (tot+totNew));
                        clusterEquiv.setCentroidPhiIndex((cPhiOld*tot + cPhiNew*totNew) / (tot+totNew));
                        clusterEquiv.setCentroidEtaIndex((cEtaOld*tot + cEtaNew*totNew) / (tot+totNew));
                    }
                    clusterEquiv.setToT(tot + totNew);
                    finalCluster.setClusterEquiv(clusterEquiv);
                    finalCluster.push_backHitList(hit);
                }
            }
        }

        if (newCluster)
            moduleClusters.push_back(cluster);
    }
}

void FPGATrackSimClusteringTool::splitAndSortHits(std::vector<FPGATrackSimHit> &hits, std::vector<std::vector<FPGATrackSimHit> > &hitsPerModule, int &eta_phi) const {
    splitHitsToModules(hits, hitsPerModule);
    sortHitsOnModules(hitsPerModule, eta_phi);
}

void FPGATrackSimClusteringTool::splitAndSortHits(std::vector<FPGATrackSimHit> &hits, std::vector<std::vector<FPGATrackSimHit> > &hitsPerModule) const{
    splitHitsToModules(hits, hitsPerModule);
    sortHitsOnModules(hitsPerModule);
}

/*Temporarilly sort the hits into module by module packets
*/
void FPGATrackSimClusteringTool::splitHitsToModules(std::vector<FPGATrackSimHit> &hits, std::vector<std::vector<FPGATrackSimHit> > &hitsPerModule) const{
    //To hold the current module
    std::vector<FPGATrackSimHit> currentModule;
    uint hashing = 0;
    //Split the incoming hits into hits by module
    for ( auto& hit:hits){
        if(hashing == 0){
            currentModule.push_back(hit);
            hashing = hit.getIdentifierHash();
        } else if (hit.getIdentifierHash() == hashing) {
            currentModule.push_back(hit);
        } else {
            hitsPerModule.push_back(currentModule);
            currentModule.clear();
            hashing = hit.getIdentifierHash();
            currentModule.push_back(hit);
        }
    }

    // Now push that last one
    if (currentModule.size() > 0) hitsPerModule.push_back(currentModule);
}

void FPGATrackSimClusteringTool::sortHitsOnModules(std::vector<std::vector<FPGATrackSimHit> > &hitsPerModule, int &eta_phi) const{
    //Loop over the module separated hits
    for ( auto& module:hitsPerModule){
        //Work out if columns are ETA (1) || PHI (0)
        if(etaOrPhi(module.at(0)) == true){
            //Sort by ETA first
            eta_phi = ETA;
            if (module.size() > 1) {
                if (module.at(0).isStrip())
                    std::stable_sort(module.begin(), module.end(), FPGATrackSimCLUSTERING::sortITkInputEta);
            }
            if (module.size() > 1) {
                if (module.at(0).isStrip())
                    std::stable_sort(module.begin(), module.end(), FPGATrackSimCLUSTERING::sortITkInputPhi);
            }
        } else {
            //Sort by PHI first
            eta_phi = PHI;
            if (module.size() > 1) {
                if (module.at(0).isStrip())
                    std::stable_sort(module.begin(), module.end(), FPGATrackSimCLUSTERING::sortITkInputPhi);
            }
            if (module.size() > 1) {
                if (module.at(0).isStrip())
                    std::stable_sort(module.begin(), module.end(), FPGATrackSimCLUSTERING::sortITkInputEta);
            }
        }
    }
}

void FPGATrackSimClusteringTool::sortHitsOnModules(std::vector<std::vector<FPGATrackSimHit> > &hitsPerModule) const{
    //Loop over the module separated hits
    for ( auto& module:hitsPerModule){
        if (module.size() > 1) {
            if (module.at(0).isStrip())
                std::stable_sort(module.begin(), module.end(), FPGATrackSimCLUSTERING::sortITkInputEta);
        }
        if (module.size() > 1) {
            if (module.at(0).isStrip())
                std::stable_sort(module.begin(), module.end(), FPGATrackSimCLUSTERING::sortITkInputPhi);
        }
    }
}



//Need to remove the fpgatracksim::scaleHitFactor and normalise the widths
void FPGATrackSimClusteringTool::normaliseClusters(std::vector<FPGATrackSimCluster> &clusters) const {
    for( auto &cluster:clusters){
        //Grab the cluster equiv
        FPGATrackSimHit clusterEquiv = cluster.getClusterEquiv();
        //Update the clusterEquiv's position and width
        if(clusterEquiv.isStrip()){
            //Clear the groupsize, set this to be one as we are only clustering one row.
            clusterEquiv.setEtaWidth(1);
            clusterEquiv.setPhiIndex(clusterEquiv.getPhiIndex()/fpgatracksim::scaleHitFactor);
            clusterEquiv.setPhiWidth(clusterEquiv.getPhiWidth()+1);
        } else {
            // Nothing to normalise for pixel clusters
            continue;
        }
        cluster.setClusterEquiv(clusterEquiv);
    }
}


/*Function to work out if we need to sort by eta or phi first.
 *Depends on the position and orientation of the module in the detector.
 * Currently backwards engineered from the MC. Need to ask ITk people to confirm.
 */
bool FPGATrackSimClusteringTool::etaOrPhi(const FPGATrackSimHit& hit) const {

    /*This currently might get complicated as eta/phi ordering varies depending on the position in the detector.
     * For ITK-20-00-00 Step 2.2 inclined duals layout, worked out by Naoki.
     * Detector position               |  Module Type (# chip)  |  Column  |  Row  |
     * =============================================================================
     * Barrel Layer 0 Eta module 1-6   |  2           (double)  |  eta     |  phi  |
     * Barrel Layer 0 Eta module 7-22  |  1           (single)  |  phi     |  eta  |
     * Barrel Layer 1 Eta module 1-6   |  3           (quad)    |  eta     |  phi  |
     * Barrel Layer 1 Eta module 7-19  |  3                     |  phi     |  eta  |
     * Barrel Layer 2 Eta module 1-11  |  3                     |  eta     |  phi  |
     * Barrel Layer 2 Eta module 12-22 |  2                     |  phi     |  eta  |
     * Barrel Layer 3 Eta module 1-12  |  3                     |  eta     |  phi  |
     * Barrel Layer 3 Eta module 13-25 |  2                     |  phi     |  eta  |
     * Barrel Layer 4 Eta module 1-13  |  3                     |  eta     |  phi  |
     * Barrel Layer 4 Eta module 14-26 |  2                     |  phi     |  eta  |
     * All Endcap modules              |  3                     |  eta     |  phi  |
     * =============================================================================
     * Module Type 1 = Single, 2, Dual, 3 Quad
     * 328x400 blocks
     * Hit type is essentially isPixel
     * DetectorZone 0 = Barrel, -ive/+ive = endcaps
     */

    //Check if the two hits are from the same module
    //If it is not a barrel module then sort eta as column
    if(hit.getDetectorZone() != DetectorZone::barrel){
        return ETA;
    }
    //Otherwise it is a barrel module and now things get more complicated
    else {
        //Start by looking at what layer it is in
        if (hit.getPhysLayer() == 0 || hit.getPhysLayer() == 1) {
            if (hit.getEtaModule() <=6) {
                return ETA;
            } else {
                return PHI;
            }
        } else if (hit.getPhysLayer() == 2) {
            if (hit.getEtaModule() <=11) {
                return ETA;
            } else {
                return PHI;
            }
        } else if (hit.getPhysLayer() == 3) {
            if (hit.getEtaModule() <=12) {
                return ETA;
            } else {
                return PHI;
            }
        } else if (hit.getPhysLayer() == 4) {
            if (hit.getEtaModule() <=13) {
                return ETA;
            } else {
                return PHI;
            }
        }
    }
    //Default to ETA, but shouldn't reach here
    return ETA;
}



void FPGATrackSimCLUSTERING::attachTruth(std::vector<FPGATrackSimHit> &hits){
    for( auto& hit : hits) {
        FPGATrackSimMultiTruth mt;
        // record highest pt contribution to the combination (cluster
        if(!hit.getTruth().isEmpty()) {
            mt.add(hit.getTruth());
            hit.setTruth(mt);
        } else {
            FPGATrackSimMultiTruth::Barcode uniquecode(hit.getEventIndex(), hit.getBarcode());
            mt.maximize(uniquecode, hit.getBarcodePt());
            hit.setTruth(mt);
        }
    }
} //record truth for each raw channel in the cluster

/*
 * This function is used in the FPGATrackSimClusteringTools to see if a new hit should be added to the current cluster under construction.
 * It checks if the hit is in a number of positions w.r.t. the cluster being formed: up/right, down/right, above, right, or inside a cluster that has formed a horseshoe.
 */
bool FPGATrackSimCLUSTERING::updatePixelCluster(FPGATrackSimCluster &currentCluster, FPGATrackSimHit &incomingHit, bool newCluster, bool digitalClustering){

    if(newCluster){
        FPGATrackSimHit newHit = incomingHit;
        newHit.setEtaIndex(incomingHit.getEtaIndex());
        newHit.setPhiIndex(incomingHit.getPhiIndex());
        newHit.setEtaCoord(incomingHit.getEtaCoord());
        newHit.setPhiCoord(incomingHit.getPhiCoord());
        newHit.setCentroidPhiIndex(incomingHit.getPhiIndex() + 0.5);
        newHit.setCentroidEtaIndex(incomingHit.getEtaIndex() + 0.5);
        newHit.setEtaWidth(1);
        newHit.setPhiWidth(1);
        //Set the initial clusterEquiv to be the incoming hit with double precision
        currentCluster.setClusterEquiv(newHit);
        //Add the current hit to the list of hits
        currentCluster.push_backHitList(incomingHit);
        //It doesn't really matter, as we will be at the end of the hit loop, but we did technically "cluster" this hit
        return true;
    } else {
        int hitRow = incomingHit.getEtaIndex();
        int hitCol = incomingHit.getPhiIndex();

        FPGATrackSimHit clusterEquiv = currentCluster.getClusterEquiv();
        int clusterRow = clusterEquiv.getEtaIndex();
        int clusterRowWidth = clusterEquiv.getEtaWidth();
        int clusterCol = clusterEquiv.getPhiIndex();
        int clusterColWidth = clusterEquiv.getPhiWidth();

        if ((hitCol == clusterCol + clusterColWidth) && (hitRow == clusterRow + clusterRowWidth)) {
            clusterColWidth++;
            clusterRowWidth++;

            return FPGATrackSimCLUSTERING::updateClusterContents(currentCluster, clusterRow, clusterRowWidth, clusterCol, clusterColWidth, incomingHit, digitalClustering);
        } else if ((hitCol == clusterCol + clusterColWidth) && (hitRow == clusterRow - 1)) {
            clusterColWidth++;
            clusterRow--;
            clusterRowWidth++;

            return FPGATrackSimCLUSTERING::updateClusterContents(currentCluster, clusterRow, clusterRowWidth, clusterCol, clusterColWidth, incomingHit, digitalClustering);
        } else if ((hitCol >= clusterCol) && (hitCol < clusterCol + clusterColWidth) && (hitRow == clusterRow + clusterRowWidth)) {
            clusterRowWidth++;

            return FPGATrackSimCLUSTERING::updateClusterContents(currentCluster, clusterRow, clusterRowWidth, clusterCol, clusterColWidth, incomingHit, digitalClustering);
        } else if ((hitCol == clusterCol + clusterColWidth) && (hitRow >= clusterRow) && (hitRow < clusterRow + clusterRowWidth)) {
            clusterColWidth++;

            return FPGATrackSimCLUSTERING::updateClusterContents(currentCluster, clusterRow, clusterRowWidth, clusterCol, clusterColWidth, incomingHit, digitalClustering);
        } else if ((hitCol >= clusterCol) && (hitCol < clusterCol + clusterColWidth) && (hitRow == clusterRow - 1)) {
            clusterRow--;
            clusterRowWidth++;

            return FPGATrackSimCLUSTERING::updateClusterContents(currentCluster, clusterRow, clusterRowWidth, clusterCol, clusterColWidth, incomingHit, digitalClustering);
        } else if ((hitCol == clusterCol - 1) && (hitRow == clusterRow - 1)) {
            clusterCol--;
            clusterColWidth++;
            clusterRow--;
            clusterRowWidth++;

            return FPGATrackSimCLUSTERING::updateClusterContents(currentCluster, clusterRow, clusterRowWidth, clusterCol, clusterColWidth, incomingHit, digitalClustering);
        } else if ((hitCol == clusterCol - 1) && (hitRow >= clusterRow) && (hitRow < clusterRow + clusterRowWidth)) {
            clusterCol--;
            clusterColWidth++;

            return FPGATrackSimCLUSTERING::updateClusterContents(currentCluster, clusterRow, clusterRowWidth, clusterCol, clusterColWidth, incomingHit, digitalClustering);
        } else if ((hitCol == clusterCol - 1) && (hitRow == clusterRow + clusterRowWidth)) {
            clusterCol--;
            clusterColWidth++;
            clusterRowWidth++;

            return FPGATrackSimCLUSTERING::updateClusterContents(currentCluster, clusterRow, clusterRowWidth, clusterCol, clusterColWidth, incomingHit, digitalClustering);
        } else if ((hitCol >= clusterCol) && (hitCol < clusterCol + clusterColWidth) && (hitRow >= clusterRow) && (hitRow < clusterRow + clusterRowWidth)) {
            return FPGATrackSimCLUSTERING::updateClusterContents(currentCluster, clusterRow, clusterRowWidth, clusterCol, clusterColWidth, incomingHit, digitalClustering);
        } else {
            return false;
        }
    }
}

/*
 * This function is used in the FPGATrackSimClusteringTools to see if a new hit should be added to the current cluster under construction. It assumes double precision hits.
 * It checks if the hit is in a number of positions w.r.t. the cluster being formed: up/right, down/right, above, right, or inside a cluster that has formed a horseshoe.
 */
bool FPGATrackSimCLUSTERING::updateStripCluster(FPGATrackSimCluster &currentCluster, FPGATrackSimHit &incomingHit, bool newCluster, bool digitalClustering){

    CXXUTILS_TRAPPING_FP;

    // Shift initial widths 1->0, 2->2, 3->4, 4->6 etc...
    //The groupSize is stored in the EtaWidth
    int tempWidth = (incomingHit.getEtaWidth()*fpgatracksim::scaleHitFactor)-fpgatracksim::scaleHitFactor;
    // Now shift to pixel width equivalents, 0->0, 2->1, 4->2, 6->3 etc...
    if(tempWidth > 0) tempWidth = tempWidth/fpgatracksim::scaleHitFactor;
    if(newCluster){
        FPGATrackSimHit newHit = incomingHit;
        //Double the precision of the strip positions.
        int tempCentroid = incomingHit.getPhiIndex()*fpgatracksim::scaleHitFactor;
        // Now shift the centroid phi+phiWidth, and store the width (put it back in the PhiWidth)
        newHit.setPhiIndex(tempCentroid+tempWidth);
        newHit.setPhiWidth(tempWidth);
        //Set the initial clusterEquiv to be the incoming hit with double precision
        currentCluster.setClusterEquiv(newHit);
        //Add the current hit to the list of hits
        currentCluster.push_backHitList(incomingHit);
        //It doesn't really matter, as we will be at the end of the hit loop, but we did technically "cluster" this hit
        return true;
    } else {
        //Now get the --START-- of the new strip cluster
        int hitRow = incomingHit.getEtaIndex();
        int hitCol = incomingHit.getPhiIndex()*fpgatracksim::scaleHitFactor;

        FPGATrackSimHit clusterEquiv = currentCluster.getClusterEquiv();
        int clusterRow = clusterEquiv.getEtaIndex();
        int clusterRowWidth = clusterEquiv.getEtaWidth();
        int clusterCol = clusterEquiv.getPhiIndex();
        int clusterColWidth = clusterEquiv.getPhiWidth();

        //Looking for a neighbour to the right. i.e. find the end of the current cluster (Col+width) and look in the next cell (+2). Compare this to the start of the new cluster. This is unlikely/impossible(?) to happen due to preclustering.
        if(hitCol == clusterCol+clusterColWidth+fpgatracksim::scaleHitFactor && hitRow == clusterRow) {
            //The new centroid will be the original column position, minus its width, plus the new width
            //So subtract the original width...
            clusterCol = clusterCol - clusterColWidth;
            //The new width will be the combination of the current widths, ++
            clusterColWidth = clusterColWidth+tempWidth+1;
            //And add on the new width
            clusterCol = clusterCol + clusterColWidth;
            FPGATrackSimCLUSTERING::updateClusterContents(currentCluster, clusterRow, clusterRowWidth, clusterCol, clusterColWidth, incomingHit, digitalClustering);
            return true;
        } else return false;
    }
}


bool FPGATrackSimCLUSTERING::updateClusterContents(FPGATrackSimCluster &currentCluster, int &clusterRow, int &clusterRowWidth, int &clusterCol, int &clusterColWidth, FPGATrackSimHit &incomingHit, bool digitalClustering) {
    //Grab the cluster equiv
    FPGATrackSimHit clusterEquiv = currentCluster.getClusterEquiv();
    bool isConnected = false;

    //Check if connected to another hit in the cluster
    if(incomingHit.isPixel()){
        for (auto & hit : currentCluster.getHitList()) {
            auto hitEta = hit.getEtaIndex();
            auto hitPhi = hit.getPhiIndex();
            auto inHitEta = incomingHit.getEtaIndex();
            auto inHitPhi = incomingHit.getPhiIndex();

            if (((inHitEta == hitEta - 1) && (inHitPhi == hitPhi - 1)) ||
                    ((inHitEta == hitEta + 1) && (inHitPhi == hitPhi - 1)) ||
                    ((inHitEta == hitEta - 1) && (inHitPhi == hitPhi + 1)) ||
                    ((inHitEta == hitEta + 1) && (inHitPhi == hitPhi + 1)) ||
                    ((inHitEta == hitEta) && (inHitPhi == hitPhi - 1)) ||
                    ((inHitEta == hitEta) && (inHitPhi == hitPhi + 1)) ||
                    ((inHitEta == hitEta - 1) && (inHitPhi == hitPhi)) ||
                    ((inHitEta == hitEta + 1) && (inHitPhi == hitPhi))) {
                isConnected = true;
                break;
            }
        }
        if (!isConnected)
            return false;
    }

    //Update the clusterEquiv's position and width
    clusterEquiv.setEtaIndex(clusterRow);
    clusterEquiv.setEtaWidth(clusterRowWidth);
    clusterEquiv.setPhiIndex(clusterCol);
    clusterEquiv.setPhiWidth(clusterColWidth);


    float xOld = clusterEquiv.getX();
    float yOld = clusterEquiv.getY();
    float zOld = clusterEquiv.getZ();
    float xPhiOld = clusterEquiv.getPhiCoord();
    float xEtaOld = clusterEquiv.getEtaCoord();
    float cPhiOld = clusterEquiv.getCentroidPhiIndex();
    float cEtaOld = clusterEquiv.getCentroidEtaIndex();
    float xNew = incomingHit.getX();
    float yNew = incomingHit.getY();
    float zNew = incomingHit.getZ();
    float xPhiNew = incomingHit.getPhiCoord();
    float xEtaNew = incomingHit.getEtaCoord();
    float cPhiNew = incomingHit.getPhiIndex() + 0.5;
    float cEtaNew = incomingHit.getEtaIndex() + 0.5;
    int tot = clusterEquiv.getToT();
    int totNew = incomingHit.getToT();
    //As strips arrive pre-clustered, this is different for pixels/strips
    if(incomingHit.isPixel()){
        CXXUTILS_TRAPPING_FP;
        if (digitalClustering) {
            // n+1 because that is old + new now
            int n = currentCluster.getHitList().size();
            clusterEquiv.setX((xOld*n + xNew) / (n+1));
            clusterEquiv.setY((yOld*n + yNew) / (n+1));
            clusterEquiv.setZ((zOld*n + zNew) / (n+1));
            clusterEquiv.setPhiCoord((xPhiOld*n + xPhiNew) / (n+1));
            clusterEquiv.setEtaCoord((xEtaOld*n + xEtaNew) / (n+1));
            clusterEquiv.setCentroidPhiIndex((cPhiOld*n + cPhiNew) / (n+1));
            clusterEquiv.setCentroidEtaIndex((cEtaOld*n + cEtaNew) / (n+1));
        } else {
            clusterEquiv.setX((xOld*tot + xNew*totNew) / (tot+totNew));
            clusterEquiv.setY((yOld*tot + yNew*totNew) / (tot+totNew));
            clusterEquiv.setZ((zOld*tot + zNew*totNew) / (tot+totNew));
            clusterEquiv.setPhiCoord((xPhiOld*tot + xPhiNew*totNew) / (tot+totNew));
            clusterEquiv.setEtaCoord((xEtaOld*tot + xEtaNew*totNew) / (tot+totNew));
            clusterEquiv.setCentroidPhiIndex((cPhiOld*tot + cPhiNew*totNew) / (tot+totNew));
            clusterEquiv.setCentroidEtaIndex((cEtaOld*tot + cEtaNew*totNew) / (tot+totNew));
        }
    } else {
        //Phi width + 1 for the seed is the width of the current cluster
        int N = currentCluster.getClusterEquiv().getPhiWidth()+1;
        //Phi width of an incoming strip is the width of the cluster
        int newN = incomingHit.getPhiWidth();
        //Now as above, N+newN
        clusterEquiv.setX((xOld*N + xNew*newN) / (N+newN));
        clusterEquiv.setY((yOld*N + yNew*newN) / (N+newN));
        clusterEquiv.setZ((zOld*N + zNew*newN) / (N+newN));
    }
    clusterEquiv.setToT(tot + totNew);

    //Put it back
    currentCluster.setClusterEquiv(clusterEquiv);

    //Pushback the hit into the hitlist
    currentCluster.push_backHitList(incomingHit);

    return true;
}

/* Sort for the ordering of ITk modules: Sort by ETA.
*/
bool FPGATrackSimCLUSTERING::sortITkInputEta(const FPGATrackSimHit& hitA, const FPGATrackSimHit& hitB)
{
    if (hitA.getIdentifierHash() != hitB.getIdentifierHash())
        return hitA.getIdentifierHash() < hitB.getIdentifierHash();
    return hitA.getEtaIndex() < hitB.getEtaIndex();
}

/* Sort for the ordering of ITk modules: Sort by PHI.
*/
bool FPGATrackSimCLUSTERING::sortITkInputPhi(const FPGATrackSimHit& hitA, const FPGATrackSimHit& hitB)
{
    if (hitA.getIdentifierHash() != hitB.getIdentifierHash())
        return hitA.getIdentifierHash() < hitB.getIdentifierHash();
    return hitA.getPhiIndex() < hitB.getPhiIndex();
}

/* Cap precision of the global coordinates in r, phi, z */
void FPGATrackSimClusteringTool::reduceGlobalCoordPrecision(FPGATrackSimCluster &cluster) const {
    FPGATrackSimHit clusterEquiv = cluster.getClusterEquiv();
    float pos[3] = { clusterEquiv.getR(), clusterEquiv.getGPhi(), clusterEquiv.getZ() };

    pos[0] = std::trunc(pos[0] / m_coordRPrecision) * m_coordRPrecision;
    pos[1] = std::trunc(pos[1] / m_coordPhiPrecision) * m_coordPhiPrecision;
    pos[2] = std::trunc(pos[2] / m_coordZPrecision) * m_coordZPrecision;

    clusterEquiv.setX(pos[0] * std::cos(pos[1]));
    clusterEquiv.setY(pos[0] * std::sin(pos[1]));
    clusterEquiv.setZ(pos[2]);

    cluster.setClusterEquiv(clusterEquiv);
}

void FPGATrackSimClusteringTool::reduceGlobalCoordPrecision(FPGATrackSimHit &hit) const {
    float pos[3] = { hit.getR(), hit.getGPhi(), hit.getZ() };

    pos[0] = std::trunc(pos[0] / m_coordRPrecision) * m_coordRPrecision;
    pos[1] = std::trunc(pos[1] / m_coordPhiPrecision) * m_coordPhiPrecision;
    pos[2] = std::trunc(pos[2] / m_coordZPrecision) * m_coordZPrecision;

    hit.setX(pos[0] * std::cos(pos[1]));
    hit.setY(pos[0] * std::sin(pos[1]));
    hit.setZ(pos[2]);
}
