/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "eflowRec/PFSimulateTruthShowerTool.h"

#include "eflowRec/eflowRecCluster.h"
#include "eflowRec/eflowRecTrack.h"
#include "eflowRec/eflowTrackClusterLink.h"

#include "xAODTruth/TruthParticleContainer.h"

PFSimulateTruthShowerTool::PFSimulateTruthShowerTool(const std::string& type,
                                                       const std::string& name,
                                                       const IInterface* parent) :
                                                       AthAlgTool(type, name, parent){}

StatusCode PFSimulateTruthShowerTool::initialize(){

    ATH_CHECK(m_tileActiveCaloCalibrationHitReadHandleKey.initialize());
    ATH_CHECK(m_lArActiveCaloCalibrationHitReadHandleKey.initialize());

    return StatusCode::SUCCESS;
}

StatusCode PFSimulateTruthShowerTool::finalize(){
    return StatusCode::SUCCESS;
}

void PFSimulateTruthShowerTool::simulateShower(eflowCaloObject& thisEFlowCaloObject) const{

    for (unsigned int trackCounter = 0; trackCounter < thisEFlowCaloObject.nTracks();trackCounter++){
        
        eflowRecTrack* thisTrack = thisEFlowCaloObject.efRecTrack(trackCounter);

        typedef ElementLink<xAOD::TruthParticleContainer> TruthLink;

        const static SG::AuxElement::Accessor<TruthLink> truthLinkAccessor("truthParticleLink");
        //if truthLink not valid don't print a WARNING because this is an expected condition as discussed here:
        //https://indico.cern.ch/event/795039/contributions/3391771/attachments/1857138/3050771/TruthTrackFTAGWS.pdf
        TruthLink truthLink = truthLinkAccessor(*(thisTrack->getTrack()));
        
        if (!truthLink.isValid()) continue;

        //get uniqueID of particle
        const int uniqueID = (*truthLink)->uid();

        SG::ReadHandle<CaloCalibrationHitContainer> tileActiveCaloCalibrationHitReadHandle(m_tileActiveCaloCalibrationHitReadHandleKey);
        if (!tileActiveCaloCalibrationHitReadHandle.isValid()){
            ATH_MSG_WARNING("TileActiveCaloCalibrationHitReadHandle is not valid");
            return;
        }

        SG::ReadHandle<CaloCalibrationHitContainer> lArActiveCaloCalibrationHitReadHandle(m_lArActiveCaloCalibrationHitReadHandleKey);
        if (!lArActiveCaloCalibrationHitReadHandle.isValid()){
            ATH_MSG_WARNING("lArActiveCaloCalibrationHitReadHandle is not valid");
            return;
        }

        std::map<Identifier,double> identifierToTruthEnergyMap;

        for (auto thisCalibHit : *tileActiveCaloCalibrationHitReadHandle) this->fillMap(identifierToTruthEnergyMap,uniqueID,*thisCalibHit);
        for (auto thisCalibHit : *lArActiveCaloCalibrationHitReadHandle) this->fillMap(identifierToTruthEnergyMap,uniqueID,*thisCalibHit);

        //find the matched clusters
        std::vector<eflowRecCluster*> matchedClusters;
        for (auto thisLink : thisTrack->getClusterMatches()) matchedClusters.push_back(thisLink->getCluster());

        for (auto thisLink : thisTrack->getClusterMatches()){
            for (auto thisCell : *(thisLink->getCluster()->getCluster()->getOwnCellLinks())){
                //look up the truth energy for this cell
                if (identifierToTruthEnergyMap.count(thisCell->ID()) == 0) continue;
                double truthEnergy = identifierToTruthEnergyMap[thisCell->ID()];
                thisTrack->insertTruthEnergyPair(thisCell,truthEnergy);
            }//cell loop
        }//matched cluster loop
    }//track loop
}

void PFSimulateTruthShowerTool::fillMap(std::map<Identifier,double>& identifierToTruthEnergyMap, int uniqueID, const CaloCalibrationHit& thisCalibHit) const{
  if (HepMC::uniqueID(thisCalibHit) == uniqueID) {
        Identifier thisIdentifier = thisCalibHit.cellID();
        unsigned int count = identifierToTruthEnergyMap.count(thisIdentifier);
        if (0 == count) identifierToTruthEnergyMap[thisIdentifier] = thisCalibHit.energyEM() + thisCalibHit.energyNonEM();
        else identifierToTruthEnergyMap[thisIdentifier] += (thisCalibHit.energyEM() + thisCalibHit.energyNonEM());
    }
}
