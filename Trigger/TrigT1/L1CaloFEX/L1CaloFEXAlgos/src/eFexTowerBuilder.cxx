/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFexTowerBuilder  -  description:
//       Builds an eFexTowerContainer from a CaloCellContainer (for supercells) and TriggerTowerContainer (for ppm tile towers)
//                              -------------------
//     begin                : 06 12 2022
//     email                : will@cern.ch
//***************************************************************************/


// MyPackage includes
#include "eFexTowerBuilder.h"

#include "xAODTrigL1Calo/eFexTowerAuxContainer.h"

#include "CaloIdentifier/CaloCell_SuperCell_ID.h"


#include "TFile.h"
#include "TTree.h"
#include "PathResolver/PathResolver.h"

namespace LVL1 {

eFexTowerBuilder::eFexTowerBuilder( const std::string& name, ISvcLocator* pSvcLocator ) : AthReentrantAlgorithm( name, pSvcLocator ){


}

StatusCode eFexTowerBuilder::initialize() {
    ATH_MSG_INFO ("Initializing " << name() << "...");

    CHECK( m_ddmKey.initialize(true) );
    CHECK( m_ttKey.initialize(true) );
    CHECK( m_scellKey.initialize(true) );
    CHECK( m_outKey.initialize(true) );
    CHECK( m_eiKey.initialize(true) );
    CHECK( m_LArLatomeHeaderContainerKey.initialize(SG::AllowEmpty) );

    if (auto fileName = PathResolverFindCalibFile( m_mappingFile ); !fileName.empty()) {
        std::unique_ptr<TFile> f( TFile::Open(fileName.c_str()) );
        if (f) {
            TTree* t = f->Get<TTree>("mapping");
            if(t) {
                unsigned long long scid = 0;
                std::pair<int,int> coord = {0,0};
                std::pair<int,int> slot;
                t->SetBranchAddress("scid",&scid);
                t->SetBranchAddress("etaIndex",&coord.first);
                t->SetBranchAddress("phiIndex",&coord.second);
                t->SetBranchAddress("slot1",&slot.first);
                t->SetBranchAddress("slot2",&slot.second);
                for(Long64_t i=0;i<t->GetEntries();i++) {
                    t->GetEntry(i);
                    m_scMap[scid] = std::make_pair(coord,slot);
                }
            }
        }
        if (m_scMap.empty()) {
            ATH_MSG_WARNING("Failed to load sc -> eFexTower map from " << fileName);
        } else {
            ATH_MSG_INFO("Loaded sc -> eFexTower map from " << fileName);
        }
    }

    return StatusCode::SUCCESS;
}

StatusCode eFexTowerBuilder::fillTowers(const EventContext& ctx) const {


    SG::ReadCondHandle<CaloSuperCellDetDescrManager> ddm{m_ddmKey,ctx};
    SG::ReadHandle<xAOD::TriggerTowerContainer> tTowers(m_ttKey,ctx);
    if(!tTowers.isValid()){
        ATH_MSG_FATAL("Could not retrieve collection " << m_ttKey.key() );
        return StatusCode::FAILURE;
    }
    SG::ReadHandle<CaloCellContainer> scells(m_scellKey,ctx); // n.b. 34048 is a full complement of scells
    if(!scells.isValid()){
        ATH_MSG_FATAL("Could not retrieve collection " << m_scellKey.key() );
        return StatusCode::FAILURE;
    }

    SG::ReadHandle<xAOD::EventInfo> ei(m_eiKey,ctx);
    if(!ei.isValid()) {
      ATH_MSG_FATAL("Cannot retrieve eventinfo");
      return StatusCode::FAILURE;
    }
    bool isMC = ei->eventType(xAOD::EventInfo::IS_SIMULATION); // currently only used to decide if should set a saturation code or not
    

    std::map<std::pair<int,int>,std::array<int,11>> towers;

    constexpr int INVALID_VALUE = -99999; // use this value to indicate invalid
    constexpr int MASKED_VALUE = std::numeric_limits<int>::max(); // use this value to indicate masked
    constexpr int SATURATED_VALUE = std::numeric_limits<int>::max()-1; // use this value to indicate saturation

    for (auto digi: *scells) {
        const auto itr = m_scMap.find(digi->ID().get_compact());
        if (itr == m_scMap.end()) { continue; } // not in map so not mapping to a tower
        int val =  std::round(digi->energy()/(12.5*std::cosh(digi->eta()))); // 12.5 is b.c. energy is in units of 12.5MeV per count
        // note: a val of < -99998 is what is produced if efex was sent an invalid code of 1022 (see LArRawtoSuperCell)
        bool isSaturated = (!isMC) ? (digi->quality()) : false; // not applying saturation codes in MC until the changes to trigger counts has been investigated
        bool isMasked = ((digi)->provenance()&0x80);
        bool isInvalid = m_applyMasking ? ((digi)->provenance()&0x40) : false;
        // note: if debugging, the SCIDs have value: digi->ID().get_compact()>>32
        if(isInvalid) {
            val = INVALID_VALUE;
        }
        if(isSaturated) {
            val = SATURATED_VALUE;
        }

        auto towerItr = towers.emplace(itr->second.first,std::array<int,11>{}); // returns pair<itr,bool> with bool indicating if emplaced
        if(towerItr.second) { // did an emplace
            towerItr.first->second.fill(INVALID_VALUE); // ensure all slots initialize with invalid value
        }
        auto& tower = (towerItr.first->second);
        if (itr->second.second.second<11) {
            // doing an energy split between slots ... don't include a masked channel (or invalid channel)
            if (!isMasked && val!=INVALID_VALUE) {
                if(isSaturated) {
                    // mark both as saturated
                    tower.at(itr->second.second.first) = SATURATED_VALUE;
                    tower.at(itr->second.second.second) = SATURATED_VALUE;
                }
                if(tower.at(itr->second.second.first)!=(SATURATED_VALUE)) { // don't override saturation
                    // if the other contribution was masked or invalid, revert to 0 before adding this contribution
                    if (tower.at(itr->second.second.first)==MASKED_VALUE || tower.at(itr->second.second.first)==INVALID_VALUE) {
                        tower.at(itr->second.second.first)=0;
                    }
                    tower.at(itr->second.second.first) += val >> 1;
                }
                if(tower.at(itr->second.second.second)!=(SATURATED_VALUE)) { // don't override saturation
                    // if the other contribution was masked or invalid, revert to 0 before adding this contribution
                    if (tower.at(itr->second.second.second)==MASKED_VALUE || tower.at(itr->second.second.second)==INVALID_VALUE) {
                        tower.at(itr->second.second.second)=0;
                    }
                    tower.at(itr->second.second.second) += (val - (val >> 1)); // HW seems fixed now!
                }
            }
            // hw is incorrectly ignoring masking on the second part
            // so always add the 2nd bit
            //tower.at(itr->second.second.second) += (val - (val >> 1)); // Removed b.c. of fix above - leaving this comment here until resolved!
        } else {
            auto& v = tower.at(itr->second.second.first);
            if (isMasked) {
                // dont mark it masked if it already has a contribution
                if(v==INVALID_VALUE) v = MASKED_VALUE;
            } else if(isSaturated) {
                v = val;
            } else {
                if(v==INVALID_VALUE) v = 0;
                v += val;
            }
        }

    }

    // add tile energies from TriggerTowers
    static const auto etaIndex = [](float eta) { return int( eta*10 ) + ((eta<0) ? -1 : 1); };
    static const auto phiIndex = [](float phi) { return int( phi*32./M_PI ) + (phi<0 ? -1 : 1); };
    for(const xAOD::TriggerTower_v2* tTower : *tTowers) {
        if (std::abs(tTower->eta()) > 1.5) continue;
        if (tTower->sampling() != 1) continue;
        double phi = tTower->phi(); if(phi > M_PI) phi -= 2.*M_PI;
        auto towerItr = towers.emplace(std::pair(etaIndex(tTower->eta()),phiIndex(phi)),std::array<int,11>{}); // returns pair<itr,bool> with bool indicating if emplaced
        if(towerItr.second) { // did an emplace
            towerItr.first->second.fill(INVALID_VALUE); // ensure all slots initialize with invalid value
        }
        (towerItr.first->second).at(10) = tTower->cpET();
    }


    SG::WriteHandle<xAOD::eFexTowerContainer> eTowers = SG::WriteHandle<xAOD::eFexTowerContainer>(m_outKey,ctx);
    ATH_CHECK( eTowers.record(std::make_unique<xAOD::eFexTowerContainer>(),std::make_unique<xAOD::eFexTowerAuxContainer>()) );

    static const auto calToFex = [](int calEt) {
        if(calEt == MASKED_VALUE) return 0; // indicates masked channel
        if(calEt == SATURATED_VALUE) return 1023; // saturated channel
        if( calEt == INVALID_VALUE ) return 1022; // invalid channel value
        if(calEt<448) return std::max((calEt&~1)/2+32,1); // 25 MeV per eFexTower count
        if(calEt<1472) return (calEt-448)/4+256;          // 50 MeV per eFexTower count
        if(calEt<3520) return (calEt-1472)/8+512;         // 100 MeV ...
        if(calEt<11584) return (calEt-3520)/32+768;       // 400 MeV ...
        return 1020;
    };

    // now create the towers. Note that we need a code for "missing" input (1025), because a tower can contain some but not all
    // inputs, depending on which sources are present in the run (e.g. if tile is present but not LAr).
    // This is different to e.g. jFex, which creates a separate tower for each source at each location
    // so jFex doesn't need a "missing" input code.
    for(auto& [coord,counts] : towers) {
        size_t ni = (std::abs(coord.first)<=15) ? 10 : 11; // ensures we skip the tile towers for next line
        for(size_t i=0;i<ni;++i) counts[i] = (scells->empty() ? 1025 : calToFex(counts[i])); // do latome energy scaling to non-tile towers - if had no cells will use code "1025" to indicate
        eTowers->push_back( std::make_unique<xAOD::eFexTower>() );
        eTowers->back()->initialize( ( (coord.first<0 ? 0.5:-0.5) + coord.first)*0.1 ,
                                 ( (coord.second<0 ? 0.5:-0.5) + coord.second)*M_PI/32,
                                     std::vector<uint16_t>(counts.begin(), counts.end()),
                                 -1, /* module number */
                                 -1, /* fpga number */
                                 0,0 /* status flags ... could use to indicate which cells were actually present?? */);
    }

    return StatusCode::SUCCESS;

}

StatusCode eFexTowerBuilder::fillMap(const EventContext& ctx) const {

    ATH_MSG_INFO("Filling sc -> eFexTower map");

    SG::ReadCondHandle<CaloSuperCellDetDescrManager> ddm{m_ddmKey,ctx};
    SG::ReadHandle<CaloCellContainer> scells(m_scellKey,ctx); // 34048 is a full complement of scells
    if(!scells.isValid()){
        ATH_MSG_FATAL("Could not retrieve collection " << m_scellKey.key() );
        return StatusCode::FAILURE;
    }
    if (scells->size() != 34048 && !m_mappingFile.empty()) {
        ATH_MSG_FATAL("Cannot fill sc -> eFexTower mapping with an incomplete sc collection");
        return StatusCode::FAILURE;
    }

    // read the LATOME header if a key is given, so that we can determine LATOME version and get mapping right
    bool doV6Mapping = m_v6Mapping;

    if(!m_LArLatomeHeaderContainerKey.empty()) {
        SG::ReadHandle<LArLATOMEHeaderContainer> hdrCont(m_LArLatomeHeaderContainerKey,ctx);
        if(hdrCont.isValid()) {
            for (const LArLATOMEHeader* hit : *hdrCont) {
                doV6Mapping = (hit->FWversion()>1600);
            }
            if (doV6Mapping != m_v6Mapping) {
                ATH_MSG_WARNING("Used LATOME Hardware to determine mapping different to python configuration (use V6 Mapping = " << doV6Mapping << " )");
            }
        }

    }
    struct TowerSCells {
        std::vector<unsigned long long> ps;
        std::vector<std::pair<float,unsigned long long>> l1;
        std::vector<std::pair<float,unsigned long long>> l2;
        std::vector<unsigned long long> l3;
        std::vector<unsigned long long> had;
        std::vector<unsigned long long> other;
    };
    static const auto etaIndex = [](float eta) { return int( eta*10 ) + ((eta<0) ? -1 : 1); }; // runs from -25 to 25, skipping over 0 (so gives outer edge eta)
    static const auto phiIndex = [](float phi) { return int( phi*32./ROOT::Math::Pi() ) + (phi<0 ? -1 : 1); }; // runs from -pi to pi, skipping over 0 (gives out edge phi)
    std::map<std::pair<int,int>,TowerSCells> towers;
    std::map<unsigned long long,int> eTowerSlots; // not used by this alg, but we produce the map for benefit of eFexTower->eTower alg

    for (auto digi: *scells) {
        Identifier id = digi->ID(); // this is if using supercells

        if (auto elem = ddm->get_element(id); elem && std::abs(elem->eta_raw())<2.5) {
            float eta = elem->eta_raw(); // this seems more symmetric
            int sampling = elem->getSampling();
            if(sampling==6 && ddm->getCaloCell_ID()->region(id)==0 && eta<0) eta-=0.01; // nudge this L2 endcap supercell into correct tower (right on boundary)

            unsigned long long val = id.get_compact();

            int towerid = -1;int slot = -1;bool issplit = false;
            CHECK(m_eFEXSuperCellTowerIdProviderTool->geteTowerIDandslot(id.get_compact(), towerid, slot, issplit));
            eTowerSlots[id.get_compact()] = slot;

            auto& sc = towers[std::pair(etaIndex(eta),phiIndex(elem->phi_raw()))];
            switch(sampling) {
                case 0: case 4: //lar barrel/endcap presampler
                    sc.ps.push_back(val);
                    break;
                case 1: case 5: //lar barrel/endcap l1
                    sc.l1.push_back({elem->eta(),val}); break;
                case 2: case 6: //lar barrel/endcap l2
                    sc.l2.push_back({elem->eta(),val}); break;
                case 3: case 7: //lar barrel/endcap l3
                    sc.l3.push_back(val); break;
                case 8: case 9: case 10: case 11: //lar hec
                    sc.had.push_back(val); break;
                default:
                    sc.other.push_back(val); break;
            }
        }
    }


    // sort (by increasing eta) l1/l2 sc and handle special cases
    // finally also output the eTower slot vector
    std::vector<size_t> slotVector(11);
    for(auto& [coord,sc] : towers) {
        std::sort(sc.l1.begin(),sc.l1.end());
        std::sort(sc.l2.begin(),sc.l2.end());
        // we have 5 l2 cells @ |eta|=1.45 ... put lowest |eta| one in l3 slot
        if (sc.l2.size()==5) {
            if (coord.first >= 0) {
                sc.l3.push_back(sc.l2.front().second);
                sc.l2.erase(sc.l2.begin()); // remove first
            } else {
                sc.l3.push_back(sc.l2.back().second);
                sc.l2.resize(sc.l2.size()-1); // remove last
            }
        }
        if (std::abs(coord.first)==15) { //|eta| = 1.45
            // in the overlap region it seems like the latome id with highest |eta| is swapped with next highest
            // so to compare we swap the first and second (3rd and 4th are fine) if eta < 0, or 3rd and 4th if eta > 0
            if (coord.first<0) {std::swap(sc.l1.at(0),sc.l1.at(1)); }
            else {std::swap(sc.l1.at(2),sc.l1.at(3));}
        }
        // handle case @ |eta|~1.8-2 with 6 L1 cells
        if (sc.l1.size()==6) {
            m_scMap[sc.l1.at(0).second] = std::pair(coord,std::pair(1,11));
            m_scMap[sc.l1.at(1).second] = std::pair(coord,(doV6Mapping && coord.first < 0) ? std::pair(2,1) : std::pair(1,2)); // in LATOME v5 FW this was (1,2) for both sides
            m_scMap[sc.l1.at(2).second] = std::pair(coord,std::pair(2,11));
            m_scMap[sc.l1.at(3).second] = std::pair(coord,std::pair(3,11));
            m_scMap[sc.l1.at(4).second] = std::pair(coord,(doV6Mapping && coord.first < 0) ? std::pair(4,3) : std::pair(3,4)); // in LATOME v5 FW this was (3,4) for both sides
            m_scMap[sc.l1.at(5).second] = std::pair(coord,std::pair(4,11));
            slotVector[1] = eTowerSlots[sc.l1.at(0).second];
            slotVector[2] = eTowerSlots[sc.l1.at(2).second];
            slotVector[3] = eTowerSlots[sc.l1.at(3).second];
            slotVector[4] = eTowerSlots[sc.l1.at(5).second];
        }

        // for |eta|>2.4 there's only 1 l1 sc, to match hardware this should be compared placed in the 'last' l1 input
        if (sc.l1.size()==1) {
            m_scMap[sc.l1.at(0).second] = std::pair(coord,std::pair(4,11));
            slotVector[1] = 1; slotVector[2] = 2; slotVector[3] = 3; slotVector[4] = eTowerSlots[sc.l1.at(0).second];
        }

        // fill the map with sc ids -> tower coord + slot
        if (!sc.ps.empty()) {m_scMap[sc.ps.at(0)] = std::pair(coord,std::pair(0,11)); slotVector[0] = eTowerSlots[sc.ps.at(0)]; }
        if(sc.l1.size()==4) for(size_t i=0;i<4;i++) if(sc.l1.size() > i) {m_scMap[sc.l1.at(i).second] = std::pair(coord,std::pair(i+1,11)); slotVector[i+1] = eTowerSlots[sc.l1.at(i).second]; }
        for(size_t i=0;i<4;i++) if(sc.l2.size() > i) { m_scMap[sc.l2.at(i).second] = std::pair(coord,std::pair(i+5,11)); slotVector[i+5] = eTowerSlots[sc.l2.at(i).second]; }
        if (!sc.l3.empty()) {m_scMap[sc.l3.at(0)] = std::pair(coord,std::pair(9,11)); slotVector[9] = eTowerSlots[sc.l3.at(0)]; }
        if (!sc.had.empty()) {m_scMap[sc.had.at(0)] = std::pair(coord,std::pair(10,11));slotVector[10] = eTowerSlots[sc.had.at(0)]; }

        // finally output the slotVector for this tower
        // do only for the slots that don't match
        // note to self: seems like everything is fine apart from the l1->ps remap for |eta|>2.4
        // so leaving this bit commented out for now ... useful to leave it here in case need to recheck in future
//        for(size_t i=0;i<slotVector.size();i++) {
//            if(slotVector[i] != i) {
//                std::cout << coord.first << "," << coord.second << "," << i << "," << slotVector[i] << std::endl;
//            }
//        }
    }

    // save the map to disk, if required
    if(!m_mappingFile.empty()) {
        TFile f(m_mappingFile.value().c_str(), "RECREATE");
        TTree *t = new TTree("mapping", "mapping");
        unsigned long long scid = 0;
        std::pair<int, int> coord = {0, 0};
        std::pair<int, int> slot = {-1, -1};
        t->Branch("scid", &scid);
        t->Branch("etaIndex", &coord.first);
        t->Branch("phiIndex", &coord.second);
        t->Branch("slot1", &slot.first);
        t->Branch("slot2", &slot.second);
        for (auto &[id, val]: m_scMap) {
            scid = id;
            coord = val.first;
            slot = val.second;
            t->Fill();
        }
        t->Write();
        f.Close();
    }
    return StatusCode::SUCCESS;

}


StatusCode eFexTowerBuilder::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("Executing " << name() << "...");
    setFilterPassed(true, ctx);


    {
        std::lock_guard lock(m_fillMapMutex);
        if (m_scMap.empty()) CHECK( fillMap(ctx) );
    }

    return fillTowers(ctx);

}

} // LVL1 Namespace