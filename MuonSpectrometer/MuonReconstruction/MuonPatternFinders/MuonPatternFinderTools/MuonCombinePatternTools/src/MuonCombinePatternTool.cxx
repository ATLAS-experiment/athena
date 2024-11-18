/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCombinePatternTools/MuonCombinePatternTool.h"

#include "CxxUtils/sincos.h"
#include "MuonHoughPatternEvent/MuonHoughPattern.h"
#include "MuonPattern/MuonPatternChamberIntersect.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkSurfaces/Surface.h"  
#include <cmath> //std::abs, M_PI etc
#include "GaudiKernel/SystemOfUnits.h"
#include "EventPrimitives/EventPrimitivesToStringConverter.h"
namespace{
    double rotatePhi(double phi, double rotationFraction) {
        // check whether we rotate to a large value than pi, if so add additional
        // rotation by -2pi
        if (phi + rotationFraction * M_PI > M_PI) return phi + (rotationFraction - 2.) * M_PI;
        return phi + rotationFraction * M_PI;
    }

    struct Unowned{
        void operator()(const Muon::MuonPrdPattern*) {}
    };

    const Amg::Vector3D& globalPos(const Trk::PrepRawData* prd ){
        if (prd->type(Trk::PrepRawDataType::MdtPrepData)) 
            return static_cast<const Muon::MdtPrepData*>(prd)->globalPosition();
        return static_cast<const Muon::MuonCluster*>(prd)->globalPosition();

    }
}
using PrdPatternPair = MuonCombinePatternTool::PrdPatternPair;

MuonCombinePatternTool::MuonCombinePatternTool(const std::string& type, const std::string& name, const IInterface* parent) :
    AthAlgTool(type, name, parent),
    m_maximum_xydistance(3500),
    m_maximum_rzdistance(1500),
    m_use_cosmics(false),
    m_splitpatterns(true),
    m_nodiscarding(true),
    m_bestphimatch(false),
    m_flipdirectionforcosmics(false) {
    declareInterface<IMuonCombinePatternTool>(this);
    declareProperty("UseCosmics", m_use_cosmics);
    declareProperty("SplitPatterns", m_splitpatterns);
    declareProperty("NoDiscarding", m_nodiscarding);
    declareProperty("BestPhiMatch", m_bestphimatch);
    declareProperty("FlipDirectionForCosmics", m_flipdirectionforcosmics);
    declareProperty("UseTightAssociation", m_useTightAssociation = false);
    declareProperty("MaxSizePhiPatternLooseCuts", m_maxSizePhiPatternLoose = 40);
    declareProperty("MaxSizeEtaPatternLooseCuts", m_maxSizeEtaPatternLoose = 200);
}

StatusCode MuonCombinePatternTool::initialize() {
    ATH_MSG_DEBUG("MuonCombinePatternTool::initialize");
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_printer.retrieve());
    if (!m_use_cosmics) { m_splitpatterns = false; }
    if (m_use_cosmics) { m_bestphimatch = true; }
    ATH_MSG_DEBUG(" UseCosmics: " << m_use_cosmics << " Split Patterns: " << m_splitpatterns << " NoDiscarding: " << m_nodiscarding
                                  << " BestPhiMatch: " << m_bestphimatch);
    return StatusCode::SUCCESS;
}

std::unique_ptr<MuonPrdPatternCollection> MuonCombinePatternTool::combineEtaPhiPatterns(
    const MuonPrdPatternCollection& phiPatternCollection, const MuonPrdPatternCollection& etaPatternCollection,
    /** phi eta association map, eta prds are key*/
    const EtaPhiHitAssocMap& phiEtaHitAssMap) const {
    bool myDebug = false;
   
    /// vector of etapatterns (key) and phipatterns (value), that are
    /// candidates for combining. Both etapatterns and phipatterns
    /// can occur multiple times
    std::vector<CandidatePatPair> candidates{};  
    
   
    // strategy
    // associate eta pattern to phi patterns
    // are phi hits on a eta and eta hits on phi pattern?

    // some printout
    ATH_MSG_VERBOSE(" combineEtaPhiPatterns: "
                    <<" eta patterns: " << etaPatternCollection.size()
                    << "  phi patterns: " << phiPatternCollection.size()
                    <<std::endl<<"#################################################################################"
                    <<std::endl<<"Print eta pattern collection "<<std::endl<<m_printer->print(etaPatternCollection)
                    <<std::endl<<"+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"                    
                    <<std::endl<<"Print phi pattern collection "<<std::endl<<m_printer->print(phiPatternCollection)
                    );

    for (unsigned int etalevel = 0; etalevel < etaPatternCollection.size(); etalevel++) {
        CandPrdPatPtr etapattern{etaPatternCollection.at(etalevel), Unowned()};
        if (etapattern->numberOfContainedPrds() == 0) continue;
        const Amg::Vector3D etaPatDir = etapattern->globalDirection().unit();
        const Amg::Vector3D& etaPatPos = etapattern->globalPosition();
        double theta = etaPatDir.theta();
        const double phieta = etaPatDir.phi();
        CxxUtils::sincos scphieta(phieta);
        CxxUtils::sincos sctheta(theta);       
        double z0 = etaPatPos.z();    // 0 for non -cosmics
        double rz0 = z0 * sctheta.sn; // closest distance in rz

        double eta_x = etaPatPos.x();
        double eta_y = etaPatPos.y();
        double pattern_z0 =z0 + etaPatPos.perp() * sctheta.cs / sctheta.sn;  
        // z belonging to (x0,y0) -> noncosmics just close to
        // 0 (0.001 * sin/cos)
        const double eta_r0 = MuonHoughMathUtils::signedDistanceOfLineToOrigin2D(eta_x, eta_y, phieta);
        const double charge = eta_r0>= 0. ? 1. : -1;
        //           Get inverse curvature from eta pattern
        const double curvature = etapattern->globalDirection().mag();
        const double invcurvature = curvature > 2 ?charge / curvature : 0.;
      
        ATH_MSG_DEBUG(" eta pattern info level: " << etalevel << " phi " << phieta << " theta " << theta << " x0 " << eta_x << " y0 "
                                                  << eta_y << " z0 " << z0 << " hits " << etapattern->numberOfContainedPrds());
        // flags for cosmics:
        double min_average_distance = m_maximum_xydistance + m_maximum_rzdistance;
        CandPrdPatPtr max_phipattern = nullptr;
        int max_philevel = -1;
        // flags
        double dotprodbest = -1.;
        int phibest = -1;
        bool ismatched = false;
        for (unsigned int philevel = 0; philevel < phiPatternCollection.size(); philevel++) {
            CandPrdPatPtr phipattern{phiPatternCollection.at(philevel), Unowned()};
            if (phipattern->numberOfContainedPrds() == 0) continue;
            bool useTightAssociation = false;

            if (m_useTightAssociation && (phipattern->numberOfContainedPrds() > m_maxSizePhiPatternLoose ||
                                          etapattern->numberOfContainedPrds() > m_maxSizeEtaPatternLoose)) {
                if (phipattern->numberOfContainedPrds() > m_maxSizePhiPatternLoose)
                    ATH_MSG_DEBUG(" using tight cuts due to phi hits " << phipattern->numberOfContainedPrds() << " cut "
                                                                       << m_maxSizePhiPatternLoose);
                if (etapattern->numberOfContainedPrds() > m_maxSizeEtaPatternLoose)
                    ATH_MSG_DEBUG(" using tight cuts due to eta hits " << etapattern->numberOfContainedPrds() << " cut "
                                                                       << m_maxSizeEtaPatternLoose);
                useTightAssociation = true;
            }

            const Amg::Vector3D phiPatDir = phipattern->globalDirection().unit();
            const double dotprod = phiPatDir.dot(etaPatDir); 

            if (dotprod > dotprodbest) {
                dotprodbest = dotprod;
                phibest = philevel;
            }

            if (!m_use_cosmics) {
                ATH_MSG_DEBUG(" eta nr " << etalevel << " phi nr " << philevel << " inproduct " << dotprod << " sin angle "
                                         << std::sin(std::acos(dotprod)));
            }

            double r0{0.}, phipattern_x{0.}, phipattern_y{0.}, phi{phiPatDir.phi()};
            CxxUtils::sincos scphi{phi};
            if (m_use_cosmics) {
                // new information: update parameters
                std::array<double,4> new_pars = updateParametersForCosmics(*phipattern, *etapattern);
                r0 = new_pars[0];
                phi = new_pars[1];
                rz0 = new_pars[2];
                theta = new_pars[3];
                scphi = CxxUtils::sincos(phi);
                sctheta = CxxUtils::sincos(theta);                
                phipattern_x = r0 * scphi.sn;
                phipattern_y = r0 * scphi.cs;
            } else {
                const Amg::Vector3D posphipattern = phipattern->globalPosition();
                phipattern_x = posphipattern.x();
                phipattern_y = posphipattern.y();
                r0 = MuonHoughMathUtils::signedDistanceOfLineToOrigin2D(phipattern_x, phipattern_y, phi);
            }

            // is etapattern on phi pattern?
            // Loop over hits etapattern

            // int nhits_in_average_eta=0;
            // int nhits_inside_distance_cut_eta=0;

            if (dotprod <= 0.5 && !m_use_cosmics) continue;

            ATH_MSG_DEBUG(" Matched Eta/phi pattern ");

            // keep track of the number of eta/phi trigger and CSC hits per chamber
            std::map<Identifier, ChamberInfo> infoPerChamber;
            std::map<Muon::MuonStationIndex::StIndex, ChamberInfo> infoPerStation;
            // Loop over hits phi pattern
            double average_distance{0};
            int nhits_in_average{0}, nhits_inside_distance_cut{0};

            double phiPatMin{1e9}, phiPatMax{-1e9};

            for (unsigned int phihitid = 0; phihitid < phipattern->numberOfContainedPrds(); phihitid++) {
                const Trk::PrepRawData* prd = phipattern->prd(phihitid);
                const Amg::Vector3D& globalposhit = globalPos(prd);
                double radius_hit = globalposhit.perp();
                double dotprodradius = sctheta.apply(radius_hit, globalposhit.z());
                ATH_MSG_VERBOSE("combine hit: " << m_idHelperSvc->toString(prd->identify()) << " dotprod: " << dotprodradius);
                if (dotprodradius < 0 && !m_use_cosmics) continue;
               
                double residu_distance_mm{100.*Gaudi::Units::meter};
                if (m_use_cosmics) {
                    const double perp = scphi.apply(globalposhit.y(), globalposhit.x());
                    residu_distance_mm = MuonHoughMathUtils::signedDistanceToLine(globalposhit.z(), perp, rz0, theta);
                } else {
                    residu_distance_mm = MuonHoughMathUtils::signedDistanceCurvedToHit(pattern_z0, theta, invcurvature, globalposhit);
                }

                double distancetoline = std::abs(residu_distance_mm);

                ATH_MSG_VERBOSE(" distance RZ: " << residu_distance_mm);
                if (distancetoline >= m_maximum_rzdistance)  continue;
              
                ATH_MSG_VERBOSE(" accepted ");
                nhits_inside_distance_cut++;
                nhits_in_average++;
                average_distance += distancetoline;

                if (!useTightAssociation) { continue; }
                Identifier chId = m_idHelperSvc->chamberId(prd->identify());
                ChamberInfo& chInfo = infoPerChamber[chId];                
                ++chInfo.nphi;
                double hitphi = globalposhit.phi();
                chInfo.phiMin = std::min(hitphi,chInfo.phiMin);
                chInfo.phiMax = std::min(hitphi,chInfo.phiMax);

                Muon::MuonStationIndex::StIndex stIndex = m_idHelperSvc->stationIndex(prd->identify());
                ChamberInfo& stInfo = infoPerStation[stIndex];                
                ++stInfo.nphi;
                stInfo.phiMin = std::min(hitphi, stInfo.phiMin);
                stInfo.phiMax = std::max(hitphi, stInfo.phiMax);
                phiPatMin =std::min(hitphi,phiPatMin);
                phiPatMax =std::max(hitphi,phiPatMax);
                
            }      // size muonpattern

            if (nhits_in_average > 0) average_distance /= nhits_in_average;

            ATH_MSG_DEBUG(" Result for phi pattern: accepted hits " << nhits_inside_distance_cut << " average distance "
                                                                    << average_distance);

            bool etapattern_passed = false;
            for (unsigned int etahitid = 0; etahitid < etapattern->numberOfContainedPrds(); etahitid++) {
                const Trk::PrepRawData* prd = etapattern->prd(etahitid);
                const Amg::Vector3D& etaglobalposhit = globalPos(prd);
                const double etahitx = etaglobalposhit.x();
                const double etahity = etaglobalposhit.y();

                if (!m_use_cosmics) {
                    double etadotprod = scphi.apply(etahity, etahitx);  
                                                                        // same as in maketruetracksegment
                    if (etadotprod < 0) continue;                       // hit in wrong hemisphere
                }

                const double xdiff = phipattern_x - etahitx;
                const double ydiff = phipattern_y - etahity;
                const double etahitr = std::hypot(xdiff, ydiff);

                bool hit_passed = false;
                double etadistancetoline = std::abs(MuonHoughMathUtils::distanceToLine(etahitx, etahity, r0, phi));

                ATH_MSG_VERBOSE("combine: " << m_idHelperSvc->toString(prd->identify()) << " distance xy " << etadistancetoline);

                if (m_use_cosmics) {  // phi cone does not work for cosmics since hits
                                      // might be close to position of pattern

                    const double scale = etahitr / 7000.;
                    hit_passed = etadistancetoline < scale * m_maximum_xydistance; 
                } else if (2 * etadistancetoline < etahitr) {  // this corresponds with 30 degrees , normal formula:
                                                            // (etadistancetoline/etahitr) < sin( Pi/180 * degrees)
                    hit_passed = true;
                }

                if (!hit_passed) { continue; }
                
                etapattern_passed = true;  // only one hit required
                // break;
                ATH_MSG_VERBOSE(" accepted");

                if (!useTightAssociation) { continue ;}
                Identifier chId = m_idHelperSvc->chamberId(prd->identify());
                
                ChamberInfo& chInfo = infoPerChamber[chId];
                ++chInfo.neta;
                if (m_idHelperSvc->isMdt(prd->identify())) {
                    Muon::MuonStationIndex::StIndex stIndex = m_idHelperSvc->stationIndex(prd->identify());
                    ChamberInfo& stInfo = infoPerStation[stIndex];

                    const MuonGM::MdtReadoutElement* mdtDetEl =
                        dynamic_cast<const MuonGM::MdtReadoutElement*>(prd->detectorElement());
                    if (!mdtDetEl) continue;

                    const Identifier id = prd->identify();
                    const Trk::Surface& surf = mdtDetEl->surface(id);
                    int layer = m_idHelperSvc->mdtIdHelper().tubeLayer(id);
                    int tube = m_idHelperSvc->mdtIdHelper().tube(id);
                    double halfLength = 0.5 * mdtDetEl->getWireLength(layer, tube);
                    Amg::Vector2D lpLeft(0, -halfLength);
                    Amg::Vector3D gposLeft = surf.localToGlobal(lpLeft);
                    double phiLeft = gposLeft.phi();

                    Amg::Vector2D lpRight(0, halfLength);
                    const Amg::Vector3D gposRight = surf.localToGlobal(lpRight);
                    double phiRight = gposRight.phi();
                    double phiMin = std::min(phiRight, phiLeft);
                    double phiMax = std::max(phiLeft ,phiRight);

                    Amg::Vector3D tubePos = mdtDetEl->tubePos(id);
                    Amg::Vector3D ROPos = mdtDetEl->ROPos(id);
                    Amg::Vector3D HVPos = 2 * tubePos - ROPos;
                    double tubeL = (HVPos - ROPos).mag();
                    double phiRO = ROPos.phi();
                    double phiHV = HVPos.phi();
                    double phiMinPos = std::min(phiHV, phiRO);
                    double phiMaxPos = std::max(phiRO, phiHV);

                    if (std::abs(phiMin - phiMinPos) > 0.01 || std::abs(phiMax - phiMaxPos) > 0.01) {
                        ATH_MSG_DEBUG(" inconsistent Phi!!: from locToGlob (" << phiMin << "," << phiMax << "), from positions ("
                                                                                << phiMinPos << "," << phiMaxPos << ")");
                    }
                    double rotationFraction = 0.;
                    if (phiMin < 0 && phiMax > 0) {
                        if (phiMin < -0.75 * M_PI || phiMax > 0.75 * M_PI)
                            rotationFraction = 1.5;
                        else
                            rotationFraction = 0.5;
                    } else if (phiMax < 0) {
                        rotationFraction = 1.;
                    }
                    double phiMinR = rotatePhi(phiMin, rotationFraction);
                    double phiMaxR = rotatePhi(phiMax, rotationFraction);
                    phiMin = std::min(phiMinR, phiMaxR);
                    phiMax = std::max(phiMinR, phiMaxR);

                    phiMinR = rotatePhi(phiMinPos, rotationFraction);
                    phiMaxR = rotatePhi(phiMaxPos, rotationFraction);
                    phiMinPos = std::min(phiMinR, phiMaxR);
                    phiMaxPos = std::max(phiMinR, phiMaxR);

                    // enlarge range by 0.1 rad
                    phiMin = phiMin > 0 ? phiMin - 0.1 : phiMin + 0.1;
                    phiMax = phiMax > 0 ? phiMax + 0.1 : phiMax - 0.1;

                    double phiMinSec{1.e9},phiMaxSec{-1.e9};
                    if (stInfo.nphi > 0 && stInfo.phiMin < 1000) {
                        phiMinR = rotatePhi(stInfo.phiMin, rotationFraction);
                        phiMaxR = rotatePhi(stInfo.phiMax, rotationFraction);
                        phiMinSec = std::min(phiMinR, phiMaxR);
                        phiMaxSec = std::max(phiMinR, phiMaxR);

                        bool inside = true;

                        // easy case
                        if (phiMinSec > 0 && phiMaxSec > 0) {
                            if (phiMin > phiMaxSec || phiMax < phiMinSec) inside = false;
                        } else if (phiMinSec < 0 && phiMaxSec < 0) {
                            // easy case (2), always outside
                            inside = false;
                        } else {
                            // finaly check if phiMaxSec large than maxPhi
                            if (phiMax < phiMaxSec) inside = false;
                        }
                        // case with a
                        if (inside) {
                            ++stInfo.ninside;
                            ++chInfo.ninside;
                             ATH_MSG_VERBOSE(__FILE__<<":"<<__LINE__<<" Inside  "); 
                        } else {
                            ++stInfo.noutside;
                            ++chInfo.noutside;
                             ATH_MSG_VERBOSE(__FILE__<<":"<<__LINE__<<" Outside "); 
                        }
                    }

                    phiMinR = rotatePhi(phiPatMin, rotationFraction);
                    phiMaxR = rotatePhi(phiPatMax, rotationFraction);
                    double phiMinPat = std::min(phiMinR, phiMaxR);
                    double phiMaxPat = std::max(phiMinR, phiMaxR);

                    bool insidePat = true;
                    // easy case
                    if (phiMinPat > 0 && phiMaxPat > 0) {
                        if (phiMin > phiMaxPat || phiMax < phiMinPat) insidePat = false;
                    } else if (phiMinPat < 0 && phiMaxPat < 0) {
                        // easy case (2), always outside
                        insidePat = false;
                    } else {
                        // finaly check if phiMaxPat large than maxPhi
                        if (phiMax < phiMaxPat) insidePat = false;
                    }

                    // case with a
                    if (insidePat) {
                        ++stInfo.ninsidePat;
                        ++chInfo.ninsidePat;
                        ATH_MSG_VERBOSE(__FILE__<<":"<<__LINE__<<" InPat  "); 
                    } else {
                        ++stInfo.noutsidePat;
                        ++chInfo.noutsidePat;
                         ATH_MSG_VERBOSE(__FILE__<<":"<<__LINE__<<" OutPat ");
                    }
                    if (myDebug) {
                        ATH_MSG_DEBUG(" : Phi MDT ("
                                        << std::setprecision(3) << std::setw(4) << phiMin << "," << std::setw(4) << phiMax << ") "
                                        << " from pos (" << std::setprecision(3) << std::setw(4) << phiMinPos << "," << std::setw(4)
                                        << phiMaxPos << ") ");
                        if (stInfo.nphi > 0 && stInfo.phiMin < 1000) {
                            ATH_MSG_DEBUG(" phi range (" << std::setprecision(3) << std::setw(4) << stInfo.phiMin << ","
                                                            << std::setw(4) << stInfo.phiMax << ")  ");
                        }
                        ATH_MSG_DEBUG(" pat range (" << std::setprecision(3) << std::setw(4) << phiMinPat << "," << std::setw(4)
                                                        << phiMaxPat << ")  " << m_idHelperSvc->toString(prd->identify()));
                        ATH_MSG_DEBUG(" ATL " << mdtDetEl->getActiveTubeLength(layer, tube) << " WL "
                                               << mdtDetEl->getWireLength(layer, tube) << " POSL " << tubeL);
                    }
                }
            }  // eta pattern

            if (!etapattern_passed) continue;  // no etahit close to phipattern, try next phi pattern
            unsigned int netaPhiPairs = 0;
            if (useTightAssociation) {
                // now we have calculated the eta/phi hit content of the 'merged'
                // pattern                
                for ( auto& [chamberId, chamberInfo] : infoPerChamber) {
                    
                    ATH_MSG_VERBOSE("  " << std::setw(32) << m_idHelperSvc->toStringChamber(chamberId) << "  eta hits "
                                        << chamberInfo.neta << "  phi hits " << chamberInfo.nphi << "  ninside  "
                                        << chamberInfo.ninside << "  noutside  " << chamberInfo.noutside << "  ninside  "
                                        << chamberInfo.ninsidePat << "  noutside  " << chamberInfo.noutsidePat);
                
                    netaPhiPairs += (chamberInfo.neta  && chamberInfo.nphi);
                }
               
                ATH_MSG_DEBUG(" eta/phi pattern hit overlap " << netaPhiPairs);
                if (!etapattern_passed) { ATH_MSG_DEBUG("  failed eta hit match "); }
                if (nhits_inside_distance_cut < (phipattern->numberOfContainedPrds() * 0.25)) {
                    ATH_MSG_DEBUG("  failed phi hit match ");
                }
                if (netaPhiPairs == 0) { ATH_MSG_DEBUG("  Bad match, no overlap "); }
                                
            }
            ATH_MSG_VERBOSE(" Eta pattern compatible with phi pattern, eta/phi overlap " << netaPhiPairs << " ass phi hits "
                                                                                       << nhits_inside_distance_cut << " tot phi hits "
                                                                                       << phipattern->numberOfContainedPrds()
                                                                                       <<(useTightAssociation ? " using tight association ": "" ));
          
            // at least 25% matched, to be more robust than 1!
            if ((!useTightAssociation || netaPhiPairs > 0) && nhits_inside_distance_cut >= (phipattern->numberOfContainedPrds() * 0.25)) {
                ismatched = true;

                // for non-cosmics try every candidate
                // for cosmics keep track of best candidate, possible eff loss when phi
                // patterns are split
                if (m_use_cosmics) {
                    std::array<double,4> new_pars{r0, phi, pattern_z0, theta};
                    PrdPatternPair updatedpatterns = updatePatternsForCosmics(*phipattern, *etapattern, new_pars);
                    phipattern = std::move(updatedpatterns.first);
                    etapattern = std::move(updatedpatterns.second);
                    ATH_MSG_DEBUG(" Combination accepted with cosmic selection ");
                } else if (useTightAssociation) {
                    ATH_MSG_DEBUG(" Tight association, cleaning patterns");

                    // clean up hits using local phi info
                    std::unique_ptr<Muon::MuonPrdPattern> etaPat = std::make_unique<Muon::MuonPrdPattern>(etapattern->globalPosition(), etapattern->globalDirection());
                    std::unique_ptr<Muon::MuonPrdPattern> phiPat = std::make_unique<Muon::MuonPrdPattern>(phipattern->globalPosition(), phipattern->globalDirection());
                    for (unsigned int etahitid = 0; etahitid < etapattern->numberOfContainedPrds(); ++etahitid) {
                        const Trk::PrepRawData* prd = etapattern->prd(etahitid);
                        const Identifier id = prd->identify();
                        Identifier chId = m_idHelperSvc->chamberId(id);
                        std::map<Identifier, ChamberInfo>::iterator chPos = infoPerChamber.find(chId);
                        if (chPos == infoPerChamber.end()) continue;

                        if (m_idHelperSvc->isMdt(id)) {
                            if (chPos->second.ninside == 0 && chPos->second.noutside > 0) continue;
                            if (chPos->second.ninsidePat == 0 && chPos->second.noutsidePat > 0) continue;
                        } else {
                            if (chPos->second.nphi == 0) continue;
                        }
                        etaPat->addPrd(prd);
                    }
                    for (unsigned int phihitid = 0; phihitid < phipattern->numberOfContainedPrds(); ++phihitid) {
                        const Trk::PrepRawData* prd = phipattern->prd(phihitid);
                        const Identifier& id = prd->identify();
                        Identifier chId = m_idHelperSvc->chamberId(id);
                        std::map<Identifier, ChamberInfo>::iterator chPos = infoPerChamber.find(chId);
                        if (chPos == infoPerChamber.end()) continue;

                        if (chPos->second.neta == 0) continue;
                        phiPat->addPrd(prd);
                    }                   
                    phipattern = std::move(phiPat);
                    etapattern = std::move(etaPat);
                }

                if (!m_bestphimatch) {
                    addCandidate(etapattern, phipattern, candidates, false, phiEtaHitAssMap);
                    ATH_MSG_DEBUG("Candidate FOUND eta " << etalevel << " phi " << philevel << " dotprod: " << dotprod);
                } else {
                    if (average_distance < min_average_distance) {
                        ATH_MSG_DEBUG(" Selected as best candidate ");
                        min_average_distance = average_distance;
                        max_phipattern = phipattern;
                        max_philevel = philevel;
                    }
                    ATH_MSG_DEBUG(" theta pattern " << etapattern->globalDirection().theta() << " phi "
                                                    << phipattern->globalDirection().phi() << "average distance " << average_distance
                                                    << " number of hits " << nhits_inside_distance_cut << " etalevel: " << etalevel);
                }

                // add recovery for the case we have an inefficiency in the eta hits
            } else if (useTightAssociation && netaPhiPairs == 0 &&
                       nhits_inside_distance_cut >= (phipattern->numberOfContainedPrds() * 0.25)) {
                ATH_MSG_DEBUG(" Combination rejected by phi/eta overlap: average distance " << average_distance);

                if (average_distance < min_average_distance) {
                    ATH_MSG_DEBUG("  but selected as best candidate ");
                    min_average_distance = average_distance;
                    max_phipattern = phipattern;
                    max_philevel = philevel;
                }
            }  // nhits>=25%
        }      // size phi level

        // for cosmics only match best phi pattern
        if (m_bestphimatch && ismatched) {            
            addCandidate(etapattern, max_phipattern, candidates, true, phiEtaHitAssMap);
            ATH_MSG_DEBUG("Candidate FOUND eta " << etalevel << " phi " << max_philevel);            
        }
        // make associated phi pattern for every etapattern and as candidate for
        // robustness not needed for cosmics when matched, since already done for
        // first match:

        if (!(m_use_cosmics && m_splitpatterns && ismatched)) {
            std::unique_ptr<Muon::MuonPrdPattern> assphipattern = makeAssPhiPattern(*etapattern, phiEtaHitAssMap, true);
            ATH_MSG_DEBUG("No match found, trying to create associated phi pattern ");
            if (assphipattern) {
                // make sure ass phi pattern is not a complete subset of one of the
                // other phi patterns:

                bool subsetcheck = false;

                std::reverse_iterator<std::vector<CandidatePatPair>::iterator> rit =
                    candidates.rbegin();
                for (; rit != candidates.rend(); ++rit) {
                    if ((*rit).first != etapattern) { break; }
                    // 	    for (;phi_it!=range.second;++phi_it) {
                    if (subset(assphipattern.get(), (*rit).second.get())) {
                        subsetcheck = true;
                        break;
                    }
                }
                if (subsetcheck) {
                    
                } else {
                    // these associated phi patterns should be deleted at end of routine:
                    

                    ATH_MSG_DEBUG("Candidate FOUND eta " << etalevel << " and associated phipattern ");

                    ismatched = true;

                    // print associated pattern:
                    if (msgLvl(MSG::VERBOSE)) printPattern(assphipattern.get());

                    if (m_use_cosmics) {
                        std::array<double,4> new_pars = updateParametersForCosmics(*assphipattern, *etapattern);
                        PrdPatternPair updatedpatterns = updatePatternsForCosmics(*assphipattern, *etapattern, new_pars);
                        assphipattern = std::move(updatedpatterns.first);
                        etapattern = std::move(updatedpatterns.second);
                    }
                    ATH_MSG_DEBUG(" adding eta pattern with recalculated associated phi pattern ");

                    addCandidate(etapattern, std::move(assphipattern), candidates, false, phiEtaHitAssMap);
                }
            }
        }
        if (!ismatched && max_philevel > -1) {
            addCandidate(etapattern, max_phipattern, candidates, true, phiEtaHitAssMap);
            ismatched = true;
            ATH_MSG_DEBUG("No good candidate found, adding best phi pattern " << etalevel << " phi " << max_philevel);
        }
        if (!ismatched) {
            if (msgLvl(MSG::DEBUG)) {
                ATH_MSG_DEBUG("NO COMBINED Candidate FOUND eta " << etalevel << " phi " << phibest);
                if (!m_use_cosmics) ATH_MSG_DEBUG("dotprodbest: " << dotprodbest);
                ATH_MSG_DEBUG("writing out eta pattern (no cleanup)");
            }
            candidates.emplace_back(etapattern, nullptr);
        } else {
            if (msgLvl(MSG::DEBUG)) ATH_MSG_DEBUG("Candidate was associated to a phi pattern ");
        }
    }  // size rtheta level
    return makeCombinedPatterns(candidates);

}  // combineEtaPhiPatterns

std::unique_ptr<MuonPrdPatternCollection> MuonCombinePatternTool::makeCombinedPatterns(
    std::vector<CandidatePatPair>& candidates) const {
    ATH_MSG_DEBUG("Number of Candidates: " << candidates.size());

    //  if (m_use_cosmics == true) {
    cleanCandidates(candidates);
    ATH_MSG_DEBUG("Number of Candidates after cleaning: " << candidates.size());
    //}

    std::unique_ptr<MuonPrdPatternCollection> combinedPatternCollection = std::make_unique<MuonPrdPatternCollection>();
    int number_comb_patterns = 0;    
    for ( const auto& [etapattern, phipattern] : candidates) {
        ATH_MSG_DEBUG("Next Candidate");
        if (phipattern) {
            std::unique_ptr<Muon::MuonPrdPattern> combinedpattern = makeCombinedPattern(*phipattern, *etapattern);
            if (combinedpattern) {
                if (msgLvl(MSG::VERBOSE)) { printPattern(combinedpattern.get()); }
                combinedPatternCollection->push_back(combinedpattern.release());
                number_comb_patterns++;
            } else
                ATH_MSG_WARNING("combined pattern lost");
        } else {  // no combined match found &&  no associated phi hits found
            if (m_splitpatterns && m_use_cosmics) {
                ATH_MSG_VERBOSE("No combined pattern, eta pattern split based on phi direction of eta pattern ");

                std::vector<PrdPatternPair> splitetapatterns = splitPatternsCylinder(nullptr, etapattern.get());
                if (splitetapatterns.empty()) {
                    combinedPatternCollection->push_back(etapattern->clone());
                } else {
                    for (unsigned int i = 0; i < splitetapatterns.size(); i++) {
                        if (splitetapatterns[i].second->numberOfContainedPrds() != 0) {
                            combinedPatternCollection->push_back(splitetapatterns[i].second.release());
                        } 
                    }
                }
            } else {  // don't split pattern
                ATH_MSG_DEBUG("No combined pattern, eta pattern copied ");
                combinedPatternCollection->push_back(etapattern->clone());
            }
        }
    }

    ATH_MSG_DEBUG("Number of combined patterns: " << number_comb_patterns << " Number of unmatched etapatterns: "
                                                  << combinedPatternCollection->size() - number_comb_patterns);

    return combinedPatternCollection;
}

std::unique_ptr<Muon::MuonPrdPattern> MuonCombinePatternTool::makeCombinedPattern(const Muon::MuonPrdPattern& phipattern,
                                                                  const Muon::MuonPrdPattern& etapattern) const {
    ATH_MSG_DEBUG("make combined pattern");

    double phi = phipattern.globalDirection().phi();
    double theta = etapattern.globalDirection().theta();
    CxxUtils::sincos scphi(phi);
    CxxUtils::sincos sctheta(theta);

    double phieta = etapattern.globalDirection().phi();
    double eta_x = etapattern.globalPosition().x();
    double eta_y = etapattern.globalPosition().y();
    double eta_r0 = MuonHoughMathUtils::signedDistanceOfLineToOrigin2D(eta_x, eta_y, phieta);
    double charge = 1.;
    if (!m_use_cosmics && eta_r0 < 0) charge = -1.;
    double curvature = etapattern.globalDirection().mag();

    // Store charge in sign of r0 (like in Muon eta pattern)

    const double x0 = charge * (phipattern.globalPosition().x());
    const double y0 = charge * (phipattern.globalPosition().y());
    const double z0_phi = m_use_cosmics ?(calculateRz0(etapattern, phi, theta) / sctheta.sn) : 0.;  // for non-cosmics

    const Amg::Vector3D dir{curvature * scphi.cs * sctheta.sn, curvature * scphi.sn * sctheta.sn, curvature * sctheta.cs};
    const Amg::Vector3D pos{x0, y0, z0_phi};

    Muon::MuonPrdPattern::PrdVector comb_prds{};
    comb_prds.insert(comb_prds.end(),etapattern.prepRawDataVec().begin(),etapattern.prepRawDataVec().end());
    comb_prds.insert(comb_prds.end(),phipattern.prepRawDataVec().begin(),phipattern.prepRawDataVec().end());    
    std::unique_ptr<Muon::MuonPrdPattern> combinedpattern = std::make_unique<Muon::MuonPrdPattern>(pos, dir, std::move(comb_prds));

    ATH_MSG_DEBUG("Combined pattern with charge " << charge << " curvature " << curvature);
    ATH_MSG_DEBUG("direction combined pattern: " << scphi.cs * sctheta.sn << " " << scphi.sn * sctheta.sn << " " << sctheta.cs);
    ATH_MSG_DEBUG("position combined pattern: " << x0 << " " << y0 << " " << z0_phi);
    ATH_MSG_DEBUG("etapatternsize: " << etapattern.numberOfContainedPrds());
    ATH_MSG_DEBUG("phipatternsize: " << phipattern.numberOfContainedPrds());
    ATH_MSG_DEBUG("Combined Track size: " << combinedpattern->numberOfContainedPrds());

    if (m_use_cosmics) {
        if (msgLvl(MSG::VERBOSE)) ATH_MSG_VERBOSE("No Cleaning for Cosmics!");

        if (msgLvl(MSG::VERBOSE)) { printPattern(combinedpattern.get()); }
        return combinedpattern;
    }

    ATH_MSG_DEBUG("Start Cleaning and Recalculating of Combined Pattern");

    bool change = true;

    while (change) {
        int size_before_cleanup = combinedpattern->numberOfContainedPrds();
        ATH_MSG_DEBUG("size before cleanup: " << size_before_cleanup);

        std::unique_ptr<Muon::MuonPrdPattern> cleaneduppattern = cleanupCombinedPattern(*combinedpattern);

        int size_after_cleanup = cleaneduppattern->numberOfContainedPrds();
        ATH_MSG_DEBUG("size after cleanup: " << size_after_cleanup);

        if (size_before_cleanup == size_after_cleanup || size_after_cleanup == 0) {
            if (msgLvl(MSG::VERBOSE)) { printPattern(cleaneduppattern.get()); }
            return cleaneduppattern;
        } else if (size_after_cleanup < size_before_cleanup) {
            combinedpattern = std::move(cleaneduppattern);
        } else {
            change = false;
            ATH_MSG_FATAL("Cosmic Muon through computer bit? ");
        }
    }  // while
    return nullptr;
}  // makeCombinedPattern

std::vector<PrdPatternPair> MuonCombinePatternTool::splitPatterns2D(
    const Muon::MuonPrdPattern* phipattern, const Muon::MuonPrdPattern* etapattern) const {
    std::vector<PrdPatternPair> splitPatterns;
    splitPatterns.reserve(2);

    double phi = phipattern->globalDirection().phi();
    double theta = etapattern->globalDirection().theta();

    CxxUtils::sincos scphi(phi);
    CxxUtils::sincos sctheta(theta);

    const Amg::Vector3D dir1{scphi.cs * sctheta.sn, scphi.sn * sctheta.sn, sctheta.cs};
    
    std::unique_ptr<Muon::MuonPrdPattern> phipattern1 = std::make_unique<Muon::MuonPrdPattern>(phipattern->globalPosition(), dir1);  // "lower" pattern (y<0)
    std::unique_ptr<Muon::MuonPrdPattern> etapattern1 = std::make_unique<Muon::MuonPrdPattern>(etapattern->globalPosition(), dir1);  // "lower" pattern (y<0)

    Amg::Vector3D dir2{dir1};

    if (m_flipdirectionforcosmics) {
        const double newphi = phi + M_PI;
        const double newtheta = M_PI - etapattern->globalDirection().theta();
        CxxUtils::sincos scnewphi(newphi);
        CxxUtils::sincos scnewtheta(newtheta);

        // flip phi and theta for second pattern:
        dir2 = Amg::Vector3D(scnewphi.cs * scnewtheta.sn, scnewphi.sn * scnewtheta.sn, scnewtheta.cs);
    }

    std::unique_ptr<Muon::MuonPrdPattern> phipattern2 = std::make_unique<Muon::MuonPrdPattern>(phipattern->globalPosition(), dir2);  // "upper" pattern (y>0)
    std::unique_ptr<Muon::MuonPrdPattern> etapattern2 = std::make_unique<Muon::MuonPrdPattern>(etapattern->globalPosition(), dir2);  // "upper" pattern (y>0)

    ATH_MSG_DEBUG(" split pattern1 theta: " << phipattern1->globalDirection().theta() << " phi: " << phipattern1->globalDirection().phi());
    ATH_MSG_DEBUG(" split pattern2 theta: " << phipattern2->globalDirection().theta() << " phi: " << phipattern2->globalDirection().phi());

    for (unsigned int hitid = 0; hitid < phipattern->numberOfContainedPrds(); hitid++) {
        const Trk::PrepRawData* prd = phipattern->prd(hitid);
        const Amg::Vector3D& globalposhit = globalPos(prd);
        const double dotprod = scphi.apply(globalposhit.y(), globalposhit.x());
        if (dotprod >= 0) {
            phipattern1->addPrd(prd);
        } else {
            phipattern2->addPrd(prd);
        }
    }
    for (unsigned int hitid = 0; hitid < etapattern->numberOfContainedPrds(); hitid++) {
        const Trk::PrepRawData* prd = etapattern->prd(hitid);
        const Amg::Vector3D& globalposhit = globalPos(prd);
        const double dotprod = scphi.apply(globalposhit.y(), globalposhit.x());
        if (dotprod >= 0) {
            etapattern1->addPrd(prd);
        } else {
            etapattern2->addPrd(prd);
        }
    }
    splitPatterns.emplace_back(std::move(phipattern1), std::move(etapattern1));
    splitPatterns.emplace_back(std::move(etapattern1), std::move(etapattern2));
    return splitPatterns;
}

std::vector<PrdPatternPair> MuonCombinePatternTool::splitPatterns3D(
    const Muon::MuonPrdPattern* phipattern, const Muon::MuonPrdPattern* etapattern) const {
    // phi pattern may be 0

    std::vector<PrdPatternPair> splitPatterns;
    splitPatterns.reserve(2);

    // phi and etapattern should have identical directions and positions (except
    // for z_0), but new built anyway to be robust

    Amg::Vector3D globalpos{Amg::Vector3D::Zero()};
    if (phipattern) {
        globalpos = phipattern->globalPosition();
    } else {
        globalpos = etapattern->globalPosition();
    }

    double phi;
    if (phipattern) {
        phi = phipattern->globalDirection().phi();
    } else {
        phi = etapattern->globalDirection().phi();
    }

    const double theta = etapattern->globalDirection().theta();

    CxxUtils::sincos scphi(phi);
    CxxUtils::sincos sctheta(theta);

    Amg::Vector3D dir1{scphi.cs * sctheta.sn, scphi.sn * sctheta.sn, sctheta.cs};
   
     Amg::Vector3D dir2 = dir1;
    if (m_flipdirectionforcosmics) {
        const double newphi = phi + M_PI;
        const double newtheta = M_PI - theta;

        CxxUtils::sincos scnewphi(newphi);
        CxxUtils::sincos scnewtheta(newtheta);

        // flip phi and theta for second pattern:
        dir2 = Amg::Vector3D(scnewphi.cs * scnewtheta.sn, scnewphi.sn * scnewtheta.sn, scnewtheta.cs);
    }

    std::unique_ptr<Muon::MuonPrdPattern> etapattern1 = std::make_unique<Muon::MuonPrdPattern>(globalpos, dir1);  //
    std::unique_ptr<Muon::MuonPrdPattern> etapattern2 = std::make_unique<Muon::MuonPrdPattern>(globalpos, dir2);  // "upper" pattern (y>0)
    std::unique_ptr<Muon::MuonPrdPattern> phipattern1, phipattern2{};

    if (phipattern) {
        phipattern1 = std::make_unique<Muon::MuonPrdPattern>(globalpos, dir1);  // "lower" pattern (y<0)
        phipattern2 = std::make_unique<Muon::MuonPrdPattern>(globalpos, dir2);  // "upper" pattern (y>0)
    }

   

    if (msgLvl(MSG::DEBUG)) {
        ATH_MSG_DEBUG(" split pattern theta: " << theta << " phi: " << phi);
        ATH_MSG_DEBUG(" split pattern1 theta: " << etapattern1->globalDirection().theta()
                                                << " phi: " << etapattern1->globalDirection().phi());
        ATH_MSG_DEBUG(" split pattern2 theta: " << etapattern2->globalDirection().theta()
                                                << " phi: " << etapattern2->globalDirection().phi());
        Amg::Vector3D splitpoint = MuonHoughMathUtils::shortestPointOfLineToOrigin(globalpos, phi, theta);
        ATH_MSG_DEBUG(" splitpoint, x: " << splitpoint[0] << " y: " << splitpoint[1] << " z: " << splitpoint[2]);
    }

    double d_x = scphi.cs * sctheta.sn;
    double d_y = scphi.sn * sctheta.sn;

    if (phipattern) {
        for (unsigned int hitid = 0; hitid < phipattern->numberOfContainedPrds(); hitid++) {
            const Trk::PrepRawData* prd = phipattern->prd(hitid);
            const Amg::Vector3D& globalposhit = globalPos(prd);
            const double hitx = globalposhit.x();
            const double hity = globalposhit.y();
            const double hitz = globalposhit.z();
            const double dotprod = hitx * d_x + hity * d_y + hitz * sctheta.cs;
            if (dotprod >= 0) {
                phipattern1->addPrd(prd);
            } else {
                phipattern2->addPrd(prd);
            }
            ATH_MSG_VERBOSE(" dotprod: " << dotprod);
        }
    }

    for (unsigned int hitid = 0; hitid < etapattern->numberOfContainedPrds(); hitid++) {
        const Trk::PrepRawData* prd = etapattern->prd(hitid);
        const Amg::Vector3D& globalposhit = globalPos(prd);
        const double hitx = globalposhit.x();
        const double hity = globalposhit.y();
        const double hitz = globalposhit.z();
        const double dotprod = hitx * d_x + hity * d_y + hitz * sctheta.cs;
        if (dotprod >= 0) {
            etapattern1->addPrd(prd);
        } else {
            etapattern2->addPrd(prd);
        }
        ATH_MSG_VERBOSE(" dotprod: " << dotprod);
    }

    if (msgLvl(MSG::DEBUG)) {
        if (phipattern) {
            ATH_MSG_DEBUG("Final size, phi: " << phipattern1->numberOfContainedPrds() << " " << phipattern2->numberOfContainedPrds());
        }
        ATH_MSG_DEBUG("Final size, eta: " << etapattern1->numberOfContainedPrds() << " " << etapattern2->numberOfContainedPrds());
    }
    splitPatterns.emplace_back(std::move(phipattern1), std::move(etapattern1));
    splitPatterns.emplace_back(std::move(phipattern2), std::move(etapattern2));

    return splitPatterns;
}

std::vector<PrdPatternPair> MuonCombinePatternTool::splitPatternsCylinder(
    const Muon::MuonPrdPattern* phipattern, const Muon::MuonPrdPattern* etapattern) const {
    // phi pattern may be 0
    Amg::Vector3D patternpos{Amg::Vector3D::Zero()};
    double phi{0.};

    if (phipattern) {
        patternpos = phipattern->globalPosition();
        phi = phipattern->globalDirection().phi();
    } else {
        patternpos = etapattern->globalPosition();
        phi = etapattern->globalDirection().phi();
    }

    const double theta = etapattern->globalDirection().theta();

    // decide if track is split:

    // calculate intersection with pattern and calorimeter cylinder (r0=4000,
    // c0=6000) if there is one, then track will be split

    bool intersect = MuonHoughMathUtils::lineThroughCylinder(patternpos, phi, theta, MuonHough::radius_cylinder, MuonHough::z_cylinder);

    if (!intersect) {  // no split
        ATH_MSG_DEBUG("Pattern not through calorimeter -> do not split ");
        return {};  
    }

    return splitPatterns3D(phipattern, etapattern);
}

std::unique_ptr<Muon::MuonPrdPattern> MuonCombinePatternTool::cleanupCombinedPattern(const Muon::MuonPrdPattern& combinedpattern) const {
    const double phipattern = combinedpattern.globalDirection().phi();
    const double thetapattern = combinedpattern.globalDirection().theta();

    CxxUtils::sincos scthetapattern(thetapattern);

    const Amg::Vector3D& patternpos = combinedpattern.globalPosition();
    const double posx = patternpos.x();
    const double posy = patternpos.y();
    const double posz = patternpos.z();

    double invcurvature = 0.;
    double r0 = MuonHoughMathUtils::signedDistanceOfLineToOrigin2D(posx, posy, phipattern);
    double charge = 1.;
    if (r0 < 0) charge = -1.;
    double curvature = combinedpattern.globalDirection().mag();
    if (curvature > 2) invcurvature = charge / curvature;

    ATH_MSG_DEBUG("cleaned up pattern: phi " << phipattern << " theta: " << thetapattern << " position: " << posx << " " << posy << " "
                                             << posz);
    ATH_MSG_DEBUG("Cleanup pattern charge " << charge << " curvature " << curvature);

    std::unique_ptr<Muon::MuonPrdPattern> combinedpattern_cleaned =
        std::make_unique<Muon::MuonPrdPattern>(combinedpattern.globalPosition(), combinedpattern.globalDirection());

    for (unsigned int hitid = 0; hitid < combinedpattern.numberOfContainedPrds(); hitid++) {
        const Trk::PrepRawData* prd = combinedpattern.prd(hitid);
        const Amg::Vector3D& globalposhit = globalPos(prd);
       
        double r0 = MuonHoughMathUtils::signedDistanceOfLineToOrigin2D(posx, posy, phipattern);
        double distance_xy = MuonHoughMathUtils::distanceToLine(globalposhit.x(), globalposhit.y(), r0, phipattern);

        double radius_pattern = globalposhit.perp();
        double z0 = posz - radius_pattern * scthetapattern.cs / scthetapattern.sn;

        double distance_rz = MuonHoughMathUtils::signedDistanceCurvedToHit(z0, thetapattern, invcurvature, globalposhit);

        const double scale = std::max(1., globalposhit.mag() / 7000.);
        ATH_MSG_VERBOSE("hit: " << globalposhit);
        ATH_MSG_VERBOSE("CLEAN distancetopattern: "
                        << " dist xy " << distance_xy << " dist rz " << distance_rz << " scale: " << scale);
        if (std::abs(distance_xy) < scale * m_maximum_xydistance && std::abs(distance_rz) < m_maximum_rzdistance) {
            combinedpattern_cleaned->addPrd(prd);
        } else {           
            ATH_MSG_DEBUG("Hit discarded: " << hitid << " dis xy " << distance_xy << " dis rz " << distance_rz);
            ATH_MSG_DEBUG("Hit info: "<<m_idHelperSvc->toString(prd->identify()));
        }
    }

    ATH_MSG_DEBUG("size of cleaned pattern: " << combinedpattern_cleaned->numberOfContainedPrds());

    if (combinedpattern_cleaned->numberOfContainedPrds() == 0 && combinedpattern.numberOfContainedPrds() != 0) {
        ATH_MSG_DEBUG(
            "cleaned up pattern is empty (should happen only when initially no phi "
            "pattern found and phi hits are added by ascociation map)");
    }

    return combinedpattern_cleaned;
}

std::unique_ptr<Muon::MuonPrdPattern> MuonCombinePatternTool::makeAssPhiPattern(
    const Muon::MuonPrdPattern& muonpattern,
    /** phi eta association map, eta prds are key*/
    const EtaPhiHitAssocMap& phiEtaHitAssMap,
    bool check_already_on_pattern) const {
    // bool hits_added = false;
    const unsigned int size = muonpattern.numberOfContainedPrds();

    PrepDataSet hits;
    if (check_already_on_pattern) {
        for (unsigned int i = 0; i < size; i++) { hits.insert(muonpattern.prd(i)); }
    }
    std::vector<const Trk::PrepRawData*> phihits;
    for (unsigned int i = 0; i < size; i++) {
        const Trk::PrepRawData* prd = muonpattern.prd(i);
        // check on type of prd?
        const Muon::MuonCluster* muoncluster = dynamic_cast<const Muon::MuonCluster*>(prd);
        if (!muoncluster) continue;
        EtaPhiHitAssocMap::const_iterator itr = phiEtaHitAssMap.find(prd);
        /// No phi hits were recorded in that chamber
        if (itr == phiEtaHitAssMap.end()) {
            continue;
        }
        std::copy_if(itr->second.begin(), itr->second.end(), std::back_inserter(phihits),
                [&check_already_on_pattern, &hits](const Trk::PrepRawData* phiHit){
            return !check_already_on_pattern || hits.insert(phiHit).second;
        });
    }
    
    
    if (phihits.empty()) { return nullptr; }
    
    std::unique_ptr<Muon::MuonPrdPattern> phipattern;

    double phi = 0., sin_phi = 0., cos_phi = 0.;
    if (m_use_cosmics) {
        phi = muonpattern.globalDirection().phi();
    } else {
        for (const Trk::PrepRawData* phiHit : phihits) {
            const Amg::Vector3D& globalposhit = globalPos(phiHit);
            CxxUtils::sincos scphihit(globalposhit.phi());
            sin_phi += scphihit.sn;
            cos_phi += scphihit.cs;
        }
        phi = std::atan2(sin_phi, cos_phi);
    }

    const double curvature = muonpattern.globalDirection().mag();
    const double theta = muonpattern.globalDirection().theta();
    CxxUtils::sincos scphi(phi);
    CxxUtils::sincos sctheta(theta);

    if (m_use_cosmics) {
        phipattern = std::make_unique<Muon::MuonPrdPattern>(muonpattern.globalPosition(), muonpattern.globalDirection(), phihits);
    } else {
        static const Amg::Vector3D globalpos{0.001, 0.001, 0.};
        const Amg::Vector3D& globaldir{curvature * scphi.cs * sctheta.sn, 
                                       curvature * scphi.sn * sctheta.sn, curvature * sctheta.cs};
        phipattern = std::make_unique<Muon::MuonPrdPattern>(globalpos, globaldir, phihits);
    }

  
    // perform cleaning on newly created phipattern:
    std::unique_ptr<Muon::MuonPrdPattern> phipatternclean = cleanPhiPattern(std::move(phipattern));
   
    if (phipatternclean->numberOfContainedPrds() <= 0) {
       phipatternclean.reset();
    }
    return phipatternclean;
}

std::array<double,4> MuonCombinePatternTool::updateParametersForCosmics(const Muon::MuonPrdPattern& phipattern,
                                                           const Muon::MuonPrdPattern& etapattern) const {
    // method returns r0, phi, rz0, theta

    const unsigned int etasize = etapattern.numberOfContainedPrds();
    const unsigned int phisize = phipattern.numberOfContainedPrds();

    const Amg::Vector3D& etaglobalpos = etapattern.globalPosition();
    const Amg::Vector3D& etaglobaldir = etapattern.globalDirection();

    const Amg::Vector3D& phiglobalpos = phipattern.globalPosition();
    const Amg::Vector3D& phiglobaldir = phipattern.globalDirection();

    std::array<double,4> old_pars{0};

    old_pars[0] = MuonHoughMathUtils::signedDistanceOfLineToOrigin2D(phiglobalpos.x(), phiglobalpos.y(), phiglobaldir.phi());
    old_pars[1] = phiglobaldir.phi();

    const double theta_orig = etaglobaldir.theta();
    old_pars[2] = etaglobalpos.z() * std::sin(theta_orig);
    old_pars[3] = theta_orig;

    if (phisize + etasize <= 1) return old_pars;

    // first calculate new phi, r0 (initial value , -Pi/2):
    std::pair<double, double> rphi_start = calculateR0Phi(phipattern, etapattern);
    // for stabilising (can be cpu-optimised greatly):
    std::pair<double, double> rphi = calculateR0Phi(phipattern, etapattern, rphi_start.first);

    if (msgLvl(MSG::DEBUG) && std::abs(std::sin(rphi.first - rphi_start.first)) > 0.15 &&
        std::abs(std::sin(etaglobaldir.phi() - phiglobaldir.phi())) < 0.15) {
        ATH_MSG_DEBUG("unexpected phi change!");
        ATH_MSG_DEBUG("phi first: " << rphi_start.first << " phi second: " << rphi.first);
        ATH_MSG_DEBUG("phi etapattern: " << etaglobaldir.phi() << " phi phipattern: " << phiglobaldir.phi());
    }

    const double phi = rphi.first;
    const double r0 = rphi.second;

    CxxUtils::sincos scphi(phi);

    // calculate new theta and rz0: (not using phi hits this time, as eta pattern
    // is larger in general

    double av_radii{0.}, av_z{0.};

    for (unsigned int i = 0; i < etasize; ++i) {
        const Trk::PrepRawData* prd = etapattern.prd(i);
        const Amg::Vector3D globalposhit = globalPos(prd);
        av_radii += scphi.apply(globalposhit.y(), globalposhit.x());  
        av_z += globalposhit.z();
    }

    if (etasize > 0) {
        av_radii /= etasize;
        av_z /= etasize;
    }
    double sumr = 0.;
    double sumz = 0.;
    for (unsigned int i = 0; i < etasize; i++) {
        const Trk::PrepRawData* prd = etapattern.prd(i);
        const Amg::Vector3D& globalposhit = globalPos(prd);
        
        double radius = scphi.apply(globalposhit.y(), globalposhit.x());  // hitx*scphi.cs + hity*scphi.sn;
        double r_offset = radius - av_radii;
        double z_offset = globalposhit.z() - av_z;
        double weight = r_offset * r_offset + z_offset * z_offset;
        int sign = 1;
        if (r_offset * std::cos(theta_orig) + z_offset * std::sin(theta_orig) < 0) { sign = -1; }
        sumr += weight * sign * r_offset;
        sumz += weight * sign * z_offset;
    }

    //  const double sum_tan = sum_tanweight/sum_weight;

    ATH_MSG_VERBOSE("av_z : " << av_z << " av_radii: " << av_radii << " sumr: " << sumr << " sumz: " << sumz);
    if (std::abs(sumr) < 0.000001 || std::abs(sumz) < 0.000001) return old_pars;

    double theta = std::atan2(sumr, sumz);

    if (theta < 0) theta += M_PI;

    double rz0 = calculateRz0(etapattern, phi, theta);

    // ATH_MSG_DEBUG("old method rz0: " << sctheta.apply(av_z,-av_radii) );
    // const double rz0 = sctheta.apply(av_z,-av_radii); // (av_z * sctheta.sn) -
    // av_radii * sctheta.cs;

    std::array<double,4> new_pars {r0,phi,rz0,theta};

    ATH_MSG_VERBOSE("updated parameters: r0: " << new_pars[0] << " phi: " << new_pars[1] << " rz0: " << new_pars[2]
                                             << " theta: " << new_pars[3]);
    ATH_MSG_VERBOSE("old parameters: r0: " << old_pars[0] << " phi: " << old_pars[1] << " rz0: " << old_pars[2] << " theta: " << old_pars[3]);

    if (msgLvl(MSG::VERBOSE)) {
        ATH_MSG_VERBOSE("phisize: " << phisize << " etasize: " << etasize);
        for (unsigned int i = 0; i < phisize; i++) {
            const Trk::PrepRawData* prd = phipattern.prd(i);
            const Amg::Vector3D& globalposhit = globalPos(prd);
            double distance = MuonHoughMathUtils::signedDistanceToLine(globalposhit.x(), globalposhit.y(), r0, phi);
            ATH_MSG_VERBOSE("distance to updated parameters in xy: " << distance);
            distance = MuonHoughMathUtils::signedDistanceToLine(globalposhit.x(), globalposhit.y(), old_pars[0], old_pars[1]);
            ATH_MSG_VERBOSE("old distance phi hit: " << distance);
        }
        for (unsigned int i = 0; i < etasize; i++) {
            const Trk::PrepRawData* prd = etapattern.prd(i);
            const Amg::Vector3D& globalposhit = globalPos(prd);
            double distance = MuonHoughMathUtils::signedDistanceToLine(globalposhit.x(), globalposhit.y(), r0, phi);
            ATH_MSG_VERBOSE("distance to updated parameters in xy: " << distance);
            distance = MuonHoughMathUtils::signedDistanceToLine(globalposhit.x(), globalposhit.y(), old_pars[0], old_pars[1]);
            ATH_MSG_VERBOSE("old distance eta hit: " << distance);
        }
        for (unsigned int i = 0; i < etasize; ++i) {
            const Trk::PrepRawData* prd = etapattern.prd(i);
            const Amg::Vector3D& globalposhit = globalPos(prd);
            double perp = scphi.apply(globalposhit.y(),
                                      globalposhit.x());  // globalposhit.x()*scphi.cs + globalposhit.y()*scphi.sn;
            double distance = MuonHoughMathUtils::signedDistanceToLine(globalposhit[Amg::z], perp, rz0, theta);
            ATH_MSG_VERBOSE("distance to updated parameters in Rz: " << distance);
            distance = MuonHoughMathUtils::signedDistanceToLine(globalposhit[Amg::z], perp, old_pars[2], old_pars[3]);
            ATH_MSG_VERBOSE("old distance: " << distance);
        }
    }
    return new_pars;
}

std::pair<double, double> MuonCombinePatternTool::calculateR0Phi(const Muon::MuonPrdPattern& phipattern,
                                                                 const Muon::MuonPrdPattern& etapattern, double phi_est) const {
    // use eta pattern as well, since phi patterns consist sometimes of only 1
    // station etahit error 200mrad (2Pi/16*2), phi hit 20mrad (should be hough
    // binsize (18 mrad), but prefer ~ factor 10 for stabilility)

    // test if lever_arm > 2 m before updating , if not old values are used

    ATH_MSG_VERBOSE("calculateR0Phi");

    CxxUtils::sincos scphi_est(phi_est);

    const unsigned int etasize = etapattern.numberOfContainedPrds();
    const unsigned int phisize = phipattern.numberOfContainedPrds();

    const Amg::Vector3D& etaglobaldir = etapattern.globalDirection();
    const double phi_etapattern = etaglobaldir.phi();
    CxxUtils::sincos scphi_eta(phi_etapattern);

    const Amg::Vector3D& phiglobaldir = phipattern.globalDirection();
    const Amg::Vector3D& phiglobalpos = phipattern.globalPosition();
    const double phi_phipattern = phiglobaldir.phi();
    CxxUtils::sincos scphi_phi(phi_phipattern);

    const double phi_error_inv = 1. / 20.;
    const double phi_error_inv2 = phi_error_inv * phi_error_inv;
    const double eta_error_inv = 1. / 400.;
    const double eta_error_inv2 = eta_error_inv * eta_error_inv;

    // from MuonHoughPattern::updateParametersRPhi (partial code duplication.. :(
    // )

    double sum_etax{0.}, sum_etay{0.}, sum_phix{0.}, sum_phiy{0.};

    // calculate average point

    for (unsigned int i = 0; i < etasize; i++) {
        const Trk::PrepRawData* prd = etapattern.prd(i);
        const Amg::Vector3D& globalposhit = globalPos(prd);
        sum_etax += globalposhit.x();
        sum_etay += globalposhit.y();
    }

    for (unsigned int i = 0; i < phisize; i++) {
        const Trk::PrepRawData* prd = phipattern.prd(i);
        const Amg::Vector3D& globalposhit = globalPos(prd);
        sum_phix += globalposhit.x();
        sum_phiy += globalposhit.y();
    }

    const double av_x = (eta_error_inv2 * sum_etax + phi_error_inv2 * sum_phix) / (eta_error_inv2 * etasize + phi_error_inv2 * phisize);
    const double av_y = (eta_error_inv2 * sum_etay + phi_error_inv2 * sum_phiy) / (eta_error_inv2 * etasize + phi_error_inv2 * phisize);

     ATH_MSG_VERBOSE(" av_x: " << av_x << " av_y: " << av_y);

    // calculate weighted sum:

    double sumx {0.}, sumy {0.};

    // keep track of extreme points

    double x_min {0.}, x_max {0.}, y_min {0.}, y_max {0.}, lever_min {0.}, lever_max{0.};

    for (unsigned int i = 0; i < etasize; i++) {
        const Trk::PrepRawData* prd = etapattern.prd(i);
        const Amg::Vector3D& globalposhit = globalPos(prd);
        double x_offset = globalposhit.x() - av_x;
        double y_offset = globalposhit.y() - av_y;
        double height_squared = x_offset * x_offset + y_offset * y_offset;
        double weight = height_squared * eta_error_inv2;
        int sign = 1;
        if (x_offset * scphi_est.cs + y_offset * scphi_est.sn < 0) { sign = -1; }
        sumx += weight * sign * x_offset;
        sumy += weight * sign * y_offset;

        if (sign == 1 && height_squared > lever_max) {
            lever_max = height_squared;
            x_max = globalposhit.x();
            y_max = globalposhit.y();
        } else if (sign == -1 && height_squared > lever_min) {
            lever_min = height_squared;
            x_min = globalposhit.x();
            y_min = globalposhit.y();
        }
    }

    for (unsigned int i = 0; i < phisize; i++) {
        const Trk::PrepRawData* prd = phipattern.prd(i);
        const Amg::Vector3D& globalposhit = globalPos(prd);
        double x_offset = globalposhit.x() - av_x;
        double y_offset = globalposhit.y() - av_y;
        double height_squared = x_offset * x_offset + y_offset * y_offset;
        double weight = height_squared * phi_error_inv2;
        int sign = 1;
        if (x_offset * scphi_est.cs + y_offset * scphi_est.sn < 0) { sign = -1; }
        sumx += weight * sign * x_offset;
        sumy += weight * sign * y_offset;

        if (sign == 1 && height_squared > lever_max) {
            lever_max = height_squared;
            x_max = globalposhit.x();
            y_max = globalposhit.y();
        } else if (sign == -1 && height_squared > lever_min) {
            lever_min = height_squared;
            x_min = globalposhit.x();
            y_min = globalposhit.y();
        }
    }

    ATH_MSG_VERBOSE("av_x : " << av_x << " av_y: " << av_y << " sumx: " << sumx << " sumy: " << sumy);

    if (std::abs(sumx) < 0.000001 || std::abs(sumy) < 0.000001) {
        ATH_MSG_DEBUG(" sum too small to update");

        return std::make_pair(phi_phipattern, MuonHoughMathUtils::signedDistanceOfLineToOrigin2D(phiglobalpos.x(), phiglobalpos.y(), phi_phipattern));
    }

    // lever arm has to be larger than 2 m, else no update:
    if (std::hypot(x_max - x_min , y_max - y_min) < 2000) {
        ATH_MSG_VERBOSE("lever arm too small: av_x : " << std::sqrt((x_max - x_min) * (x_max - x_min) + (y_max - y_min) * (y_max - y_min))
                                                       << " x_max: " << x_max << " x_min: " << x_min << " y_max: " << y_max
                                                       << " y_min: " << y_min);
        return std::make_pair(phi_phipattern, MuonHoughMathUtils::signedDistanceOfLineToOrigin2D(phiglobalpos.x(), phiglobalpos.y(), phi_phipattern));
    }

    double phi_fit = std::atan2(sumy, sumx);
    if (phi_fit > 0) phi_fit -= M_PI;  // phi between 0,-Pi for cosmics!
    CxxUtils::sincos scphi(phi_fit);
    const double r0_fit = scphi.apply(av_x, -av_y);  // av_x * scphi.sn - av_y * scphi.cs;

    return std::make_pair(phi_fit, r0_fit);
}

double MuonCombinePatternTool::calculateRz0(const Muon::MuonPrdPattern& pattern, double phi, double theta) {
    double nhits = pattern.numberOfContainedPrds();
    CxxUtils::sincos sctheta(theta);
    CxxUtils::sincos scphi(phi);

    /*
      x = r_0 sin(phi) + t*cos(phi)sin(theta)
      y = - r_0 cos(phi) + t*sin(phi)sin(theta)
      z = z_0 + t*cos(theta)

      2 methods (average point of average_z0):

      r^2 = x^2+y^2 = r_0^2 + t^2*sin^2(theta)
      (this projects the hit radially onto the line)

      Not so good as the radius of the hits can be smaller than r_0

      method based on:

      x*cos(phi) + y*sin(phi) = t*sin(theta)
      (projects the hit on the line in x-y and calculates the distance to r_0)

      works best
    */

    // method 3:

    double rz0 = 0.;
    for (unsigned int i = 0; i < nhits; i++) {
        const Trk::PrepRawData* prd = pattern.prd(i);
        const Amg::Vector3D& poshit = globalPos(prd);
        int sign = (poshit.x() * scphi.cs + poshit.y() * scphi.sn > 0) ? 1 : -1;
        rz0 += poshit.z() * sctheta.sn - sign * sctheta.cs * poshit.perp();
    }

    if (nhits > 0) rz0 /= nhits;
    return rz0;
}

MuonCombinePatternTool::PrdPatternPair MuonCombinePatternTool::updatePatternsForCosmics(
    const Muon::MuonPrdPattern& phipattern, const Muon::MuonPrdPattern& etapattern, const std::array<double,4>&  new_pars) const  {
    double phi = new_pars[1];
    double theta = new_pars[3];

    CxxUtils::sincos scphi(phi);
    CxxUtils::sincos sctheta(theta);

    double x0 = new_pars[0] * scphi.sn;
    double y0 = -new_pars[0] * scphi.cs;
    double z0_phi = new_pars[2];
    double z0_eta = new_pars[2];
    if (std::abs(sctheta.sn) > 1e-7) {
        z0_phi = (new_pars[2] + new_pars[0] * sctheta.cs) / sctheta.sn;  // z0 belonging to (x0,y0)
        z0_eta = new_pars[2] / sctheta.sn;                               // z0 of rz0
    }

    const Amg::Vector3D globalDir{scphi.cs * sctheta.sn, scphi.sn * sctheta.sn, sctheta.cs};
    const Amg::Vector3D globalPosPhi{x0, y0, z0_phi};
    const Amg::Vector3D globalPosEta{x0, y0, z0_eta};
    ATH_MSG_VERBOSE("updatePatternsForCosmics() -- eta pattern "<<std::endl
                    <<m_printer->print(*etapattern.prd(0))<<std::endl
                    <<" -- phi pattern "<<std::endl
                    <<m_printer->print(*phipattern.prd(0))<<std::endl
                    <<"Theta: " <<theta<<"   globalPosEta: "<<Amg::toString(globalPosEta)<<", globalDir: "<<Amg::toString(globalDir));
    
    
    std::unique_ptr<Muon::MuonPrdPattern> updatedphipattern = std::make_unique<Muon::MuonPrdPattern>(globalPosPhi, globalDir, phipattern.prepRawDataVec());
    std::unique_ptr<Muon::MuonPrdPattern> updatedetapattern = std::make_unique<Muon::MuonPrdPattern>(globalPosEta, globalDir, etapattern.prepRawDataVec());

    return std::make_pair(std::move(updatedphipattern), std::move(updatedetapattern));
}

std::unique_ptr<MuonPatternCombinationCollection> MuonCombinePatternTool::makePatternCombinations(const MuonPrdPatternCollection& muonpatterns) const {
    ATH_MSG_DEBUG("makePatternCombinations");

    std::unique_ptr<MuonPatternCombinationCollection> patterncombinations = std::make_unique<MuonPatternCombinationCollection>();

    for (const Muon::MuonPattern* pit : muonpatterns) {
        const Amg::Vector3D roadmom = pit->globalDirection();
        const Amg::Vector3D roadpos = pit->globalPosition();
        ATH_MSG_DEBUG("phi: " << roadmom.phi() << " eta: " << roadmom.eta());
        ATH_MSG_DEBUG("x: " << roadpos.x() << " y: " << roadpos.y() << " z: " << roadpos.z());

        // sort pattern per chamber
        std::map<Identifier, std::vector<const Trk::PrepRawData*>> chamberMap;        
        for (unsigned int i = 0; i < pit->numberOfContainedPrds(); ++i) {
            const Trk::PrepRawData* prd = pit->prd(i);
            Identifier channelId = prd->identify();
            const Identifier moduleId = m_idHelperSvc->chamberId(channelId);            
            std::vector<const Trk::PrepRawData*>& chambVec = chamberMap[moduleId];
            if (chambVec.empty()) chambVec.reserve(pit->numberOfContainedPrds());
            chambVec.push_back(prd);
        }

        // build chamberintersect vector
        std::vector<Muon::MuonPatternChamberIntersect> mpciVec;
        mpciVec.reserve(chamberMap.size());
        Amg::Vector3D patpose{Amg::Vector3D::Zero()}, patdire{Amg::Vector3D::Zero()};
        for ( const auto& [moduleId, chambVec] : chamberMap) {
            const Trk::PrepRawData* prd = chambVec.front();           
            const Amg::Vector3D& globalpos = globalPos(prd);
            if (m_use_cosmics) {
                // not flip
                patdire = roadmom.unit();
                patpose = roadpos;
            } else {
                MuonHoughMathUtils::extrapolateCurvedRoad(roadpos, roadmom, globalpos, patpose, patdire);
            }

            Muon::MuonPatternChamberIntersect mpci = Muon::MuonPatternChamberIntersect(patpose, patdire, chambVec);
            mpciVec.push_back(mpci);
        }

        ATH_MSG_VERBOSE(__FILE__<<":"<<__LINE__<<"  Pattern position "<<Amg::toString(roadpos));
        Amg::Vector3D tvertex{Amg::Vector3D::Zero()};
        Trk::TrackParameters* parameters = new Trk::Perigee(roadpos, roadmom, 1., tvertex);  // if -1 then charge flipped anyway
        Muon::MuonPatternCombination* combination = new Muon::MuonPatternCombination(parameters, mpciVec);
        patterncombinations->push_back(combination);
    }
    return patterncombinations;
}

bool MuonCombinePatternTool::subset(const Muon::MuonPrdPattern* pattern1, const Muon::MuonPrdPattern* pattern2) {
    /** is pattern1 a complete subset of other pattern2? */

    // first check if pattern 1 is not larger than 2:
    if (pattern1->numberOfContainedPrds() > pattern2->numberOfContainedPrds()) { return false; }

    PrepDataSet hits;
    for (unsigned int hitnr = 0; hitnr < pattern2->numberOfContainedPrds(); ++hitnr) { hits.insert(pattern2->prd(hitnr)); }

    for (unsigned int hitnr = 0; hitnr < pattern1->numberOfContainedPrds(); ++hitnr) {
        if (!hits.count(pattern1->prd(hitnr))) {
           return false;
        }
    }
    return true;
}

bool MuonCombinePatternTool::subset(std::pair<PrepDataSet,
                                              PrepDataSet>& candidate1,
                                    std::pair<PrepDataSet,
                                              PrepDataSet>& candidate2) {
    /** is candidate1 a complete subset of other candidate2? */

    // first check if pattern 1 is not larger than 2:
    if (candidate1.first.size() > candidate2.first.size() || 
        candidate1.second.size() > candidate2.second.size()) { return false; }
    for (const Trk::PrepRawData* find_me : candidate1.first) {
        if (!candidate2.first.count(find_me)) { return false; }
    }
    for (const Trk::PrepRawData* find_me :  candidate1.second) {
        if (candidate2.second.count(find_me)) { return false; }
    }

    return true;
}

void MuonCombinePatternTool::printPattern(const Muon::MuonPrdPattern* muonpattern) const {
    if (msgLvl(MSG::VERBOSE)) {
        ATH_MSG_VERBOSE("Printout of Pattern: ");

        const Amg::Vector3D& pos = muonpattern->globalPosition();
        const Amg::Vector3D& dir = muonpattern->globalDirection();

        ATH_MSG_VERBOSE("pos: x: " << pos.x() << " y: " << pos.y() << " z: " << pos.z());
        ATH_MSG_VERBOSE("dir: x: " << dir.x() << " y: " << dir.y() << " z: " << dir.z());
        ATH_MSG_VERBOSE("phi: " << dir.phi() << " theta: " << dir.theta() << " rz0: " << pos.z() * std::sin(dir.theta()));

        for (unsigned int k = 0; k < muonpattern->numberOfContainedPrds(); k++) {
            const Trk::PrepRawData* prd = muonpattern->prd(k);
            const Muon::MdtPrepData* mdtprd = dynamic_cast<const Muon::MdtPrepData*>(prd);
            if (mdtprd) {
                const Trk::Surface& surface = mdtprd->detectorElement()->surface(mdtprd->identify());
                const Amg::Vector3D& gpos = surface.center();
                ATH_MSG_VERBOSE("mdt " << k << " x: " << gpos.x() << " y: " << gpos.y() << " z: " << gpos.z());
            } else if (!mdtprd) {
                const Muon::MuonCluster* muoncluster = dynamic_cast<const Muon::MuonCluster*>(prd);
                if (muoncluster) {
                    const Amg::Vector3D& gpos = muoncluster->globalPosition();
                    ATH_MSG_VERBOSE("cluster " << k << " x: " << gpos.x() << " y: " << gpos.y() << " z: " << gpos.z());
                }
                if (!muoncluster) { ATH_MSG_VERBOSE("no muon prd? "); }
            }
        }
    }
}

std::unique_ptr<Muon::MuonPrdPattern> MuonCombinePatternTool::cleanPhiPattern(std::unique_ptr<Muon::MuonPrdPattern> phipattern) const {
    const Amg::Vector3D& olddir = phipattern->globalDirection();
    const double theta = olddir.theta();
    const unsigned int size = phipattern->numberOfContainedPrds();

    if (msgLvl(MSG::DEBUG)) {
        ATH_MSG_DEBUG("Start Phi hits cleaning with " << size << " hits "
                                                      << " theta " << theta);
        const Amg::Vector3D& oldpos = phipattern->globalPosition();
        double r0 = MuonHoughMathUtils::signedDistanceOfLineToOrigin2D(oldpos.x(), oldpos.y(), olddir.phi());
        ATH_MSG_DEBUG("Start Phi: " << olddir.phi() << " r0: " << r0);
    }

    // need internal class to be able to remove hits fast
    std::unique_ptr<MuonHoughPattern> newpattern = std::make_unique<MuonHoughPattern>(MuonHough::hough_xy);
    for (unsigned int phihitnr = 0; phihitnr < size; phihitnr++) { newpattern->addHit(std::make_shared<MuonHoughHit>(phipattern->prd(phihitnr))); }
    newpattern->updateParametersRPhi(m_use_cosmics);
    double phi = newpattern->getEPhi();
    double r0 = newpattern->getERPhi();

    CxxUtils::sincos scphi(phi);

    constexpr int number_of_iterations = 4;
    std::array<double,number_of_iterations> cutvalues {1000., 500., 250., 125.};
    if (m_use_cosmics) {
        cutvalues[0] = 5000.;
        cutvalues[1] = 2500.;
        cutvalues[2] = 1250.;
        cutvalues[3] = 1250.;
    }

    for (int it = 0; it < number_of_iterations; ++it) {
        ATH_MSG_VERBOSE("iteration " << it << " cutvalue: " << cutvalues[it]);
        bool change = true;
        while (change) {
            ATH_MSG_VERBOSE("size: " << newpattern->size() << " r0: " << r0 << " phi: " << phi);

            double max_dist = 0.;
            unsigned int max_i = 99999;
            for (unsigned int i = 0; i < newpattern->size(); i++) {
                double dist =
                    scphi.apply(newpattern->getHitx(i), -newpattern->getHity(i)) - r0;  //  newpattern->getHitx(i) * scphi.sn -
                                                                                        //  newpattern->getHity(i) * scphi.cs - r0;
                ATH_MSG_VERBOSE("Dist: " << dist);
                if (std::fabs(dist) > std::abs(max_dist)) {
                    max_dist = dist;
                    max_i = i;
                }
            }
            if (std::abs(max_dist) < cutvalues[it]) {
                change = false;
            } else {
                newpattern->removeHit(max_i);
                newpattern->updateParametersRPhi(m_use_cosmics);
                phi = newpattern->getEPhi();
                r0 = newpattern->getERPhi();
                scphi = CxxUtils::sincos(phi);
            }
        }
    }

    ATH_MSG_DEBUG("Final size: " << newpattern->size() << " r0: " << r0 << " phi: " << phi);

    // update parameters rz (not very important as later overwritten)
    // put r0 to IP for collisions

    double thetanew = 0.;
    double r0_new = 1.;  // put 1 mm r0 value

    if (m_use_cosmics) { r0_new = r0; }

    unsigned int nPatterns = newpattern->size();
    for (unsigned int i = 0; i < nPatterns; i++) { thetanew += newpattern->getTheta(i); }

    if (nPatterns > 0) thetanew /= nPatterns;

    double z0_new = 0.;

    double x0_new = r0_new * scphi.sn;
    double y0_new = -r0_new * scphi.cs;
    CxxUtils::sincos sctheta(thetanew);

    const Amg::Vector3D pos {x0_new, y0_new, z0_new};
    const Amg::Vector3D dir {sctheta.sn * scphi.cs, sctheta.sn * scphi.sn, sctheta.cs};

    std::unique_ptr<Muon::MuonPrdPattern> cleanpattern = std::make_unique<Muon::MuonPrdPattern>(pos, dir);

    for (unsigned int i = 0; i < newpattern->size(); i++) { cleanpattern->addPrd(newpattern->getPrd(i)); }

    return cleanpattern;
}

void MuonCombinePatternTool::addCandidate(const CandPrdPatPtr& etapattern, const CandPrdPatPtr& phipattern,
                                          std::vector<CandidatePatPair>& candidates, bool add_asspattern,
                                          const EtaPhiHitAssocMap& phiEtaHitAssMap) const {
    if (!m_use_cosmics || !m_splitpatterns) {
        candidates.emplace_back(etapattern, phipattern);
        return;
    }

    std::vector<PrdPatternPair> splitpatterns = splitPatternsCylinder(phipattern.get(), etapattern.get());

    if (splitpatterns.empty()) {
        candidates.emplace_back(etapattern, phipattern);
    }

    else {
        for (auto& [phiPattern, etaPattern] :  splitpatterns) {
            // skip when empty eta pattern , possible duplication when associated phi
            // pattern is found, but then will be cleaned later
            if (etaPattern->numberOfContainedPrds() == 0) {
                continue;
            }   
            candidates.emplace_back(std::move(etaPattern), std::move(phiPattern));
        }
    }

    // make associated pattern don't split eta pattern yet, but split based on phi
    // of ass. pattern bool asspattern_added = false;
    if (!add_asspattern) { return ;}
    std::unique_ptr<Muon::MuonPrdPattern> assphipattern = makeAssPhiPattern(*etapattern, phiEtaHitAssMap, true);
    
    if (!assphipattern) { return;}
    
    // print associated pattern:
    if (msgLvl(MSG::VERBOSE)) {
        ATH_MSG_VERBOSE("Associated Pattern: ");
        printPattern(assphipattern.get());
    }

    // update parameters:
    std::array<double,4> new_pars = updateParametersForCosmics(*assphipattern, *etapattern);
    PrdPatternPair updatedpatterns = updatePatternsForCosmics(*assphipattern, *etapattern, new_pars);
    
    std::unique_ptr<Muon::MuonPrdPattern>& cosmicPhiPattern = updatedpatterns.first;
    std::unique_ptr<Muon::MuonPrdPattern>& cosmicEtaPattern = updatedpatterns.second;
    
    std::vector<PrdPatternPair> splitpatterns_ass = splitPatternsCylinder(cosmicPhiPattern.get(), cosmicEtaPattern.get());
    if (splitpatterns_ass.empty()) {
        candidates.emplace_back(std::move(cosmicEtaPattern), std::move(cosmicPhiPattern));
        return;
    }

    
    for (auto& [splitPhiPattern, splitEtaPattern] : splitpatterns_ass) {
        if (splitPhiPattern->numberOfContainedPrds() == 0 ||
            splitEtaPattern->numberOfContainedPrds() == 0) {
            continue;
        }   
        candidates.emplace_back(std::move(splitEtaPattern), std::move(splitPhiPattern));
    }    
}

void MuonCombinePatternTool::cleanCandidates(std::vector<CandidatePatPair>& candidates) {
    std::vector<CandidatePatPair>::iterator it1;
    std::vector<CandidatePatPair>::iterator it2;   
    // map between set of prd's (eta and phi) and candidates , stored for speed
    std::map<CandidatePatPair, std::pair<PrepDataSet, PrepDataSet>> hitsMap;

    // fill map
    for (it1 = candidates.begin(); it1 != candidates.end(); ++it1) {
        PrepDataSet etahits;
        for (unsigned int hitnr = 0; hitnr < (*it1).first->numberOfContainedPrds(); hitnr++) { etahits.insert((*it1).first->prd(hitnr)); }
        PrepDataSet phihits;
        if ((*it1).second) {  // phi pattern might be 0!
            for (unsigned int hitnr = 0; hitnr < (*it1).second->numberOfContainedPrds(); hitnr++) {
                phihits.insert((*it1).second->prd(hitnr));
            }
        }
        hitsMap.insert(std::make_pair((*it1), std::make_pair(etahits, phihits)));
    }
    for (it1 = candidates.begin(); it1 != candidates.end(); ++it1) {
        std::pair<PrepDataSet, PrepDataSet>& hits1 = hitsMap[(*it1)];
        it2 = it1 + 1;
        while (it2 != candidates.end()) {
            std::pair<PrepDataSet, PrepDataSet>& hits2 = hitsMap[(*it2)];
            if (subset((hits2), (hits1))) {         // 2 subset of 1, remove 2 // in case of
                                                    // equality best (earliest) is kept
                it2 = candidates.erase(it2);        // it2 points to next item, it1 not invalidated!
            } else if (subset((hits1), (hits2))) {  // 1 subset of 2, remove 1
                it1 = candidates.erase(it1);        // it1 points to next item, it2 invalidated!
                it2 = it1 + 1;
                //cppcheck-suppress selfAssignment
                hits1 = hitsMap[(*it1)];  // redefine hits1
            } else {
                ++it2;
            }
        }
    }
}
