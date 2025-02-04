// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

/**
 * @file FPGATrackSimRoadFilterTool.h
 * @author Tomasz Bold - toold to filter rods & hits for LLP
 * @date 2021
 * @brief Declares a concrete class that is directly used the FPGATrackSim main algorithm to filter hits belonging to already found roads
 */
#ifndef FPGATrackSim_LLPROADFILTERTOOL_H
#define FPGATrackSim_LLPROADFILTERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "FPGATrackSimObjects/FPGATrackSimTrack.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"

#include <string>
#include <vector>

class FPGATrackSimLLPRoadFilterTool : public AthAlgTool {
public:
  FPGATrackSimLLPRoadFilterTool (const std::string& algname, const std::string& name, const IInterface* ifc);

  StatusCode filterUsedHits(std::vector<FPGATrackSimTrack> &tracks, 
			    const std::vector<std::shared_ptr<const FPGATrackSimHit>>& allHits, 
			    std::vector<std::shared_ptr<const FPGATrackSimHit>>& unusedHits );

private:
    // Make a list of all hits used in any track
    struct HitCompare {
        bool operator()(std::shared_ptr<const FPGATrackSimHit> a, std::shared_ptr<const FPGATrackSimHit> b) const { 
            auto hash_a = a->getIdentifierHash();
            auto hash_b = b->getIdentifierHash();
            if ( hash_a == hash_b ) {
                auto phi_a = a->getPhiIndex();
                auto phi_b = b->getPhiIndex();
                if ( phi_a == phi_b ) {
                    auto eta_a = a->getEtaIndex();
                    auto eta_b = b->getEtaIndex();
                    if ( eta_a == eta_b) {
                        auto layer_a = a->getLayer();
                        auto layer_b = b->getLayer();
                        return layer_a < layer_b;
                    }
                    return eta_a < eta_b;
                }
                return phi_a < phi_b;
            }
            return hash_a <  hash_b; 
        }
    };

};

#endif
