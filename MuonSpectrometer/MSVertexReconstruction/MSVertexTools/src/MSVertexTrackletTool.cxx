/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MSVertexTrackletTool.h"

#include "TMath.h"
#include "CxxUtils/trapping_fp.h"

/*
  Tracklet reconstruction tool
  See documentation at https://cds.cern.ch/record/1455664 and https://cds.cern.ch/record/1520894
*/

namespace Muon {

    //** ----------------------------------------------------------------------------------------------------------------- **//

    // Delta Alpha Constants -- p = k/(delta_alpha)
    // inner and outer small stations don't have a from data determined constant. Instead a default value for DeltaAlpgaCut, momentum and momentum error is used.
    constexpr double c_BIL = 28.4366;  // MeV*mrad
    constexpr double c_BMS = 53.1259;  // MeV*mrad
    constexpr double c_BML = 62.8267;  // MeV*mrad
    constexpr double c_BOL = 29.7554;  // MeV*mrad
    //** ----------------------------------------------------------------------------------------------------------------- **//

    MSVertexTrackletTool::MSVertexTrackletTool(const std::string& type, const std::string& name, const IInterface* parent) :
        AthAlgTool(type, name, parent) {
        declareInterface<IMSVertexTrackletTool>(this);
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    StatusCode MSVertexTrackletTool::initialize() {
        ATH_CHECK(m_mdtTESKey.initialize());
        ATH_CHECK(m_TPContainer.initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());

        return StatusCode::SUCCESS;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    StatusCode MSVertexTrackletTool::findTracklets(std::vector<Tracklet>& tracklets, const EventContext& ctx) const {
        // record TrackParticle container in StoreGate
        SG::WriteHandle<xAOD::TrackParticleContainer> container(m_TPContainer, ctx);
        ATH_CHECK(container.record(std::make_unique<xAOD::TrackParticleContainer>(), std::make_unique<xAOD::TrackParticleAuxContainer>()));

        // sort the MDT hits into chambers & MLs
        std::vector<std::vector<const Muon::MdtPrepData*> > SortedMdt;

        int nMDT = SortMDThits(SortedMdt, ctx);

        if (nMDT <= 0) { return StatusCode::SUCCESS; }

        if (msgLvl(MSG::DEBUG)) ATH_MSG_DEBUG("MDT hits are selected and sorted");

        // loop over the MDT hits and find segments
        // select the tube combinations to be fit
        /*Select hits in at least 2 layers and require hits be ordered by increasing tube number (see diagrams below).
           ( )( )(3)( )     ( )(3)( )( )     ( )( )( )( )     ( )( )(3)( )     ( )(2)(3)( )
          ( )(2)( )( )     ( )(2)( )( )     ( )(2)(3)( )     ( )(1)(2)( )     ( )(1)( )( )
           (1)( )( )( )     (1)( )( )( )     (1)( )( )( )     ( )( )( )( )     ( )( )( )( )
         Barrel selection criteria: |z_mdt1 - z_mdt2| < m_d12_max (50 mm), |z_mdt1 - z_mdt3| < m_d13_max (80 mm)
         Endcap selection criteria: |r_mdt1 - r_mdt2| < m_d12_max (50 mm), |r_mdt1 - r_mdt3| < m_d13_max (80 mm)
        */

        std::vector<TrackletSegment> segs[6][2][16];  // single ML segment array (indicies [station type][ML][sector]) with station type iterating through barrel inner, middle, outer and then endcap inner, middle, outer
        std::vector<std::vector<const Muon::MdtPrepData*> >::const_iterator ChamberItr = SortedMdt.begin();
        for (; ChamberItr != SortedMdt.end(); ++ChamberItr) {
            std::vector<TrackletSegment> mlsegments;
            std::vector<const Muon::MdtPrepData*>::const_iterator mdt1 = ChamberItr->begin();
            std::vector<const Muon::MdtPrepData*>::const_iterator mdtEnd = ChamberItr->end();
            if (IgnoreMDTChamber(*mdt1)) continue;
    
            // get information about current chamber
            Identifier mdt1_ID = (*mdt1)->identify();
            bool mdt1_isBarrel = m_idHelperSvc->mdtIdHelper().isBarrel(mdt1_ID);
            bool mdt1_isEndcap = m_idHelperSvc->mdtIdHelper().isEndcap(mdt1_ID);
            int sector = m_idHelperSvc->sector(mdt1_ID);
            int maxLayer = m_idHelperSvc->mdtIdHelper().tubeLayerMax(mdt1_ID);
            int ML = m_idHelperSvc->mdtIdHelper().multilayer(mdt1_ID);

            // loop on hits inside the chamber
            for (; mdt1 != mdtEnd; ++mdt1) {
                if (Amg::error((*mdt1)->localCovariance(), Trk::locR) < m_errorCutOff) {
                    ATH_MSG_WARNING("  " << m_idHelperSvc->mdtIdHelper().print_to_string(mdt1_ID) << " with too small error "
                                         << Amg::error((*mdt1)->localCovariance(), Trk::locR));
                    continue;
                }

                int tl1 = m_idHelperSvc->mdtIdHelper().tubeLayer(mdt1_ID);
                if (tl1 == maxLayer) break;  // require hits in at least 2 layers

                // loop on second hits
                std::vector<const Muon::MdtPrepData*>::const_iterator mdt2 = (mdt1 + 1);
                if (mdt2 == mdtEnd) continue;
                Identifier mdt2_ID = (*mdt2)->identify();
                for (; mdt2 != mdtEnd; ++mdt2) {
                    if (Amg::error((*mdt2)->localCovariance(), Trk::locR) < m_errorCutOff) {
                        ATH_MSG_WARNING("  " << m_idHelperSvc->mdtIdHelper().print_to_string(mdt2_ID)
                                             << " with too small error " << Amg::error((*mdt2)->localCovariance(), Trk::locR));
                        continue;
                    }

                    // reject the bad tube combinations
                    int tl2 = m_idHelperSvc->mdtIdHelper().tubeLayer(mdt2_ID);
                    if (mdt1 == mdt2 || (tl2 - tl1) > 1 || (tl2 - tl1) < 0) continue;
                    if ((tl2 - tl1) == 0 && (m_idHelperSvc->mdtIdHelper().tube(mdt2_ID) -
                                             m_idHelperSvc->mdtIdHelper().tube(mdt1_ID)) < 0) continue;
                    // reject bad hit separations
                    if (mdt1_isBarrel && std::abs((*mdt1)->globalPosition().z() - (*mdt2)->globalPosition().z()) > m_d12_max) continue;
                    if (mdt1_isEndcap && std::abs((*mdt1)->globalPosition().perp() - (*mdt2)->globalPosition().perp()) > m_d12_max) continue;

                    // loop on third hits
                    std::vector<const Muon::MdtPrepData*>::const_iterator mdt3 = (mdt2 + 1);
                    if (mdt3 == mdtEnd) continue;
                    Identifier mdt3_ID = (*mdt3)->identify();
                    for (; mdt3 != mdtEnd; ++mdt3) {
                        if (Amg::error((*mdt3)->localCovariance(), Trk::locR) < m_errorCutOff) {
                            ATH_MSG_WARNING("  " << m_idHelperSvc->mdtIdHelper().print_to_string(mdt3_ID)
                                                 << " with too small error " << Amg::error((*mdt3)->localCovariance(), Trk::locR));
                            continue;
                        }

                        // reject the bad tube combinations
                        if (mdt1 == mdt3 || mdt2 == mdt3) continue;
                        int tl3 = m_idHelperSvc->mdtIdHelper().tubeLayer(mdt3_ID);
                        if ((tl3 - tl2) > 1 || (tl3 - tl2) < 0 || (tl3 - tl1) <= 0) continue;
                        if ((tl3 - tl2) == 0 && (m_idHelperSvc->mdtIdHelper().tube(mdt3_ID) -
                                                 m_idHelperSvc->mdtIdHelper().tube(mdt2_ID)) < 0) continue;
                        // reject bad hit separations
                        if (mdt1_isBarrel && std::abs((*mdt1)->globalPosition().z() - (*mdt3)->globalPosition().z()) > m_d13_max) continue;  
                        if (mdt1_isEndcap && std::abs((*mdt1)->globalPosition().perp() - (*mdt3)->globalPosition().perp()) > m_d13_max) continue;

                        // store and fit the good combinations
                        std::vector<const Muon::MdtPrepData*> mdts;
                        mdts.push_back((*mdt1));
                        mdts.push_back((*mdt2));
                        mdts.push_back((*mdt3));
                        std::vector<TrackletSegment> tmpSegs = TrackletSegmentFitter(mdts);
                        for (const TrackletSegment &tmpSeg : tmpSegs) mlsegments.push_back(tmpSeg);
                    }  // end loop on mdt3
                }      // end loop on mdt2
            }          // end loop on mdt1

            // store the reconstructed segments according to station, ML and sector
            // MS region decoded in MuonIdHelpers/MuonIdHelper.h
            int stationRegion = m_idHelperSvc->mdtIdHelper().stationRegion(mdt1_ID);
            if (mdt1_isBarrel){
                if (stationRegion == 0)
                    for (const TrackletSegment &mlsegment : mlsegments) segs[0][ML - 1][sector - 1].push_back(mlsegment); // barrel inner
                else if (stationRegion == 2)
                    for (const TrackletSegment &mlsegment : mlsegments) segs[1][ML - 1][sector - 1].push_back(mlsegment); // barrel middle
                else if (stationRegion == 3)
                    for (const TrackletSegment &mlsegment : mlsegments) segs[2][ML - 1][sector - 1].push_back(mlsegment); // barrel outer
            }
            else if (mdt1_isEndcap){
                if (stationRegion == 0)
                    for (const TrackletSegment &mlsegment : mlsegments) segs[3][ML - 1][sector - 1].push_back(mlsegment); // endcap inner
                else if (stationRegion == 2)
                    for (const TrackletSegment &mlsegment : mlsegments) segs[4][ML - 1][sector - 1].push_back(mlsegment); // endcap middle
                else if (stationRegion == 3)
                    for (const TrackletSegment &mlsegment : mlsegments) segs[5][ML - 1][sector - 1].push_back(mlsegment); // endcap outer
            }   
            else
                ATH_MSG_WARNING("Found segments belonging to chamber " << m_idHelperSvc->mdtIdHelper().stationNameString(m_idHelperSvc->mdtIdHelper().stationName(mdt1_ID)) << " that have not been stored");
        }  // end loop on mdt chambers

        // Combine/remove duplicate segments
        std::vector<TrackletSegment> CleanSegs[6][2][16];
        for (int st = 0; st < 6; ++st) {
            for (int ml = 0; ml < 2; ++ml) {
                for (int sector = 0; sector < 16; ++sector) {
                    if (!segs[st][ml][sector].empty()) {
                        CleanSegs[st][ml][sector] = CleanSegments(segs[st][ml][sector]);
                    }
                }
            }
        }

        // loop over TrackletSegments in barrel inner, middle, outer and endcap inner, middle, outer stations
        for (int st = 0; st < 6; ++st) {
            double DeltaAlphaCut = m_BarrelDeltaAlphaCut;
            for (int sector = 0; sector < 16; ++sector) {
                for (const TrackletSegment &ML1seg : CleanSegs[st][0][sector]) {
                    // Set the delta alpha cut depending on station type
                    const Identifier trkID = ML1seg.getIdentifier();
                    bool isBarrel = m_idHelperSvc->mdtIdHelper().isBarrel(trkID);
                    bool isSmall = m_idHelperSvc->mdtIdHelper().isSmall(trkID); 
                    int stationRegion = m_idHelperSvc->mdtIdHelper().stationRegion(trkID);

                    if (isBarrel){
                        if (stationRegion == 0){
                            if (isSmall) DeltaAlphaCut = m_BarrelDeltaAlphaCut; // default value for BIS
                            else DeltaAlphaCut = c_BIL / 750.0;
                        } 
                        else if (stationRegion == 2){
                            if (isSmall) DeltaAlphaCut = c_BMS / 750.0;
                            else DeltaAlphaCut = c_BML / 750.0;
                        }
                        else if (stationRegion == 3){
                            if (isSmall) DeltaAlphaCut = m_BarrelDeltaAlphaCut; // default value for BOS
                            else DeltaAlphaCut = c_BOL / 750.0;
                        }
                    }
                    else{
                        DeltaAlphaCut = m_EndcapDeltaAlphaCut;
                    } 

                    // loop on ML2 segments from same sector
                    for (const TrackletSegment &ML2seg : CleanSegs[st][1][sector]) {
                        if (ML1seg.mdtChamber() != ML2seg.mdtChamber() || ML1seg.mdtChEta() != ML2seg.mdtChEta()) continue;

                        double deltaAlpha = ML1seg.alpha() - ML2seg.alpha();
                        bool goodDeltab = DeltabCalc(ML1seg, ML2seg);
                        // select the good combinations
                        if (std::abs(deltaAlpha) < DeltaAlphaCut && goodDeltab) {
                            if (isBarrel) {
                                // barrel chambers
                                double charge_discriminant = deltaAlpha * ML1seg.globalPosition().z() * std::tan(ML1seg.alpha());
                                double charge = charge_discriminant < 0 ? -1 : 1;
                           
                                double pTot = TrackMomentum(ML1seg.getIdentifier(), deltaAlpha);
                                if (pTot < m_minpTot) continue;
                                if (pTot > m_maxpTot) {
                                    // if we find a straight track, try to do a global refit to minimize the number of duplicates
                                    charge = 0;
                                    std::vector<const Muon::MdtPrepData*> mdts = ML1seg.mdtHitsOnTrack();
                                    std::vector<const Muon::MdtPrepData*> mdts2 = ML2seg.mdtHitsOnTrack();
                                    for (const Muon::MdtPrepData *mdt2 : mdts2) mdts.push_back(mdt2);
                                    std::vector<TrackletSegment> CombinedSeg = TrackletSegmentFitter(mdts);

                                    if (!CombinedSeg.empty()) {
                                        // calculate momentum components & uncertainty
                                        double Trk1overPErr = TrackMomentumError(CombinedSeg[0]);
                                        double pT = pTot * std::sin(CombinedSeg[0].alpha());
                                        double pz = pTot * std::cos(CombinedSeg[0].alpha());
                                        Amg::Vector3D momentum(pT * std::cos(CombinedSeg[0].globalPosition().phi()),
                                                               pT * std::sin(CombinedSeg[0].globalPosition().phi()), 
                                                               pz);
                                        // create the error matrix
                                        AmgSymMatrix(5) matrix;
                                        matrix.setIdentity();
                                        matrix(0, 0) = std::pow(CombinedSeg[0].rError(),2);  // delta locR
                                        matrix(1, 1) = std::pow(CombinedSeg[0].zError(),2);  // delta locz
                                        matrix(2, 2) = std::pow(0.00000000001,2);  // delta phi (~0 because we explicitly rotate all tracklets into
                                                                           // the middle of the chamber)
                                        matrix(3, 3) = std::pow(CombinedSeg[0].alphaError(),2);  // delta theta
                                        matrix(4, 4) = std::pow(Trk1overPErr,2);                 // delta 1/p
                                        Tracklet tmpTrk(CombinedSeg[0], momentum, matrix, charge);
                                        ATH_MSG_DEBUG("Track " << tracklets.size() << " found with p = (" << momentum.x() << ", "
                                                               << momentum.y() << ", " << momentum.z()
                                                               << ") and |p| = " << tmpTrk.momentum().mag() << " MeV");
                                        tracklets.push_back(tmpTrk);
                                    }
                                } else {
                                    // tracklet has a measurable momentum
                                    double Trk1overPErr = TrackMomentumError(ML1seg, ML2seg);
                                    double pT = pTot * std::sin(ML1seg.alpha());
                                    double pz = pTot * std::cos(ML1seg.alpha());
                                    Amg::Vector3D momentum(pT * std::cos(ML1seg.globalPosition().phi()),
                                                           pT * std::sin(ML1seg.globalPosition().phi()), 
                                                           pz);
                                    // create the error matrix
                                    AmgSymMatrix(5) matrix;
                                    matrix.setIdentity();
                                    matrix(0, 0) = std::pow(ML1seg.rError(),2);  // delta locR
                                    matrix(1, 1) = std::pow(ML1seg.zError(),2);  // delta locz
                                    matrix(2, 2) = std::pow(0.00000000001,2);  // delta phi (~0 because we explicitly rotate all tracks into the
                                                                       // middle of the chamber)
                                    matrix(3, 3) = std::pow(ML1seg.alphaError(),2);  // delta theta
                                    matrix(4, 4) = std::pow(Trk1overPErr,2);                                  // delta 1/p
                                    Tracklet tmpTrk(ML1seg, ML2seg, momentum, matrix, charge);
                                    ATH_MSG_DEBUG("Track " << tracklets.size() << " found with p = (" << momentum.x() << ", "
                                                           << momentum.y() << ", " << momentum.z()
                                                           << ") and |p| = " << tmpTrk.momentum().mag() << " MeV");
                                    tracklets.push_back(tmpTrk);
                                }
                            } // end barrel chamber selection
                            else if (!isBarrel) {  
                                // endcap tracklets
                                // always straight tracklets (no momentum measurement possible)
                                std::vector<const Muon::MdtPrepData*> mdts = ML1seg.mdtHitsOnTrack();
                                std::vector<const Muon::MdtPrepData*> mdts2 = ML2seg.mdtHitsOnTrack();
                                for (const Muon::MdtPrepData *mdt2 : mdts2) mdts.push_back(mdt2);
                                std::vector<TrackletSegment> CombinedSeg = TrackletSegmentFitter(mdts);

                                if (!CombinedSeg.empty()) {
                                    double charge = 0;
                                    double pTot = m_straightTrackletpTot;
                                    double pT = pTot * std::sin(CombinedSeg[0].alpha());
                                    double pz = pTot * std::cos(CombinedSeg[0].alpha());
                                    Amg::Vector3D momentum(pT * std::cos(CombinedSeg[0].globalPosition().phi()),
                                                           pT * std::sin(CombinedSeg[0].globalPosition().phi()), 
                                                           pz);
                                    // create the error matrix
                                    AmgSymMatrix(5) matrix;
                                    matrix.setIdentity();
                                    matrix(0, 0) = std::pow(CombinedSeg[0].rError(),2);  // delta locR
                                    matrix(1, 1) = std::pow(CombinedSeg[0].zError(),2);  // delta locz
                                    matrix(2, 2) = std::pow(0.0000001,2);  // delta phi (~0 because we explicitly rotate all tracks into the middle
                                                                   // of the chamber)
                                    matrix(3, 3) = std::pow(CombinedSeg[0].alphaError(),2);  // delta theta
                                    matrix(4, 4) = std::pow(m_straightTrackletInvPerr,2);  // delta 1/p (endcap tracks are straight lines with no momentum that we can measure ...)
                                  
                                    Tracklet tmpTrk(CombinedSeg[0], momentum, matrix, charge);
                                    tracklets.push_back(tmpTrk);
                                }
                            }  // end endcap tracklet selection

                        }  // end tracklet selection (delta alpha & delta b)

                    }  // end loop on ML2 segments
                }      // end loop on ML1 segments
            }          // end loop on sectors
        }              // end loop on stations

        // Resolve any ambiguous tracklets
        tracklets = ResolveAmbiguousTracklets(tracklets);

        // convert from tracklets to Trk::Tracks
        convertToTrackParticles(tracklets, container);

        return StatusCode::SUCCESS;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    void MSVertexTrackletTool::convertToTrackParticles(std::vector<Tracklet>& tracklets,
                                                       SG::WriteHandle<xAOD::TrackParticleContainer>& container) {
        // convert tracklets to xAOD::TrackParticle and store in a TrackCollection
        for (Tracklet &tracklet : tracklets) {
            xAOD::TrackParticle* trackparticle = new xAOD::TrackParticle();
            tracklet.setTrackParticle(trackparticle);
            container->push_back(trackparticle);

            AmgSymMatrix(5) covariance{tracklet.errorMatrix()};
            auto MyPerigee(std::make_unique<Trk::Perigee>(tracklet.globalPosition(), tracklet.momentum(), tracklet.charge(), Trk::PerigeeSurface(Amg::Vector3D::Zero()), covariance));

            // fill the xAOD::TrackParticle with the tracklet content
            trackparticle->setDefiningParameters(MyPerigee->parameters()[Trk::d0], MyPerigee->parameters()[Trk::z0],
                                                 MyPerigee->parameters()[Trk::phi0], MyPerigee->parameters()[Trk::theta],
                                                 MyPerigee->parameters()[Trk::qOverP]);                  
            trackparticle->setFitQuality(1., (float)tracklet.mdtHitsOnTrack().size());
            trackparticle->setTrackProperties(xAOD::TrackProperties::LowPtTrack);
            std::vector<float> covMatrixVec;
            Amg::compress(covariance, covMatrixVec);
            trackparticle->setDefiningParametersCovMatrixVec(covMatrixVec);
        }
        return;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    bool MSVertexTrackletTool::IgnoreMDTChamber(const Muon::MdtPrepData* mdtHit) const {
        // return true if the MDT hit is in a chamber to be ignored. These hits are then not used to reconstruct tracklets. 

        bool ignore = false;
        int stName = m_idHelperSvc->mdtIdHelper().stationName(mdtHit->identify());
        int stEta = m_idHelperSvc->mdtIdHelper().stationEta(mdtHit->identify());

        // Doesn't consider hits belonging to chambers BEE, EEL and EES
        if (stName == m_idHelperSvc->mdtIdHelper().stationNameIndex("BEE") || 
            stName == m_idHelperSvc->mdtIdHelper().stationNameIndex("EEL") || 
            stName == m_idHelperSvc->mdtIdHelper().stationNameIndex("EES")) ignore = true;

        // Doesn't consider hits belonging to chambers BIS7/8
        if (stName == m_idHelperSvc->mdtIdHelper().stationNameIndex("BIS") && std::abs(stEta) >= 7) ignore = true;

        // Doesn't consider hits belonging to BME or BMG chambers
        if (stName == m_idHelperSvc->mdtIdHelper().stationNameIndex("BME") || 
            stName == m_idHelperSvc->mdtIdHelper().stationNameIndex("BMG")) ignore = true;

        return ignore;
    } 

    //** ----------------------------------------------------------------------------------------------------------------- **//

    int MSVertexTrackletTool::SortMDThits(std::vector<std::vector<const Muon::MdtPrepData*> >& SortedMdt, const EventContext& ctx) const {
        SortedMdt.clear();
        int nMDT(0);

        SG::ReadHandle<Muon::MdtPrepDataContainer> mdtTES(m_mdtTESKey, ctx);
        if (!mdtTES.isValid()) {
            if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "Muon::MdtPrepDataContainer with key MDT_DriftCircles was not retrieved" << endmsg;
            return 0;
        } else {
            if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "Muon::MdtPrepDataContainer with key MDT_DriftCircles retrieved" << endmsg;
        }

        // iterators over collections, a collection corresponds to a chamber
        for (const Muon::MdtPrepDataCollection* MDTch : *mdtTES){
            if (MDTch->empty()) continue;
            if (IgnoreMDTChamber(*(MDTch->begin()))) continue;

            // sort per multi layer
            std::vector<const Muon::MdtPrepData*> hitsML1;
            std::vector<const Muon::MdtPrepData*> hitsML2;

            // loop on mdt hits in the current chamber
            for (const Muon::MdtPrepData* mdt : *MDTch) {
                // Removes noisy hits
                if (mdt->adc() < 50) continue;
                // Removes dead modules or out of time hits
                if (mdt->status() != Muon::MdtStatusDriftTime) continue;
                // Removes tubes out of readout during drift time or with unphysical errors
                if (mdt->localPosition()[Trk::locR] == 0.) continue;
                if (mdt->localCovariance()(Trk::locR, Trk::locR) < 1e-6) {
                    ATH_MSG_WARNING("Found MDT with unphysical error " << m_idHelperSvc->mdtIdHelper().print_to_string(mdt->identify())
                                                                       << " cov " << mdt->localCovariance()(Trk::locR, Trk::locR));
                    continue;
                }
                ++nMDT;

                // sort per multi layer
                if (m_idHelperSvc->mdtIdHelper().multilayer(mdt->identify()) == 1)
                    hitsML1.push_back(mdt);
                else
                    hitsML2.push_back(mdt);

            }  // end MdtPrepDataCollection

            // add
            addMDTHits(hitsML1, SortedMdt);
            addMDTHits(hitsML2, SortedMdt);
        }  // end MdtPrepDataContainer

        return nMDT;
    }

    void MSVertexTrackletTool::addMDTHits(std::vector<const Muon::MdtPrepData*>& hits,
                                          std::vector<std::vector<const Muon::MdtPrepData*> >& SortedMdt) const {
        if (hits.empty()) return;

        // calculate number of hits in ML
        int ntubes = hits.front()->detectorElement()->getNLayers() * hits.front()->detectorElement()->getNtubesperlayer();
        if (hits.size() > 0.75 * ntubes) return;
        std::sort(hits.begin(), hits.end(), [this](const Muon::MdtPrepData* mprd1, const Muon::MdtPrepData* mprd2) -> bool {
            if (m_idHelperSvc->mdtIdHelper().tubeLayer(mprd1->identify()) > m_idHelperSvc->mdtIdHelper().tubeLayer(mprd2->identify()))
                return false;
            if (m_idHelperSvc->mdtIdHelper().tubeLayer(mprd1->identify()) < m_idHelperSvc->mdtIdHelper().tubeLayer(mprd2->identify()))
                return true;
            if (m_idHelperSvc->mdtIdHelper().tube(mprd1->identify()) < m_idHelperSvc->mdtIdHelper().tube(mprd2->identify())) return true;
            return false;
        });  // sort the MDTs by layer and tube number

        SortedMdt.push_back(hits);
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    std::vector<TrackletSegment> MSVertexTrackletTool::TrackletSegmentFitter(const std::vector<const Muon::MdtPrepData*>& mdts) const {
        // fits TrackletSegments from an as compatible identified set of MDT hits
        // create the segment seeds
        std::vector<std::pair<double, double> > SeedParams = SegSeeds(mdts);
        // fit the segments
        std::vector<TrackletSegment> segs = TrackletSegmentFitterCore(mdts, SeedParams);

        return segs;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    std::vector<std::pair<double, double> > MSVertexTrackletTool::SegSeeds(const std::vector<const Muon::MdtPrepData*>& mdts) const {
        std::vector<std::pair<double, double> > SeedParams;
        // create seeds by drawing the 4 possible lines tangent to the two outermost drift circles
        // see http://cds.cern.ch/record/620198 (section 4.3) for description of the algorithm
        // keep all seeds which satisfy the criterion: residual(mdt 2) < m_SeedResidual
        // NOTE: here there is an assumption that each MDT has a radius of 30mm
        //      -- needs to be revisited when the small tubes in sectors 12 & 14 are installed
        double x1 = mdts.front()->globalPosition().z();
        double y1 = mdts.front()->globalPosition().perp();
        double r1 = std::abs(mdts.front()->localPosition()[Trk::locR]);

        double x2 = mdts.back()->globalPosition().z();
        double y2 = mdts.back()->globalPosition().perp();
        double r2 = std::abs(mdts.back()->localPosition()[Trk::locR]);

        double DeltaX = x2 - x1;
        double DeltaY = y2 - y1;
        double DistanceOfCenters = std::hypot(DeltaX, DeltaY);
        if (DistanceOfCenters < 30) return SeedParams;
        double Alpha0 = std::acos(DeltaX / DistanceOfCenters);

        // First seed
        double phi = mdts.front()->globalPosition().phi();
        double RSum = r1 + r2;
        if (RSum > DistanceOfCenters) return SeedParams;
        double Alpha1 = std::asin(RSum / DistanceOfCenters);
        double line_theta = Alpha0 + Alpha1;
        double z_line = x1 + r1 * std::sin(line_theta);
        double rho_line = y1 - r1 * std::cos(line_theta);

        Amg::Vector3D gPos1(rho_line * std::cos(phi), rho_line * std::sin(phi), z_line);
        Amg::Vector3D gDir(std::cos(phi) * std::sin(line_theta), std::sin(phi) * std::sin(line_theta), std::cos(line_theta));
        Amg::Vector3D globalDir1(std::cos(phi) * std::sin(line_theta), std::sin(phi) * std::sin(line_theta), std::cos(line_theta));
        double gSlope1 = (globalDir1.perp() / globalDir1.z());
        double gInter1 = gPos1.perp() - gSlope1 * gPos1.z();
        double resid = SeedResiduals(mdts, gSlope1, gInter1);
        if (resid < m_SeedResidual) SeedParams.emplace_back(gSlope1, gInter1);
        // Second seed
        line_theta = Alpha0 - Alpha1;
        z_line = x1 - r1 * std::sin(line_theta);
        rho_line = y1 + r1 * std::cos(line_theta);
        Amg::Vector3D gPos2(rho_line * std::cos(phi), rho_line * std::sin(phi), z_line);
        Amg::Vector3D globalDir2(std::cos(phi) * std::sin(line_theta), std::sin(phi) * std::sin(line_theta), std::cos(line_theta));
        double gSlope2 = (globalDir2.perp() / globalDir2.z());
        double gInter2 = gPos2.perp() - gSlope2 * gPos2.z();
        resid = SeedResiduals(mdts, gSlope2, gInter2);
        if (resid < m_SeedResidual) SeedParams.emplace_back(gSlope2, gInter2);

        double Alpha2 = std::asin(std::abs(r2 - r1) / DistanceOfCenters);
        if (r1 < r2) {
            // Third seed
            line_theta = Alpha0 + Alpha2;
            z_line = x1 - r1 * std::sin(line_theta);
            rho_line = y1 + r1 * std::cos(line_theta);

            Amg::Vector3D gPos3(rho_line * std::cos(phi), rho_line * std::sin(phi), z_line);
            Amg::Vector3D globalDir3(std::cos(phi) * std::sin(line_theta), std::sin(phi) * std::sin(line_theta), std::cos(line_theta));
            double gSlope3 = (globalDir3.perp() / globalDir3.z());
            double gInter3 = gPos3.perp() - gSlope3 * gPos3.z();
            resid = SeedResiduals(mdts, gSlope3, gInter3);
            if (resid < m_SeedResidual) SeedParams.emplace_back(gSlope3, gInter3);

            // Fourth seed
            line_theta = Alpha0 - Alpha2;
            z_line = x1 + r1 * std::sin(line_theta);
            rho_line = y1 - r1 * std::cos(line_theta);

            Amg::Vector3D gPos4(rho_line * std::cos(phi), rho_line * std::sin(phi), z_line);
            Amg::Vector3D globalDir4(std::cos(phi) * std::sin(line_theta), std::sin(phi) * std::sin(line_theta), std::cos(line_theta));
            double gSlope4 = (globalDir4.perp() / globalDir4.z());
            double gInter4 = gPos4.perp() - gSlope4 * gPos4.z();
            resid = SeedResiduals(mdts, gSlope4, gInter4);
            if (resid < m_SeedResidual) SeedParams.emplace_back(gSlope4, gInter4);
        } else {
            // Third seed
            line_theta = Alpha0 + Alpha2;
            z_line = x1 + r1 * std::sin(line_theta);
            rho_line = y1 - r1 * std::cos(line_theta);

            Amg::Vector3D gPos3(rho_line * std::cos(phi), rho_line * std::sin(phi), z_line);
            Amg::Vector3D globalDir3(std::cos(phi) * std::sin(line_theta), std::sin(phi) * std::sin(line_theta), std::cos(line_theta));
            double gSlope3 = (globalDir3.perp() / globalDir3.z());
            double gInter3 = gPos3.perp() - gSlope3 * gPos3.z();
            resid = SeedResiduals(mdts, gSlope3, gInter3);
            if (resid < m_SeedResidual) SeedParams.emplace_back(gSlope3, gInter3);

            // Fourth seed
            line_theta = Alpha0 - Alpha2;
            z_line = x1 - r1 * std::sin(line_theta);
            rho_line = y1 + r1 * std::cos(line_theta);

            Amg::Vector3D gPos4(rho_line * std::cos(phi), rho_line * std::sin(phi), z_line);
            Amg::Vector3D globalDir4(std::cos(phi) * std::sin(line_theta), std::sin(phi) * std::sin(line_theta), std::cos(line_theta));
            double gSlope4 = (globalDir4.perp() / globalDir4.z());
            double gInter4 = gPos4.perp() - gSlope4 * gPos4.z();
            resid = SeedResiduals(mdts, gSlope4, gInter4);
            if (resid < m_SeedResidual) SeedParams.emplace_back(gSlope4, gInter4);
        }

        return SeedParams;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    double MSVertexTrackletTool::SeedResiduals(const std::vector<const Muon::MdtPrepData*>& mdts, double slope, double inter) {
        // calculate the residual of the MDTs not used to create the seed
        double resid = 0;
        for (const Muon::MdtPrepData *mdt : mdts) {
            double mdtR = mdt->globalPosition().perp();
            double mdtZ = mdt->globalPosition().z();
            double res = std::abs((mdt->localPosition()[Trk::locR] - std::abs((mdtR - inter - slope * mdtZ) / std::hypot(slope, 1))) /
                                (Amg::error(mdt->localCovariance(), Trk::locR)));
            if (res > resid) resid = res;
        }
        return resid;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    std::vector<TrackletSegment> MSVertexTrackletTool::TrackletSegmentFitterCore(const std::vector<const Muon::MdtPrepData*>& mdts,
                                                                                 const std::vector<std::pair<double, double> >& SeedParams) const {
        std::vector<TrackletSegment> segs;

        Identifier mdtID = mdts.at(0)->identify();
        for (const std::pair<double,double> &SeedParam : SeedParams) {
            // Min chi^2 fit from "Precision of the ATLAS Muon Spectrometer" -- M. Woudstra
            // http://cds.cern.ch/record/620198?ln=en (section 4.3)
            double chi2(0);
            double s(0), sz(0), sy(0);
            // loop on the mdt hits, find the weighted center
            for (const Muon::MdtPrepData *prd : mdts) {
                // Tell clang to optimize assuming that FP exceptions can trap.
                // Otherwise, it can vectorize the division, which can lead to
                // spurious division-by-zero traps from unused vector lanes.
                CXXUTILS_TRAPPING_FP;
                const double mdt_y = std::hypot(prd->globalPosition().x(), prd->globalPosition().y());
                const double mdt_z = prd->globalPosition().z();
                const double sigma2 = std::pow(Amg::error(prd->localCovariance(), Trk::locR),2);
                s += 1 / sigma2;
                sz += mdt_z / sigma2;
                sy += mdt_y / sigma2;
            }
            const double yc = sy / s;
            const double zc = sz / s;

            // Find the initial parameters of the fit
            double alpha = std::atan2(SeedParam.first, 1.0);
            if (alpha < 0) alpha += M_PI;
            double dalpha = 0;
            double d = (SeedParam.second - yc + zc * SeedParam.first) * std::cos(alpha);
            double dd = 0;

            // require segments to point to the second ML
            if (std::abs(std::cos(alpha)) > 0.97 && (m_idHelperSvc->mdtIdHelper().isBarrel(mdtID))) continue;
            if (std::abs(std::cos(alpha)) < 0.03 && (m_idHelperSvc->mdtIdHelper().isEndcap(mdtID))) continue;

            // calculate constants used in the fit
            double sPyy(0), sPyz(0), sPyyzz(0);
            for (const Muon::MdtPrepData *prd : mdts) {
                double mdt_y = std::hypot(prd->globalPosition().x(), prd->globalPosition().y());
                double mdt_z = prd->globalPosition().z();
                double sigma2 = std::pow(Amg::error(prd->localCovariance(), Trk::locR),2);
                sPyy += std::pow(mdt_y-yc,2) / sigma2;
                sPyz += (mdt_y - yc) * (mdt_z - zc) / sigma2;
                sPyyzz += ((mdt_y - yc) - (mdt_z - zc)) * ((mdt_y - yc) + (mdt_z - zc)) / sigma2;
            }

            // iterative fit
            int Nitr = 0;
            double deltaAlpha = 0;
            double deltad = 0;
            while (true) {
                double sumRyi(0), sumRzi(0), sumRi(0);
                chi2 = 0;
                ++Nitr;
                const double cos_a = std::cos(alpha);
                const double sin_a = std::sin(alpha);
                for (const Muon::MdtPrepData *prd : mdts) {
                    double mdt_y = prd->globalPosition().perp(); 
                    double mdt_z = prd->globalPosition().z();
                    double yPi = -(mdt_z - zc) * sin_a + (mdt_y - yc) * cos_a - d;
                    double signR = yPi >= 0 ? -1. : 1;
                    double sigma2 = std::pow(Amg::error(prd->localCovariance(), Trk::locR),2);
                    double ri = signR * prd->localPosition()[Trk::locR];
                    sumRyi += ri * (mdt_y - yc) / sigma2;
                    sumRzi += ri * (mdt_z - zc) / sigma2;
                    sumRi += ri / sigma2;
                    chi2 += std::pow(yPi+ri,2) / sigma2;
                }
                double bAlpha = -1 * sPyz + cos_a * (sin_a * sPyyzz + 2 * cos_a * sPyz + sumRzi) + sin_a * sumRyi;
                double AThTh = sPyy + cos_a * (2 * sin_a * sPyz - cos_a * sPyyzz);
                /// The AThTh represent the derivative fed in to the Newton
                /// minimization. If the value is too small the procedure will
                /// diverge anyway let's break the loop
                if (std::abs(AThTh) < 1.e-7) break;
                // the new alpha & d parameters
                double alphaNew = alpha + bAlpha / AThTh;
                double dNew = sumRi / s;
                // the errors
                dalpha = std::sqrt(1 / std::abs(AThTh));
                dd = std::sqrt(1 / s);
                deltaAlpha = std::abs(alphaNew - alpha);
                deltad = std::abs(d - dNew);
                // test if the new segment is different than the previous
                if (deltaAlpha < 5.e-7 && deltad < 5.e-6) break; 
                alpha = alphaNew;
                d = dNew;
                // Guard against infinite loops
                if (Nitr > 10) break;
            }  // end while loop

            // find the chi^2 probability of the segment
            double chi2Prob = TMath::Prob(chi2, mdts.size() - 2);
            // keep only "good" segments
            if (chi2Prob > m_minSegFinderChi2) {
                double z0 = zc - d * std::sin(alpha);
                double dz0 = std::hypot(dd*std::sin(alpha), d*dalpha*std::cos(alpha));
                double y0 = yc + d * std::cos(alpha);
                double dy0 = std::hypot(dd*std::cos(alpha), d*dalpha*std::sin(alpha));
                // find the hit pattern, which side of the wire did the particle pass? (1==Left, 2==Right)
                /*
                    (  )(/O)(  )
                     (./)(  )(  ) == RRL == 221
                    (O/)(  )(  )
                */
                int pattern(0);
                if (mdts.size() > 8)
                    pattern = -1;  // with more then 8 MDTs the pattern is unique
                else {
                    for (unsigned int k = 0; k < mdts.size(); ++k) {
                        int base = std::pow(10, k);
                        double mdtR = std::hypot(mdts.at(k)->globalPosition().x(), mdts.at(k)->globalPosition().y());
                        double mdtZ = mdts.at(k)->globalPosition().z();
                        double zTest = (mdtR - y0) / std::tan(alpha) + z0 - mdtZ;
                        if (zTest > 0)
                            pattern += 2 * base;
                        else
                            pattern += base;
                    }
                }

                // find the position of the tracklet in the global frame
                double mdtPhi = mdts.at(0)->globalPosition().phi();
                Amg::Vector3D segpos(y0 * std::cos(mdtPhi), y0 * std::sin(mdtPhi), z0);
                // create the tracklet
                TrackletSegment MyTrackletSegment{m_idHelperSvc.get(), mdts, segpos, alpha, dalpha, dy0, dz0, pattern};
                segs.push_back(MyTrackletSegment);
                if (pattern == -1) break;  // stop if we find a segment with more than 8 hits (guaranteed to be unique!)
            }
        }  // end loop on segment seeds

        // in case more than 1 segment is reconstructed, check if there are duplicates using the hit patterns
        if (segs.size() > 1) {
            std::vector<TrackletSegment> tmpSegs;
            for (unsigned int i1 = 0; i1 < segs.size(); ++i1) {
                bool isUnique = true;
                int pattern1 = segs.at(i1).getHitPattern();
                for (unsigned int i2 = (i1 + 1); i2 < segs.size(); ++i2) {
                    if (pattern1 == -1) break;
                    int pattern2 = segs.at(i2).getHitPattern();
                    if (pattern1 == pattern2) isUnique = false;
                }
                if (isUnique) tmpSegs.push_back(segs.at(i1));
            }
            segs = tmpSegs;
        }

        // return the unique segments
        return segs;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    std::vector<TrackletSegment> MSVertexTrackletTool::CleanSegments(const std::vector<TrackletSegment>& Segs) const {
        std::vector<TrackletSegment> CleanSegs;
        std::vector<TrackletSegment> segs = Segs; // set of segments to perform cleaning on
        bool keepCleaning(true);
        int nItr(0);

        while (keepCleaning) {
            ++nItr;
            keepCleaning = false;

            for (std::vector<TrackletSegment>::iterator it = segs.begin(); it != segs.end(); ++it) {
                if (it->isCombined()) continue;
                std::vector<TrackletSegment> segsToCombine;
                double tanTh1 = std::tan(it->alpha());
                double r1 = it->globalPosition().perp();
                double zi1 = it->globalPosition().z() - r1 / tanTh1;
                // find all segments with similar parameters & attempt to combine
                for (std::vector<TrackletSegment>::iterator sit = (it + 1); sit != segs.end(); ++sit) {
                    if (sit->isCombined()) continue;
                    if (it->mdtChamber() != sit->mdtChamber()) continue;         // require the segments are in the same chamber
                    if ((it->mdtChEta()) * (sit->mdtChEta()) < 0) continue;      // check both segments are on the same side of the detector
                    if (it->mdtChPhi() != sit->mdtChPhi()) continue;             // in the same sector
                    if (std::abs(it->alpha() - sit->alpha()) > 0.005) continue;  // same trajectory
                    double tanTh2 = std::tan(sit->alpha());
                    double r2 = sit->globalPosition().perp();
                    double zi2 = sit->globalPosition().z() - r2 / tanTh2;
                    // find the distance at the midpoint between the two segments
                    double rmid = (r1 + r2) / 2.;
                    double z1 = rmid / tanTh1 + zi1;
                    double z2 = rmid / tanTh2 + zi2;
                    double zdist = std::abs(z1 - z2);
                    if (zdist < 0.5) {
                        segsToCombine.push_back(*sit);
                        sit->isCombined(true);
                    }
                }  // end sit loop

                // if the segment is unique, keep it
                if (segsToCombine.empty()) {
                    CleanSegs.push_back(*it);
                }
                // else, combine all like segments & refit
                else if (!segsToCombine.empty()) {
                    // create a vector of all unique MDT hits in the segments
                    std::vector<const Muon::MdtPrepData*> mdts = it->mdtHitsOnTrack();
                    for (const TrackletSegment &seg : segsToCombine) {
                        std::vector<const Muon::MdtPrepData*> tmpmdts = seg.mdtHitsOnTrack();
                        for (const Muon::MdtPrepData *tmpprd : tmpmdts){
                            bool isNewHit(true);
                            for (const Muon::MdtPrepData *tmpprd2 : mdts){
                                if (tmpprd->identify() == tmpprd2->identify()) {
                                    isNewHit = false;
                                    break;
                                }
                            }
                            if (isNewHit && Amg::error(tmpprd->localCovariance(), Trk::locR) > m_errorCutOff) mdts.push_back(tmpprd);
                        }
                    }  // end segsToCombine loop

                    // only need to combine if there are extra hits added to the first segment
                    if (mdts.size() > it->mdtHitsOnTrack().size()) {
                        std::vector<TrackletSegment> refitsegs = TrackletSegmentFitter(mdts);
                        // if the refit fails, what to do?
                        if (refitsegs.empty()) {
                            if (segsToCombine.size() == 1) {
                                segsToCombine[0].isCombined(false);
                                CleanSegs.push_back(*it);
                                CleanSegs.push_back(segsToCombine[0]);
                            } else {
                                // loop on the mdts and count the number of segments that share that hit
                                std::vector<int> nSeg;
                                for (unsigned int i = 0; i < mdts.size(); ++i) {
                                    nSeg.push_back(0);
                                    // hit belongs to the first segment
                                    for (unsigned int k = 0; k < it->mdtHitsOnTrack().size(); ++k) {
                                        if (it->mdtHitsOnTrack()[k]->identify() == mdts[i]->identify()) {
                                            ++nSeg[i];
                                            break;
                                        }
                                    }
                                    // hit belongs to one of the duplicate segments
                                    for (unsigned int k = 0; k < segsToCombine.size(); ++k) {
                                        for (unsigned int m = 0; m < segsToCombine[k].mdtHitsOnTrack().size(); ++m) {
                                            if (segsToCombine[k].mdtHitsOnTrack()[m]->identify() == mdts[i]->identify()) {
                                                ++nSeg[i];
                                                break;
                                            }
                                        }  // end loop on mdtHitsOnTrack
                                    }      // end loop on segsToCombine
                                }          // end loop on mdts

                                // loop over the duplicates and remove the MDT used by the fewest segments until the fit converges
                                bool keeprefitting(true);
                                int nItr2(0);
                                while (keeprefitting) {
                                    ++nItr2;
                                    int nMinSeg(nSeg[0]);
                                    const Muon::MdtPrepData* minmdt = mdts[0];
                                    std::vector<int> nrfsegs;
                                    std::vector<const Muon::MdtPrepData*> refitmdts;
                                    // loop on MDTs, identify the overlapping set of hits
                                    for (unsigned int i = 1; i < mdts.size(); ++i) {
                                        if (nSeg[i] < nMinSeg) {
                                            refitmdts.push_back(minmdt);
                                            nrfsegs.push_back(nMinSeg);
                                            minmdt = mdts[i];
                                            nMinSeg = nSeg[i];
                                        } else {
                                            refitmdts.push_back(mdts[i]);
                                            nrfsegs.push_back(nSeg[i]);
                                        }
                                    }
                                    // reset the list of MDTs & the minimum number of segments an MDT must belong to
                                    mdts = refitmdts;
                                    nSeg = nrfsegs;
                                    // try to fit the new set of MDTs
                                    refitsegs = TrackletSegmentFitter(mdts);
                                    if (!refitsegs.empty()) {
                                        for (const TrackletSegment &refitseg : refitsegs) CleanSegs.push_back(refitseg);
                                        keeprefitting = false;  // stop refitting if segments are found
                                    } else if (mdts.size() <= 3) {
                                        CleanSegs.push_back(*it);
                                        keeprefitting = false;
                                    }
                                    if (nItr2 > 10) break;
                                }  // end while
                            }
                        } else {
                            keepCleaning = true;
                            for (const TrackletSegment &refitseg : refitsegs) CleanSegs.push_back(refitseg);
                        }
                    }
                    // if there are no extra MDT hits, keep only the first segment as unique
                    else
                        CleanSegs.push_back(*it);
                }
            }  // end it loop
            if (keepCleaning) {
                segs = CleanSegs;
                CleanSegs.clear();
            }
            if (nItr > 10) break;
        }  // end while

        return CleanSegs;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    bool MSVertexTrackletTool::DeltabCalc(const TrackletSegment& ML1seg, const TrackletSegment& ML2seg) const {
        double ChMid = (ML1seg.getChMidPoint() + ML2seg.getChMidPoint()) / 2.0;
        // Calculate the Delta b (see http://inspirehep.net/record/1266438)
        double mid1(100), mid2(1000);
        double deltab(100);
        if (m_idHelperSvc->mdtIdHelper().isBarrel(ML1seg.getIdentifier())) {
            // delta b in the barrel
            mid1 = (ChMid - ML1seg.globalPosition().perp()) / std::tan(ML1seg.alpha()) + ML1seg.globalPosition().z();
            mid2 = (ChMid - ML2seg.globalPosition().perp()) / std::tan(ML2seg.alpha()) + ML2seg.globalPosition().z();
            double r01 = ML1seg.globalPosition().perp() - ML1seg.globalPosition().z() * std::tan(ML1seg.alpha());
            double r02 = ML2seg.globalPosition().perp() - ML2seg.globalPosition().z() * std::tan(ML2seg.alpha());
            deltab = (mid2 * std::tan(ML1seg.alpha()) - ChMid + r01) / (std::hypot(1,std::tan(ML1seg.alpha())));
            double deltab2 = (mid1 * std::tan(ML2seg.alpha()) - ChMid + r02) / (std::hypot(1,std::tan(ML2seg.alpha())));
            if (std::abs(deltab2) < std::abs(deltab)) deltab = deltab2;
        } else {
            // delta b in the endcap
            mid1 = ML1seg.globalPosition().perp() + std::tan(ML1seg.alpha()) * (ChMid - ML1seg.globalPosition().z());
            mid2 = ML2seg.globalPosition().perp() + std::tan(ML2seg.alpha()) * (ChMid - ML2seg.globalPosition().z());
            double z01 = ML1seg.globalPosition().z() - ML1seg.globalPosition().perp() / std::tan(ML1seg.alpha());
            double z02 = ML1seg.globalPosition().z() - ML1seg.globalPosition().perp() / std::tan(ML1seg.alpha());
            deltab = (mid2 / std::tan(ML1seg.alpha()) - ChMid + z01) / (std::hypot(1,1/std::tan(ML1seg.alpha())));
            double deltab2 = (mid1 / std::tan(ML2seg.alpha()) - ChMid + z02) / (std::hypot(1,1/std::tan(ML2seg.alpha())));
            if (std::abs(deltab2) < std::abs(deltab)) deltab = deltab2;
        }

        // calculate the maximum allowed Delta b based on delta alpha uncertainties and ML spacing
        double dbmax = 5 * std::abs(ChMid - ML1seg.getChMidPoint()) * std::hypot(ML1seg.alphaError(), ML2seg.alphaError());
        if (dbmax > m_maxDeltabCut) dbmax = m_maxDeltabCut;
        return std::abs(deltab) < dbmax;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    double MSVertexTrackletTool::TrackMomentum(const Identifier trkID, const double deltaAlpha) const {
        // p = k/delta_alpha
        bool isBarrel = m_idHelperSvc->mdtIdHelper().isBarrel(trkID);
        bool isSmall = m_idHelperSvc->mdtIdHelper().isSmall(trkID);
        int stationRegion = m_idHelperSvc->mdtIdHelper().stationRegion(trkID);

        double dalpha = std::abs(deltaAlpha);
        double pTot = m_straightTrackletpTot;
        if (isBarrel){
            if (stationRegion == 0){
                if (isSmall) pTot = m_straightTrackletpTot; // default value for BIS
                else pTot = c_BIL / dalpha;
            } 
            else if (stationRegion == 2){
                if (isSmall) pTot = c_BMS / dalpha;
                else pTot = c_BML / dalpha;
            }
            else if (stationRegion == 3){
                if (isSmall) pTot = m_straightTrackletpTot; // default value for BOS
                else pTot = c_BOL / dalpha;
            }
        }
        
        if (pTot > m_maxpTot) pTot = m_straightTrackletpTot;

        return pTot;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    double MSVertexTrackletTool::TrackMomentumError(const TrackletSegment& ml1, const TrackletSegment& ml2) const{
        // uncertainty on 1/p
        const Identifier trkID = ml1.getIdentifier();
        bool isBarrel = m_idHelperSvc->mdtIdHelper().isBarrel(trkID);
        bool isSmall = m_idHelperSvc->mdtIdHelper().isSmall(trkID);
        int stationRegion = m_idHelperSvc->mdtIdHelper().stationRegion(trkID);
        
        double dalpha = std::hypot(ml1.alphaError(), ml2.alphaError());
        double pErr = dalpha / c_BML;
        if (isBarrel){
            if (stationRegion == 0){
                if (isSmall) pErr = dalpha / c_BML; // default value for BIS
                else pErr = dalpha / c_BIL;
            } 
            else if (stationRegion == 2){
                if (isSmall) pErr = dalpha / c_BMS;
                else pErr = dalpha / c_BML;
            }
            else if (stationRegion == 3){
                if (isSmall) pErr = dalpha / c_BML; // default value for BOS
                else pErr = dalpha / c_BOL;
            }
        }

        return pErr;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    double MSVertexTrackletTool::TrackMomentumError(const TrackletSegment& ml1) const {
        // uncertainty in 1/p
        const Identifier trkID = ml1.getIdentifier();
        bool isBarrel = m_idHelperSvc->mdtIdHelper().isBarrel(trkID);
        bool isSmall = m_idHelperSvc->mdtIdHelper().isSmall(trkID);
        int stationRegion = m_idHelperSvc->mdtIdHelper().stationRegion(trkID);
        
        double dalpha = std::abs(ml1.alphaError());
        double pErr = dalpha / c_BML;
        if (isBarrel){
            if (stationRegion == 0){
                if (isSmall) pErr = dalpha / c_BML; // default value for BIS
                else pErr = dalpha / c_BIL;
            } 
            else if (stationRegion == 2){
                if (isSmall) pErr = dalpha / c_BMS;
                else pErr = dalpha / c_BML;
            }
            else if (stationRegion == 3){
                if (isSmall) pErr = dalpha / c_BML; // default value for BOS
                else pErr = dalpha / c_BOL;
            }
        }

        return pErr;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    std::vector<Tracklet> MSVertexTrackletTool::ResolveAmbiguousTracklets(std::vector<Tracklet>& tracks) const {
        ATH_MSG_DEBUG("In ResolveAmbiguousTracks");
        // considering only tracklets with the number of associated hits
        // being more than 3/4 the number of layers in the MS chamber

        if (m_tightTrackletRequirement) {
            std::vector<Tracklet> myTracks = tracks;
            tracks.clear();
            for (const Tracklet &track : myTracks) {
                Identifier id1 = track.getML1seg().mdtHitsOnTrack().at(0)->identify();
                Identifier id2 = track.getML2seg().mdtHitsOnTrack().at(0)->identify();
                int nLayerML1 = m_idHelperSvc->mdtIdHelper().tubeLayerMax(id1);
                int nLayerML2 = m_idHelperSvc->mdtIdHelper().tubeLayerMax(id2);
                double ratio = (double)(track.mdtHitsOnTrack().size()) / (nLayerML1 + nLayerML2);
                if (ratio > 0.75) tracks.push_back(track);
            }
        }

        std::vector<Tracklet> UniqueTracks;
        std::vector<unsigned int> AmbigTrks; // indices of ambigious tracklets
        for (unsigned int tk1 = 0; tk1 < tracks.size(); ++tk1) {
            int nShared = 0;
            // check if any Ambiguity has been broken
            bool isResolved = false;
            for (unsigned int AmbigTrksIdx : AmbigTrks) {
                if (tk1 == AmbigTrksIdx) {
                    isResolved = true;
                    break;
                }
            }
            if (isResolved) continue;
            std::vector<Tracklet> AmbigTracks;
            AmbigTracks.push_back(tracks.at(tk1));
            // get a point on the track
            double Trk1ML1R = tracks.at(tk1).getML1seg().globalPosition().perp();
            double Trk1ML1Z = tracks.at(tk1).getML1seg().globalPosition().z();
            double Trk1ML2R = tracks.at(tk1).getML2seg().globalPosition().perp();
            double Trk1ML2Z = tracks.at(tk1).getML2seg().globalPosition().z();

            Identifier tk1ID = tracks.at(tk1).muonIdentifier();
            bool tk1_isBarrel = m_idHelperSvc->mdtIdHelper().isBarrel(tk1ID);
            bool tk1_isEndcap = m_idHelperSvc->mdtIdHelper().isEndcap(tk1ID);

            // loop over the rest of the tracks and find any amibuities
            for (unsigned int tk2 = (tk1 + 1); tk2 < tracks.size(); ++tk2) {
                if (tracks.at(tk1).mdtChamber() == tracks.at(tk2).mdtChamber() && tracks.at(tk1).mdtChPhi() == tracks.at(tk2).mdtChPhi() &&
                    (tracks.at(tk1).mdtChEta()) * (tracks.at(tk2).mdtChEta()) > 0) {
                    // check if any Ambiguity has been broken
                    for (unsigned int AmbigTrksIdx : AmbigTrks) {
                        if (tk2 == AmbigTrksIdx) {
                            isResolved = true;
                            break;
                        }
                    }
                    if (isResolved) continue;
                    // get a point on the track
                    double Trk2ML1R = tracks.at(tk2).getML1seg().globalPosition().perp();
                    double Trk2ML1Z = tracks.at(tk2).getML1seg().globalPosition().z();
                    double Trk2ML2R = tracks.at(tk2).getML2seg().globalPosition().perp();
                    double Trk2ML2Z = tracks.at(tk2).getML2seg().globalPosition().z();

                    // find the distance between the tracks
                    double DistML1(1000), DistML2(1000);
                    if (tk1_isBarrel) {
                        DistML1 = std::abs(Trk1ML1Z - Trk2ML1Z);
                        DistML2 = std::abs(Trk1ML2Z - Trk2ML2Z);
                    } else if (tk1_isEndcap) {
                        DistML1 = std::abs(Trk1ML1R - Trk2ML1R);
                        DistML2 = std::abs(Trk1ML2R - Trk2ML2R);
                    }
                    if (DistML1 < 40 || DistML2 < 40) {
                        // find how many MDTs the tracks share
                        std::vector<const Muon::MdtPrepData*> mdts1 = tracks.at(tk1).mdtHitsOnTrack();
                        std::vector<const Muon::MdtPrepData*> mdts2 = tracks.at(tk2).mdtHitsOnTrack();
                        nShared = 0;
                        for (const Muon::MdtPrepData *mdt1 : mdts1) {
                            for (const Muon::MdtPrepData *mdt2 : mdts2) {
                                if (mdt1->identify() == mdt2->identify()) {
                                    ++nShared;
                                    break;
                                }
                            }
                        }

                        if (nShared <= 1) continue;  // if the tracks share only 1 hits move to next track
                        // store the track as ambiguous
                        AmbigTracks.push_back(tracks.at(tk2));
                        AmbigTrks.push_back(tk2);
                    }
                }  // end chamber match
            }      // end tk2 loop

            if (AmbigTracks.size() == 1) {
                UniqueTracks.push_back(tracks.at(tk1));
                continue;
            }
            // Deal with any ambiguities
            // Barrel tracks
            if (tk1_isBarrel) {
                bool hasMomentum = tracks.at(tk1).charge() != 0; 
                double aveX(0), aveY(0), aveZ(0), aveAlpha(0);
                double aveP(0), nAmbigP(0), TrkCharge(tracks.at(tk1).charge());
                bool allSameSign(true);

                for (const Tracklet &AmbigTrack : AmbigTracks) {
                    if (!hasMomentum) {
                        aveX += AmbigTrack.globalPosition().x();
                        aveY += AmbigTrack.globalPosition().y();
                        aveZ += AmbigTrack.globalPosition().z();
                        aveAlpha += AmbigTrack.getML1seg().alpha();
                    } else {
                        // check the charge is the same
                        if (std::abs(AmbigTrack.charge() - TrkCharge) > 0.1) allSameSign = false;
                        // find the average momentum
                        aveP += AmbigTrack.momentum().mag();
                        ++nAmbigP;
                        aveAlpha += AmbigTrack.alpha();
                        aveX += AmbigTrack.globalPosition().x();
                        aveY += AmbigTrack.globalPosition().y();
                        aveZ += AmbigTrack.globalPosition().z();
                    }
                }  // end loop on ambiguous tracks
                if (!hasMomentum) {
                    aveX = aveX / (double)AmbigTracks.size();
                    aveY = aveY / (double)AmbigTracks.size();
                    aveZ = aveZ / (double)AmbigTracks.size();
                    Amg::Vector3D gpos(aveX, aveY, aveZ);
                    aveAlpha = aveAlpha / (double)AmbigTracks.size();
                    double alphaErr = tracks.at(tk1).getML1seg().alphaError();
                    double rErr = tracks.at(tk1).getML1seg().rError();
                    double zErr = tracks.at(tk1).getML1seg().zError();

                    TrackletSegment aveSegML1(m_idHelperSvc.get(), tracks.at(tk1).getML1seg().mdtHitsOnTrack(), gpos, aveAlpha, alphaErr, rErr, zErr, 0);
                    double pT = m_maxpTot * std::sin(aveSegML1.alpha());
                    double pz = m_maxpTot * std::cos(aveSegML1.alpha());
                    Amg::Vector3D momentum(pT * std::cos(aveSegML1.globalPosition().phi()), 
                                           pT * std::sin(aveSegML1.globalPosition().phi()),
                                           pz);
                    AmgSymMatrix(5) matrix;
                    matrix.setIdentity();
                    matrix(0, 0) = std::pow(tracks.at(tk1).getML1seg().rError(),2);  // delta R
                    matrix(1, 1) = std::pow(tracks.at(tk1).getML1seg().zError(),2);  // delta z
                    matrix(2, 2) = std::pow(0.0000001,2);  // delta phi (~0 because we explicitly rotate all tracks into the middle of the chamber)
                    matrix(3, 3) = std::pow(tracks.at(tk1).getML1seg().alphaError(),2);  // delta theta
                    matrix(4, 4) = std::pow(m_straightTrackletInvPerr,2);                                  // delta 1/p
                    Tracklet aveTrack(aveSegML1, momentum, matrix, 0);
                    UniqueTracks.push_back(aveTrack);
                } else if (allSameSign) {
                    aveP = aveP / nAmbigP;
                    double pT = aveP * std::sin(tracks.at(tk1).getML1seg().alpha());
                    double pz = aveP * std::cos(tracks.at(tk1).getML1seg().alpha());
                    Amg::Vector3D momentum(pT * std::cos(tracks.at(tk1).globalPosition().phi()),
                                           pT * std::sin(tracks.at(tk1).globalPosition().phi()), 
                                           pz);
                    Tracklet MyTrack = tracks.at(tk1);
                    MyTrack.momentum(momentum);
                    MyTrack.charge(tracks.at(tk1).charge());
                    UniqueTracks.push_back(MyTrack);
                } else {
                    aveX = aveX / (double)AmbigTracks.size();
                    aveY = aveY / (double)AmbigTracks.size();
                    aveZ = aveZ / (double)AmbigTracks.size();
                    Amg::Vector3D gpos(aveX, aveY, aveZ);
                    aveAlpha = aveAlpha / (double)AmbigTracks.size();
                    double alphaErr = tracks.at(tk1).getML1seg().alphaError();
                    double rErr = tracks.at(tk1).getML1seg().rError();
                    double zErr = tracks.at(tk1).getML1seg().zError();

                    TrackletSegment aveSegML1(m_idHelperSvc.get(), tracks.at(tk1).getML1seg().mdtHitsOnTrack(), gpos, aveAlpha, alphaErr, rErr, zErr, 0);
                    double pT = m_maxpTot * std::sin(aveSegML1.alpha());
                    double pz = m_maxpTot * std::cos(aveSegML1.alpha());
                    Amg::Vector3D momentum(pT * std::cos(aveSegML1.globalPosition().phi()), 
                                           pT * std::sin(aveSegML1.globalPosition().phi()),
                                           pz);
                    AmgSymMatrix(5) matrix;
                    matrix.setIdentity();
                    matrix(0, 0) = std::pow(tracks.at(tk1).getML1seg().rError(),2);  // delta R
                    matrix(1, 1) = std::pow(tracks.at(tk1).getML1seg().zError(),2);  // delta z
                    matrix(2, 2) = std::pow(0.0000001,2);  // delta phi (~0 because we explicitly rotate all tracks into the middle of the chamber)
                    matrix(3, 3) = std::pow(tracks.at(tk1).getML1seg().alphaError(),2);  // delta theta
                    matrix(4, 4) = std::pow(m_straightTrackletInvPerr,2);                                  // delta 1/p
                    Tracklet aveTrack(aveSegML1, momentum, matrix, 0);
                    UniqueTracks.push_back(aveTrack);
                }
            }  // end barrel Tracks

            // Endcap tracks
            else if (tk1_isEndcap) {
                std::vector<const Muon::MdtPrepData*> AllMdts;
                for (Tracklet const &AmbigTrack : AmbigTracks) {
                    std::vector<const Muon::MdtPrepData*> mdts = AmbigTrack.mdtHitsOnTrack();
                    std::vector<const Muon::MdtPrepData*> tmpAllMdt = AllMdts;
                    for (const Muon::MdtPrepData *mdt : mdts) {
                        bool isNewHit = true;
                        for (const Muon::MdtPrepData *tmpmdt : tmpAllMdt) {
                            if (mdt->identify() == tmpmdt->identify()) {
                                isNewHit = false;
                                break;
                            }
                        }
                        if (isNewHit) AllMdts.push_back(mdt);
                    }  // end loop on mdts
                }      // end loop on ambiguous tracks

                std::vector<TrackletSegment> MyECsegs = TrackletSegmentFitter(AllMdts);
                if (!MyECsegs.empty()) {
                    TrackletSegment ECseg = MyECsegs.at(0);
                    ECseg.clearMdt();
                    double pT = m_maxpTot * std::sin(ECseg.alpha());
                    double pz = m_maxpTot * std::cos(ECseg.alpha());
                    Amg::Vector3D momentum(pT * std::cos(ECseg.globalPosition().phi()),
                                           pT * std::sin(ECseg.globalPosition().phi()), 
                                           pz);
                    AmgSymMatrix(5) matrix;
                    matrix.setIdentity();
                    matrix(0, 0) = std::pow(ECseg.rError(),2);  // delta R
                    matrix(1, 1) = std::pow(ECseg.zError(),2);  // delta z
                    matrix(2, 2) = std::pow(0.0000001,2);  // delta phi (~0 because we explicitly rotate all tracks into the middle of the chamber)
                    matrix(3, 3) = std::pow(ECseg.alphaError(),2);  // delta theta
                    matrix(4, 4) = std::pow(m_straightTrackletInvPerr,2);  // delta 1/p (endcap tracks are straight lines with no momentum that we can measure ...)
                    Tracklet MyCombTrack(MyECsegs.at(0), ECseg, momentum, matrix, 0);
                    UniqueTracks.push_back(MyCombTrack);
                } else
                    UniqueTracks.push_back(tracks.at(tk1));
            }  // end endcap tracks

        }  // end loop on tracks -- tk1

        return UniqueTracks;
    }

}  // namespace Muon
