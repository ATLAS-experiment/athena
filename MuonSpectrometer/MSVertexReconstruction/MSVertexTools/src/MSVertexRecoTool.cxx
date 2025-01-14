/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MSVertexRecoTool.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "EventPrimitives/EventPrimitivesHelpers.h" //Amg::error
#include "AthenaKernel/RNGWrapper.h"

#include "CLHEP/Random/RandFlat.h"
#include "TMath.h"  


/*
  MS vertex reconstruction routine
  See documentation at https://cds.cern.ch/record/1455664 and https://cds.cern.ch/record/1520894
  Takes Tracklets as input and creates vertices in the MS volume
*/

namespace {
    constexpr unsigned int MAXPLANES = 100; 
}

namespace Muon {

    //** ----------------------------------------------------------------------------------------------------------------- **//

    MSVertexRecoTool::MSVertexRecoTool(const std::string& type, const std::string& name, const IInterface* parent) :
        AthAlgTool(type, name, parent) {
        declareInterface<IMSVertexRecoTool>(this);
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    StatusCode MSVertexRecoTool::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_rndmSvc.retrieve());

        ATH_CHECK(m_extrapolator.retrieve());
        ATH_CHECK(m_xAODContainerKey.initialize());
        ATH_CHECK(m_rpcTESKey.initialize());
        ATH_CHECK(m_tgcTESKey.initialize());
        ATH_CHECK(m_mdtTESKey.initialize());

        return StatusCode::SUCCESS;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    StatusCode MSVertexRecoTool::findMSvertices(const std::vector<Tracklet>& tracklets, std::vector<std::unique_ptr<MSVertex>>& vertices,
                                                const EventContext& ctx) const {
        ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this);
        rngWrapper->setSeed(name(), ctx);
        CLHEP::HepRandomEngine* rndmEngine = rngWrapper->getEngine(ctx);

        SG::WriteHandle<xAOD::VertexContainer> xAODVxContainer(m_xAODContainerKey, ctx);
        ATH_CHECK(xAODVxContainer.record(std::make_unique<xAOD::VertexContainer>(), std::make_unique<xAOD::VertexAuxContainer>()));

        // decorators for the number of hits in the MDT, RPC, and TGC layers
        std::vector<SG::AuxElement::Decorator<int>> nMDT_decs;
        for (const std::string& str : m_decMDT_str) nMDT_decs.push_back(SG::AuxElement::Decorator<int>(str));
        std::vector<SG::AuxElement::Decorator<int>> nRPC_decs;
        for (const std::string& str : m_decRPC_str) nRPC_decs.push_back(SG::AuxElement::Decorator<int>(str));
        std::vector<SG::AuxElement::Decorator<int>> nTGC_decs;
        for (const std::string& str : m_decTGC_str) nTGC_decs.push_back(SG::AuxElement::Decorator<int>(str));

        if (tracklets.size() < 3) { 
            ATH_MSG_DEBUG("Fewer than 3 tracks found, vertexing not possible. Exiting...");
            return StatusCode::SUCCESS; 
        }

        if (tracklets.size() > m_maxGlobalTracklets) {
            ATH_MSG_DEBUG("Too many tracklets found globally. Exiting...");
            return StatusCode::SUCCESS;
        }

        // group the tracks
        std::vector<Tracklet> BarrelTracklets;
        std::vector<Tracklet> EndcapTracklets;
        for (const Tracklet &tracklet : tracklets){
            if (m_idHelperSvc->mdtIdHelper().isBarrel(tracklet.muonIdentifier()))
                BarrelTracklets.push_back(tracklet);
            else if (m_idHelperSvc->mdtIdHelper().isEndcap(tracklet.muonIdentifier()))
                EndcapTracklets.push_back(tracklet);
        }

        if (BarrelTracklets.size() > m_maxClusterTracklets || EndcapTracklets.size() > m_maxClusterTracklets) {
            ATH_MSG_DEBUG("Too many tracklets found in barrel or endcap for clustering. Exiting...");
            return StatusCode::SUCCESS;
        }

        ATH_MSG_DEBUG("Running on event with " << BarrelTracklets.size() << " barrel tracklets, " << EndcapTracklets.size()
                                               << " endcap tracklets.");

        // find any clusters of tracks & decide if tracks are from single muon
        std::vector<Muon::MSVertexRecoTool::TrkCluster> BarrelClusters = findTrackClusters(BarrelTracklets);
        std::vector<Muon::MSVertexRecoTool::TrkCluster> EndcapClusters = findTrackClusters(EndcapTracklets);

        // if doSystematics, remove tracklets according to the tracklet reco uncertainty and rerun the cluster finder
        if (m_doSystematics) {
            std::vector<Tracklet> BarrelSystTracklets, EndcapSystTracklets;
            for (const Tracklet &BarrelTracklet : BarrelTracklets) {
                double prob = CLHEP::RandFlat::shoot(rndmEngine, 0, 1);
                if (prob > m_BarrelTrackletUncert) BarrelSystTracklets.push_back(BarrelTracklet);
            }
            if (BarrelSystTracklets.size() >= 3) {
                std::vector<Muon::MSVertexRecoTool::TrkCluster> BarrelSystClusters = findTrackClusters(BarrelSystTracklets);
                for (Muon::MSVertexRecoTool::TrkCluster &BarrelSystCluster : BarrelSystClusters) {
                    BarrelSystCluster.isSystematic = true;
                    BarrelClusters.push_back(BarrelSystCluster);
                }
            }

            for (const Tracklet &EndcapTracklet : EndcapTracklets) {
                double prob = CLHEP::RandFlat::shoot(rndmEngine, 0, 1);
                if (prob > m_EndcapTrackletUncert) EndcapSystTracklets.push_back(EndcapTracklet);
            }
            if (EndcapSystTracklets.size() >= 3) {
                std::vector<Muon::MSVertexRecoTool::TrkCluster> EndcapSystClusters = findTrackClusters(EndcapSystTracklets);
                for (Muon::MSVertexRecoTool::TrkCluster &EndcapSystCluster : EndcapSystClusters) {
                    EndcapSystCluster.isSystematic = true;
                    EndcapClusters.push_back(EndcapSystCluster);
                }
            }
        }

        ///////////////////////////////////////////VERTEX ROUTINES/////////////////////////////////////////
        // find vertices in the barrel MS (vertices using barrel tracklets)
        for (const Muon::MSVertexRecoTool::TrkCluster &BarrelCluster : BarrelClusters) {
            if (BarrelCluster.ntrks < 3) continue;
            ATH_MSG_DEBUG("Attempting to build vertex from " << BarrelCluster.ntrks << " tracklets in the barrel");
            std::unique_ptr<MSVertex> barvertex(nullptr);
            MSVxFinder(BarrelCluster.tracks, barvertex, ctx);
            if (!barvertex) continue;
            // barrel minimum good vertex criteria
            if (barvertex->getChi2Probability() > m_VxChi2ProbCUT) {
                HitCounter(barvertex.get(), ctx);
                if (barvertex->getNMDT() > m_MinMDTHits && (barvertex->getNRPC() + barvertex->getNTGC()) > m_MinTrigHits) {
                    ATH_MSG_DEBUG("Vertex found in the barrel with n_trk = " << barvertex->getNTracks() << " located at (eta,phi) = ("
                                                                             << barvertex->getPosition().eta() << ", "
                                                                             << barvertex->getPosition().phi() << ")");
                    if (BarrelCluster.isSystematic) barvertex->setAuthor(3);
                    vertices.push_back(std::move(barvertex));
                } 
            }
        } 

        // find vertices in the endcap MS (vertices using endcap tracklets)
        for (const Muon::MSVertexRecoTool::TrkCluster &EndcapCluster : EndcapClusters) {
            if (EndcapCluster.ntrks < 3) continue;
            ATH_MSG_DEBUG("Attempting to build vertex from " << EndcapCluster.ntrks << " tracklets in the endcap");

            std::unique_ptr<MSVertex> endvertex(nullptr);
            if (m_useOldMSVxEndcapMethod)
                MSStraightLineVx_oldMethod(EndcapCluster.tracks, endvertex, ctx);
            else
                MSStraightLineVx(EndcapCluster.tracks, endvertex, ctx);

            if (!endvertex) continue;
            // endcap minimum good vertex criteria
            if (endvertex->getPosition().perp() < m_MaxLxyEndcap && std::abs(endvertex->getPosition().z()) < m_MaxZEndcap &&
                std::abs(endvertex->getPosition().z()) > m_MinZEndcap && endvertex->getNTracks() >= 3) {
                HitCounter(endvertex.get(), ctx);
                if (endvertex->getNMDT() > m_MinMDTHits && (endvertex->getNRPC() + endvertex->getNTGC()) > m_MinTrigHits) {
                    ATH_MSG_DEBUG("Vertex found in the endcap with n_trk = " << endvertex->getNTracks() << " located at (eta,phi) = ("
                                                                             << endvertex->getPosition().eta() << ", "
                                                                             << endvertex->getPosition().phi() << ")");
                    if (EndcapCluster.isSystematic) endvertex->setAuthor(4);
                    vertices.push_back(std::move(endvertex));
                } 
            }

        }  // end loop on endcap tracklet clusters

        ATH_CHECK(FillOutputContainer(vertices, xAODVxContainer, nMDT_decs, nRPC_decs, nTGC_decs));
        return StatusCode::SUCCESS;
    }  // end find vertices

    //** ----------------------------------------------------------------------------------------------------------------- **//

    std::optional<Muon::MSVertexRecoTool::TrkCluster> MSVertexRecoTool::ClusterizeTracks(std::vector<Tracklet>& tracks) const {
        // returns the best cluster of tracklets and removes them from the input vector for the next iteration
                
        if (tracks.size() > m_maxClusterTracklets) {
            ATH_MSG_DEBUG("Too many tracks found, returning empty cluster");
            return std::nullopt;
        }

        std::vector<TrkCluster> trkClu;
        std::vector<TrkCluster> trkClu0;
        // use each tracklet as a seed for the clusters
        int ncluster = 0;
        for (const Tracklet& trk : tracks) {
            TrkCluster clu;
            clu.eta = trk.globalPosition().eta();
            clu.phi = trk.globalPosition().phi();

            trkClu.push_back(clu);
            trkClu0.push_back(clu);
            ++ncluster;
            if (ncluster >= 99) {
                return std::nullopt;
            }
        }

        // loop on the clusters and let the center move to find the optimal cluster centers
        for (TrkCluster& clu : trkClu) {
            // add tracklets to the cluster and update it
            int ntracks = 0;
            for (const TrkCluster& trk : trkClu0) {
                double dEta = clu.eta - trk.eta;
                double dPhi = xAOD::P4Helpers::deltaPhi(clu.phi , trk.phi);
                if (std::abs(dEta) < m_ClusterdEta && std::abs(dPhi) < m_ClusterdPhi) {
                    ++ntracks;
                    clu.eta = clu.eta - dEta / ntracks;
                    clu.phi = xAOD::P4Helpers::deltaPhi(clu.phi - dPhi / ntracks, 0);
                }
            }  
    
            // the updated cluster position might cause tracklets considered near the start of the loop to now lie inside or outside the cluster
            // run over all tracklets again to check is any more tracklets are picked up. Is so, do this at most 5 times
            // *_best store the centre of the cluster containing the most tracklets 
            double eta_best = clu.eta;
            double phi_best = clu.phi;
            int nitr = 0;
            bool improvement = true;
            while (improvement) {
                unsigned int ntracks_new = 0;
                double eta_new = 0.0;
                double phi_new = 0.0;
                double cosPhi_new = 0.0;
                double sinPhi_new = 0.0;

                for (const TrkCluster& trk : trkClu0) {
                    double dEta = clu.eta - trk.eta;
                    double dPhi = xAOD::P4Helpers::deltaPhi(clu.phi , trk.phi);
                    if (std::abs(dEta) < m_ClusterdEta && std::abs(dPhi) < m_ClusterdPhi) {
                        eta_new += trk.eta;
                        cosPhi_new += std::cos(trk.phi);
                        sinPhi_new += std::sin(trk.phi);
                        ++ntracks_new;
                    }
                } 

                eta_new = eta_new / ntracks_new;
                phi_new = std::atan2(sinPhi_new, cosPhi_new);

                if (ntracks_new > clu.ntrks) {
                    // better cluster found - update the centre and number of tracklets 
                    eta_best = clu.eta; // not eta_new in case the iteration threshold was exceeded
                    phi_best = clu.phi;
                    clu.ntrks = ntracks_new;    
                    if (nitr < 6) {
                        // update the cluster for the next improvement iteration 
                        clu.eta = eta_new;
                        clu.phi = phi_new;
                    } 
                    else
                        break;
                } 
                else {
                    clu.eta = eta_best;
                    clu.phi = phi_best;
                    improvement = false;
                }
                ++nitr;
            }  // end while loop to check for cluster improvements
        }      // end loop over clusters 

        // find the best cluster as the one containing the most tracklets
        TrkCluster& BestCluster = trkClu[0];
        for (const TrkCluster& clu : trkClu) {
            if (clu.ntrks > BestCluster.ntrks) BestCluster = clu;
        }

        // store the tracks inside the cluster
        std::vector<Tracklet> unusedTracks;
        for (const Tracklet& trk : tracks) {
            double dEta = BestCluster.eta - trk.globalPosition().eta();
            double dPhi = xAOD::P4Helpers::deltaPhi(BestCluster.phi , trk.globalPosition().phi());
            if (std::abs(dEta) < m_ClusterdEta && std::abs(dPhi) < m_ClusterdPhi)
                BestCluster.tracks.push_back(trk);
            else
                unusedTracks.push_back(trk);
        }
        // return the best cluster and the unused tracklets
        tracks = std::move(unusedTracks);
        return BestCluster;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    std::vector<Muon::MSVertexRecoTool::TrkCluster> MSVertexRecoTool::findTrackClusters(const std::vector<Tracklet>& tracks) const {
        // only clusters with 3 or more tracklets are returned
        std::vector<Tracklet> trks = tracks;
        std::vector<TrkCluster> clusters;
        // keep making clusters until there are no more possible
        while (true) {
            if (trks.size() < 3) break;
            std::optional<TrkCluster> clust = ClusterizeTracks(trks);
            if (clust && clust->ntrks>=3) clusters.push_back(clust.value());
            else break;
        }

        return clusters;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    void MSVertexRecoTool::MSVxFinder(const std::vector<Tracklet>& tracklets, std::unique_ptr<MSVertex>& vtx,
                                      const EventContext& ctx) const {
        int nTrkToVertex(0);
        double NominalAngle(m_TrackPhiAngle.value()), RotationAngle(m_TrackPhiAngle.value() + m_TrackPhiRotation.value());
        
        Amg::Vector3D aveTrkPos(0, 0, 0);
        for (const Tracklet &trk : tracklets) aveTrkPos += trk.globalPosition();
        aveTrkPos /= tracklets.size();

        // calculate the two angles (theta & phi)
        double avePhi = aveTrkPos.phi();
        double LoF = std::atan2(aveTrkPos.perp(), aveTrkPos.z());  // Line of Flight (theta)
        avePhi = vxPhiFinder(std::abs(LoF), avePhi, ctx);

        // find the positions of the radial planes
        std::vector<double> Rpos;
        double RadialDist = m_VertexMaxRadialPlane - m_VertexMinRadialPlane;
        double LoFdist = std::abs(RadialDist / std::sin(LoF));
        int nplanes = LoFdist / m_VxPlaneDist + 1;
        double PlaneSpacing = std::abs(m_VxPlaneDist / std::cos(LoF));
        for (int k = 0; k < nplanes; ++k) Rpos.push_back(m_VertexMinRadialPlane + PlaneSpacing * k);

        // loop on barrel tracklets and create two types of track parameters -- nominal and phi shifted tracklets
        std::array< std::vector<std::unique_ptr<Trk::TrackParameters>>,  MAXPLANES> TracksForVertexing{};  // vector of tracklets to be used at each vertex plane
        std::array< std::vector<std::unique_ptr<Trk::TrackParameters>>, MAXPLANES> TracksForErrors{};  // vector of tracklets to be used for uncertainty at each vertex plane
        std::array< std::vector<bool>, MAXPLANES> isNeutralTrack{};

        for (const Tracklet &trk : tracklets) {
            if (m_idHelperSvc->isEndcap(trk.muonIdentifier())) continue;
            ++nTrkToVertex;
            // coordinate transform variables
            Amg::Vector3D trkgpos(trk.globalPosition().perp() * std::cos(avePhi),
                                  trk.globalPosition().perp() * std::sin(avePhi), 
                                  trk.globalPosition().z());
            double x0 = trkgpos.x();
            double y0 = trkgpos.y();
            double r0 = trkgpos.perp();

            // decide which way the tracklet gets rotated -- positive or negative phi
            double anglesign = ((trk.globalPosition().phi() - avePhi) < 0) ? -1.0 : 1.0;
            double NominalTrkAng = anglesign * NominalAngle;   // in case there is a nominal tracklet angle
            double MaxTrkAng = anglesign * RotationAngle;      // the rotated tracklet phi position

            // loop over the radial planes
            for (int k = 0; k < nplanes; ++k) {
                // only use tracklets that start AFTER the vertex plane
                if (Rpos[k] > trk.globalPosition().perp()) break;

                // nominal tracks for vertexing
                double Xp = Rpos[k] * std::cos(avePhi);
                double Yp = Rpos[k] * std::sin(avePhi);
                // in case there is a nominal opening angle, calculate tracklet direction
                // the tracklet must cross the candidate vertex plane at the correct phi
                double DelR = std::hypot(x0 - Xp, y0 - Yp) / std::cos(NominalAngle);
                double X1 = DelR * std::cos(NominalTrkAng + avePhi) + Xp;
                double Y1 = DelR * std::sin(NominalTrkAng + avePhi) + Yp;
                double R1 = std::hypot(X1, Y1);
                double Norm = r0 / R1;
                X1 = X1 * Norm;
                Y1 = Y1 * Norm;
                double Dirmag = std::hypot(X1 - Xp, Y1 - Yp);
                double Xdir = (X1 - Xp) / Dirmag;
                double Ydir = (Y1 - Yp) / Dirmag;
                double trkpx = Xdir * trk.momentum().perp();
                double trkpy = Ydir * trk.momentum().perp();
                double trkpz = trk.momentum().z();

                // check if the tracklet has a charge & momentum measurement -- if not, set charge=1 so extrapolator will work
                double charge = trk.charge();
                if (std::abs(charge) < 0.1) {
                    charge = 1;  // for "straight" tracks, set charge = 1
                    isNeutralTrack[k].push_back(true);
                } else
                    isNeutralTrack[k].push_back(false);

                // store the tracklet as a Trk::Perigee
                Amg::Vector3D trkmomentum(trkpx, trkpy, trkpz);
                Amg::Vector3D trkgpos(X1, Y1, trk.globalPosition().z());
                AmgSymMatrix(5) covariance = AmgSymMatrix(5)(trk.errorMatrix());
                TracksForVertexing[k].push_back(std::make_unique<Trk::Perigee>(0., 0., trkmomentum.phi(), trkmomentum.theta(), charge / trkmomentum.mag(), Trk::PerigeeSurface(trkgpos), covariance));

                // tracks for errors -- rotate the plane & recalculate the tracklet parameters
                double xp = Rpos[k] * std::cos(avePhi);
                double yp = Rpos[k] * std::sin(avePhi);
                double delR = std::hypot(x0 - xp, y0 - yp) / std::cos(RotationAngle);
                double x1 = delR * std::cos(MaxTrkAng + avePhi) + xp;
                double y1 = delR * std::sin(MaxTrkAng + avePhi) + yp;
                double r1 = std::hypot(x1, y1);
                double norm = r0 / r1;
                x1 = x1 * norm;
                y1 = y1 * norm;
                double dirmag = std::hypot(x1 - xp, y1 - yp);
                double xdir = (x1 - xp) / dirmag;
                double ydir = (y1 - yp) / dirmag;
                double errpx = xdir * trk.momentum().perp();
                double errpy = ydir * trk.momentum().perp();
                double errpz = trk.momentum().z();

                // store the tracklet as a Trk::Perigee
                AmgSymMatrix(5) covariance2 = AmgSymMatrix(5)(trk.errorMatrix());
                Amg::Vector3D trkerrmom(errpx, errpy, errpz);
                Amg::Vector3D trkerrpos(x1, y1, trk.globalPosition().z());
                TracksForErrors[k].push_back(std::make_unique<Trk::Perigee>(0., 0., trkerrmom.phi(), trkerrmom.theta(), charge / trkerrmom.mag(), Trk::PerigeeSurface(trkerrpos), covariance2));
            }  // end loop on vertex planes
        }      // end loop on tracks

        // return if there are not enough tracklets
        if (nTrkToVertex < 3) return;

        // calculate the tracklet positions and uncertainty on each surface
        bool boundaryCheck = true;
        std::array<std::vector<double>, MAXPLANES> ExtrapZ{};  // extrapolated z position
        std::array<std::vector<double>, MAXPLANES> dlength{};  // extrapolated z position uncertainty
        std::array<std::vector<std::pair<unsigned int, unsigned int>>, MAXPLANES> UsedTracks{};
        std::array<std::vector<bool>, MAXPLANES> ExtrapSuc{};  // did the extrapolation succeed?
        std::vector<std::unique_ptr<MSVertex>> vertices; vertices.reserve(nplanes);
        std::array<std::vector<double>, MAXPLANES> sigmaZ{}; // total uncertainty at each plane
        std::array<std::vector<Amg::Vector3D>, MAXPLANES> pAtVx{}; // tracklet momentum expressed at the plane

        // extrapolate tracklates and store results at each radial plane
        for (int k = 0; k < nplanes; ++k) { // loop on planes
            double rpos = Rpos[k];
            for (unsigned int i = 0; i < TracksForVertexing[k].size(); ++i) { // loop on tracklets
                // at least three tracklets per plane are needed
                if (TracksForVertexing[k].size() < 3) break;

                Amg::Transform3D surfaceTransformMatrix;
                surfaceTransformMatrix.setIdentity();
                Trk::CylinderSurface cyl(surfaceTransformMatrix, rpos, 10000.);  // create the surface
                // extrapolate to the surface
                std::unique_ptr<const Trk::TrackParameters> extrap_par(
                    m_extrapolator->extrapolate(ctx,
                                                *TracksForVertexing[k].at(i), cyl, Trk::anyDirection, boundaryCheck, Trk::muon));

                const Trk::AtaCylinder* extrap = dynamic_cast<const Trk::AtaCylinder*>(extrap_par.get());

                if (extrap) {
                    // if the track is neutral just store the uncertainty due to angular uncertainty of the orignal tracklet
                    if (isNeutralTrack[k].at(i)) {
                        double pTot = std::hypot(TracksForVertexing[k].at(i)->momentum().perp(), TracksForVertexing[k].at(i)->momentum().z());
                        double dirErr = Amg::error(*TracksForVertexing[k].at(i)->covariance(), Trk::theta);
                        double extrapRdist = TracksForVertexing[k].at(i)->position().perp() - Rpos[k];
                        double sz = std::abs(20 * dirErr * extrapRdist * std::pow(pTot,2) / std::pow(TracksForVertexing[k].at(i)->momentum().perp(), 2));
                        double ExtrapErr = sz;
                        if (ExtrapErr > m_MaxTrackUncert)
                            ExtrapSuc[k].push_back(false);
                        else {
                            ExtrapSuc[k].push_back(true);
                            std::pair<unsigned int, unsigned int> trkmap(ExtrapZ[k].size(), i);
                            UsedTracks[k].push_back(trkmap);
                            ExtrapZ[k].push_back(extrap->localPosition().y());
                            sigmaZ[k].push_back(sz);
                            pAtVx[k].push_back(extrap->momentum());
                            dlength[k].push_back(0);
                        }
                    }  // end neutral tracklets
                    // if the tracklet has a momentum measurement
                    else {
                        // now extrapolate taking into account the extra path length & differing magnetic field
                        Amg::Transform3D srfTransMat2;
                        srfTransMat2.setIdentity();
                        Trk::CylinderSurface cyl2(srfTransMat2, rpos, 10000.);
                        std::unique_ptr<const Trk::TrackParameters> extrap_par2(
                            m_extrapolator->extrapolate(ctx,
                                                        *TracksForErrors[k].at(i), cyl, Trk::anyDirection, boundaryCheck, Trk::muon));
                        const Trk::AtaCylinder* extrap2 = dynamic_cast<const Trk::AtaCylinder*>(extrap_par2.get());

                        if (extrap2) {
                            double sz = Amg::error(*extrap->covariance(), Trk::locY);
                            double zdiff = extrap->localPosition().y() - extrap2->localPosition().y();
                            double ExtrapErr = std::hypot(sz, zdiff);
                            if (ExtrapErr > m_MaxTrackUncert)
                                ExtrapSuc[k].push_back(false);
                            else {
                                // iff both extrapolations succeed && error is acceptable, store the information
                                ExtrapSuc[k].push_back(true);
                                std::pair<unsigned int, unsigned int> trkmap(ExtrapZ[k].size(), i);
                                UsedTracks[k].push_back(trkmap);
                                ExtrapZ[k].push_back(extrap->localPosition().y());
                                sigmaZ[k].push_back(sz);
                                pAtVx[k].push_back(extrap->momentum());
                                dlength[k].push_back(zdiff);
                            }
                        } else
                            ExtrapSuc[k].push_back(false);  // not possible to calculate the uncertainty -- do not use tracklet in vertex
                    }
                } 
                // not possible to extrapolate the tracklet
                else
                    ExtrapSuc[k].push_back(false);  
            } // loop on tracklets
        } // loop on radial planes


        // perform the vertex fit
        std::array<std::vector<Amg::Vector3D>, MAXPLANES> trkp{}; // tracklet momentum 
        // loop on planes
        for (int k = 0; k < nplanes; ++k) {
            if (ExtrapZ[k].size() < 3) continue;  // require at least 3 tracklets to build a vertex
            // initialize the variables used in the routine
            double zLoF = Rpos[k] / std::tan(LoF);
            double dzLoF(10);
            double aveZpos(0), posWeight(0);
            for (unsigned int i = 0; i < ExtrapZ[k].size(); ++i) {
                double ExtrapErr = std::hypot(sigmaZ[k][i], dlength[k][i], dzLoF);
                if (isNeutralTrack[k][i]) ExtrapErr = std::hypot(sigmaZ[k][i], dzLoF);
                aveZpos += ExtrapZ[k][i] / std::pow(ExtrapErr,2);
                posWeight += 1. / std::pow(ExtrapErr,2);
            }
            // calculate the weighted average position of the tracklets
            zLoF = aveZpos / posWeight;
            double zpossigma(dzLoF), Chi2(0), Chi2Prob(-1);
            unsigned int Nitr(0);
            std::vector<unsigned int> vxtracks;  // tracklets to be used in the vertex routine
            std::vector<bool> blacklist(ExtrapZ[k].size(), false);         // tracklets that do not belong to the vertex

            // minimum chi^2 iterative fit
            while (true) {
                vxtracks.clear(); trkp[k].clear();
                int tmpnTrks(0);
                double tmpzLoF(0), tmpzpossigma(0), tmpchi2(0), posWeight(0), worstdelz(0);
                unsigned int iworst(0); // tracklet index contributing to the vertex chi2 the most
                // loop on the tracklets, find the chi^2 contribution from each tracklet
                for (unsigned int i = 0; i < ExtrapZ[k].size(); ++i) {
                    if (blacklist[i]) continue;
                    trkp[k].push_back(pAtVx[k][i]);
                    double delz = zLoF - ExtrapZ[k][i];
                    double ExtrapErr = std::hypot(sigmaZ[k][i], dlength[k][i], dzLoF);
                    double trkchi2 = std::pow(delz,2) / std::pow(ExtrapErr,2);
                    if (trkchi2 > worstdelz) {
                        iworst = i;
                        worstdelz = trkchi2;
                    }
                    tmpzLoF += ExtrapZ[k][i] / std::pow(ExtrapErr,2);
                    posWeight += 1. / std::pow(ExtrapErr,2);
                    tmpzpossigma += std::pow(delz,2);
                    tmpchi2 += trkchi2;
                    ++tmpnTrks;
                } 

                if (tmpnTrks < 3) break;  // stop searching for a vertex at this plane
                tmpzpossigma = std::sqrt(tmpzpossigma / (double)tmpnTrks);
                zLoF = tmpzLoF / posWeight;
                zpossigma = tmpzpossigma;
                double testChi2 = TMath::Prob(tmpchi2, tmpnTrks - 1);
                if (testChi2 < m_VxChi2ProbCUT)
                    blacklist[iworst] = true;
                else {
                    Chi2 = tmpchi2;
                    Chi2Prob = testChi2;
                    // loop on the tracklets and find all that belong to the vertex
                    for (unsigned int i = 0; i < ExtrapZ[k].size(); ++i) {
                        double delz = zLoF - ExtrapZ[k][i];
                        double ExtrapErr = std::hypot(sigmaZ[k][i], dlength[k][i], dzLoF);
                        double trkErr = std::hypot(ExtrapErr, zpossigma) + 0.001;
                        double trkNsigma = std::abs(delz / trkErr);
                        if (trkNsigma < 3) vxtracks.push_back(i);
                    }
                    break;  // found a vertex, stop removing tracklets! Break chi^2 iterative fit at this radial plane
                }
                if (Nitr >= (ExtrapZ[k].size() - 3)) break;  // stop searching for a vertex at this plane
                ++Nitr;
            }  // end while

            if (vxtracks.size() < 3) continue;

            // create TrackParticle vector for all tracklets used in the vertex fit
            std::vector<const xAOD::TrackParticle*> vxTrackParticles;
            vxTrackParticles.reserve(vxtracks.size());
            for (std::vector<unsigned int>::iterator vxtrk = vxtracks.begin(); vxtrk != vxtracks.end(); ++vxtrk) {
                for (unsigned int i = 0; i < UsedTracks[k].size(); ++i) {
                    if ((*vxtrk) != UsedTracks[k].at(i).first) continue;
                    const Tracklet& trklt = tracklets.at(UsedTracks[k].at(i).second);
                    vxTrackParticles.push_back(trklt.getTrackParticle());
                    break; // found the tracklet used for the vertex reconstruction in the tracklet collection. Hence can stop looking
                }
            }
            Amg::Vector3D position(Rpos[k] * std::cos(avePhi), Rpos[k] * std::sin(avePhi), zLoF);
            vertices.push_back(std::make_unique<MSVertex>(1, position, vxTrackParticles, Chi2Prob, Chi2, 0, 0, 0));
        }  // end loop on Radial planes

        if (vertices.empty()) return;

        // loop on the vertex candidates and select the best based on max n(tracks) and max chi^2 probability

        unsigned int bestVx(0);
        for (unsigned int k = 1; k < vertices.size(); ++k) {
            if (vertices[k]->getChi2Probability() < m_VxChi2ProbCUT || vertices[k]->getNTracks() < 3) continue;
            if (vertices[k]->getNTracks() < vertices[bestVx]->getNTracks()) continue;
            if (vertices[k]->getNTracks() == vertices[bestVx]->getNTracks() &&
                vertices[k]->getChi2Probability() < vertices[bestVx]->getChi2Probability())
                continue;
            bestVx = k;
        }
        vtx = std::make_unique<MSVertex>(*vertices[bestVx]);
        vertices.clear(); // cleanup
   }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    void MSVertexRecoTool::MSStraightLineVx(const std::vector<Tracklet>& trks, std::unique_ptr<MSVertex>& vtx,
                                            const EventContext& ctx) const {
        // Running set of all vertices found.  The inner set is the indices of trks that are used to make the vertex
        std::set<std::set<int> > prelim_vx;

        // We don't consider all 3-tracklet combinations when a  high number of tracklets is found
        // Faster method is used for > 40 tracklets
        if (trks.size() > 40) {
            MSStraightLineVx_oldMethod(trks, vtx, ctx);
            return;
        }

        // Okay, if we get here then we know there's 40 or fewer tracklets in the cluster.
        // Make a list of all 3-tracklet combinations that make vertices
        for (unsigned int i = 0; i < trks.size() - 2; ++i) {
            for (unsigned int j = i + 1; j < trks.size() - 1; ++j) {
                for (unsigned int k = j + 1; k < trks.size(); ++k) {
                    std::set<int> tmpTracks;
                    tmpTracks.insert(i);
                    tmpTracks.insert(j);
                    tmpTracks.insert(k);

                    Amg::Vector3D MyVx;
                    MyVx = VxMinQuad(getTracklets(trks, tmpTracks));
                    if (MyVx.perp() < 10000 && std::abs(MyVx.z()) > 7000 && std::abs(MyVx.z()) < 15000 &&
                        !EndcapHasBadTrack(getTracklets(trks, tmpTracks), MyVx))
                        prelim_vx.insert(tmpTracks);
                }
            }
        }

        // If no preliminary vertices were found from 3 tracklets, then there is no vertex and we are done.
        if (prelim_vx.empty()) return;

        // The remaining algorithm is very time consuming for large numbers of tracklets.  To control this,
        // we run the old algorithm when there are too many tracklets and a vertex is found.
        if (trks.size() <= 20) {
            std::set<std::set<int> > new_prelim_vx = prelim_vx;
            std::set<std::set<int> > old_prelim_vx;

            int foundNewVx = true;
            while (foundNewVx) {
                foundNewVx = false;

                old_prelim_vx = new_prelim_vx;
                new_prelim_vx.clear();

                for (std::set<std::set<int> >::iterator itr = old_prelim_vx.begin(); itr != old_prelim_vx.end(); ++itr) {
                    for (unsigned int i_trks = 0; i_trks < trks.size(); ++i_trks) {
                        std::set<int> tempCluster = *itr;
                        if (tempCluster.insert(i_trks).second) {
                            Amg::Vector3D MyVx = VxMinQuad(getTracklets(trks, tempCluster));
                            if (MyVx.perp() < 10000 && std::abs(MyVx.z()) > 7000 && std::abs(MyVx.z()) < 15000 &&
                                !EndcapHasBadTrack(getTracklets(trks, tempCluster), MyVx)) {
                                new_prelim_vx.insert(tempCluster);
                                prelim_vx.insert(tempCluster);
                                foundNewVx = true;
                            }
                        }
                    }
                }
            }
        } else {
            // Since there are 20 or more tracklets, we're going to use the old MSVx finding method.  Note that
            // if the old method fails, we do not return here; in this case a 3-tracklet vertex that was found
            // earlier in this algorithm will be returned
            MSStraightLineVx_oldMethod(trks, vtx, ctx);
            if (vtx) return;
        }

        // Find the preliminary vertex with the maximum number of tracklets - that is the final vertex.  If
        // multiple preliminary vertices with same number of tracklets, the first one found is returned
        std::set<std::set<int> >::iterator prelim_vx_max = prelim_vx.begin();
        for (std::set<std::set<int> >::iterator itr = prelim_vx.begin(); itr != prelim_vx.end(); ++itr) {
            if ((*itr).size() > (*prelim_vx_max).size()) prelim_vx_max = itr;
        }

        std::vector<Tracklet> tracklets = getTracklets(trks, *prelim_vx_max);
        // use tracklets to estimate the line of flight of decaying particle
        double aveX(0);
        double aveY(0);
        for (const Tracklet &trk : tracklets) {
            aveX += trk.globalPosition().x();
            aveY += trk.globalPosition().y();
        }

        Amg::Vector3D MyVx = VxMinQuad(tracklets);
        double vxtheta = std::atan2(MyVx.x(), MyVx.z());
        double tracklet_vxphi = std::atan2(aveY, aveX);
        double vxphi = vxPhiFinder(std::abs(vxtheta), tracklet_vxphi, ctx);

        Amg::Vector3D vxpos(MyVx.x() * std::cos(vxphi), MyVx.x() * std::sin(vxphi), MyVx.z());

        std::vector<const xAOD::TrackParticle*> vxTrackParticles;
        for (const Tracklet &trk : tracklets) vxTrackParticles.push_back(trk.getTrackParticle());

        vtx = std::make_unique<MSVertex>(2, vxpos, vxTrackParticles, 1, vxTrackParticles.size(), 0, 0, 0);
   }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    // vertex finding routine for the endcap
    void MSVertexRecoTool::MSStraightLineVx_oldMethod(const std::vector<Tracklet>& trks, std::unique_ptr<MSVertex>& vtx,
                                                      const EventContext& ctx) const {
        // find the line of flight
        double aveX(0), aveY(0);
        for (const Tracklet &trk : trks) {
            aveX += trk.globalPosition().x();
            aveY += trk.globalPosition().y();
        }
        double vxphi = std::atan2(aveY, aveX);

        Amg::Vector3D MyVx(0, 0, 0);
        std::vector<Tracklet> tracks = RemoveBadTrk(trks, MyVx);
        if (tracks.size() < 2) return;

        // remove back tracks one by one until non are considered bad (large distance from the vertex)
        while (true) {
            MyVx = VxMinQuad(tracks);
            std::vector<Tracklet> Tracks = RemoveBadTrk(tracks, MyVx);
            if (tracks.size() == Tracks.size()) break;
            tracks = std::move(Tracks);
        }

        if (tracks.size() >= 3 && MyVx.x() > 0) {
            double vxtheta = std::atan2(MyVx.x(), MyVx.z());
            vxphi = vxPhiFinder(std::abs(vxtheta), vxphi, ctx);
            Amg::Vector3D vxpos(MyVx.x() * std::cos(vxphi), MyVx.x() * std::sin(vxphi), MyVx.z());

            std::vector<const xAOD::TrackParticle*> vxTrackParticles;
            for (const Tracklet &trk : tracks) vxTrackParticles.push_back(trk.getTrackParticle());

            vtx = std::make_unique<MSVertex>(2, vxpos, vxTrackParticles, 1, (double)vxTrackParticles.size(), 0, 0, 0);
        }
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    std::vector<Tracklet> MSVertexRecoTool::RemoveBadTrk(const std::vector<Tracklet>& tracks, const Amg::Vector3D& Vx) const {
        // Removes at most one track with the largest distance to the vertex above the predefined threshold set by m_MaxTollDist 
        // check for default vertex
        if (Vx.x() == 0 && Vx.z() == 0) return tracks;

        double WorstTrkDist = m_MaxTollDist;
        unsigned int iWorstTrk = -1;
        for (unsigned int i = 0; i < tracks.size(); ++i) {
            double TrkSlope = std::tan(tracks.at(i).getML1seg().alpha());
            double TrkInter = tracks.at(i).getML1seg().globalPosition().perp() - tracks.at(i).getML1seg().globalPosition().z() * TrkSlope;
            double dist = std::abs((TrkSlope * Vx.z() - Vx.x() + TrkInter) / std::hypot(TrkSlope, 1));
            if (dist > m_MaxTollDist && dist > WorstTrkDist) {
                iWorstTrk = i;
                WorstTrkDist = dist;
            }
        }

        // Remove the worst track from the list
        std::vector<Tracklet> Tracks;
        for (unsigned int i = 0; i < tracks.size(); ++i) {
            if (i != iWorstTrk) Tracks.push_back(tracks.at(i));
        }
        return Tracks;
    }

    std::vector<Tracklet> MSVertexRecoTool::getTracklets(const std::vector<Tracklet>& trks, const std::set<int>& tracklet_subset) const {
        std::vector<Tracklet> returnVal;
        for (std::set<int>::const_iterator itr = tracklet_subset.cbegin(); itr != tracklet_subset.cend(); ++itr) {
            if ((unsigned int)*itr > trks.size()) ATH_MSG_ERROR("ERROR - Index out of bounds in getTracklets");
            returnVal.push_back(trks.at(*itr));
        }

        return returnVal;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    bool MSVertexRecoTool::EndcapHasBadTrack(const std::vector<Tracklet>& tracks, const Amg::Vector3D& Vx) const {
        if (Vx.x() == 0 && Vx.z() == 0) return true;
        // return true a track is further away from the vertex than m_MaxTollDist
        for (const Tracklet &track : tracks) {
            double TrkSlope = std::tan(track.getML1seg().alpha());
            double TrkInter = track.getML1seg().globalPosition().perp() - track.getML1seg().globalPosition().z() * TrkSlope;
            double dist = std::abs((TrkSlope * Vx.z() - Vx.x() + TrkInter) / std::hypot(TrkSlope, 1));
            if (dist > m_MaxTollDist) { return true; }
        }

        // No tracks found that are too far, so it is okay.
        return false;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    void MSVertexRecoTool::dressVtxHits(xAOD::Vertex* xAODVx, std::vector<SG::AuxElement::Decorator<int>>& decs, const std::vector<int> & hits){
            unsigned int i{0};
            for (SG::AuxElement::Decorator<int> &dec : decs) {
                dec(*xAODVx) = hits[i];
                ++i;    
            }

        return;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    StatusCode MSVertexRecoTool::FillOutputContainer(const std::vector<std::unique_ptr<MSVertex>>& vertices,
                                                     SG::WriteHandle<xAOD::VertexContainer>& xAODVxContainer,
                                                     std::vector<SG::AuxElement::Decorator<int>>& nMDT_decs,
                                                     std::vector<SG::AuxElement::Decorator<int>>& nRPC_decs,
                                                     std::vector<SG::AuxElement::Decorator<int>>& nTGC_decs){
        for (const std::unique_ptr<MSVertex> &vtx : vertices){
            xAOD::Vertex* xAODVx = new xAOD::Vertex();
            xAODVx->makePrivateStore();
            xAODVx->setVertexType(xAOD::VxType::SecVtx);
            xAODVx->setPosition(vtx->getPosition());
            xAODVx->setFitQuality(vtx->getChi2(), vtx->getNTracks() - 1);

            // link TrackParticle to vertex
            for (const xAOD::TrackParticle *trk : *(vtx->getTracks())) {
                ElementLink<xAOD::TrackParticleContainer> link_trk(*(dynamic_cast<const xAOD::TrackParticleContainer *>(trk->container())), trk->index());
                if (link_trk.isValid()) xAODVx->addTrackAtVertex(link_trk);
            }

            // store the new xAOD vertex
            xAODVxContainer->push_back(xAODVx);

            // dress the vertex with the hit counts
            dressVtxHits(xAODVx, nMDT_decs, vtx->getNMDT_all()); 
            dressVtxHits(xAODVx, nRPC_decs, vtx->getNRPC_all()); 
            dressVtxHits(xAODVx, nTGC_decs, vtx->getNTGC_all());     
        }

        return StatusCode::SUCCESS;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    // core algorithm for endcap vertex reconstruction
    Amg::Vector3D MSVertexRecoTool::VxMinQuad(const std::vector<Tracklet>& tracks) {
        double s(0.), sx(0.), sy(0.), sxy(0.), sxx(0.), d(0.);
        double sigma = 1.;
        for (const Tracklet &track : tracks) {
            double TrkSlope = std::tan(track.getML1seg().alpha());
            double TrkInter = track.getML1seg().globalPosition().perp() - track.getML1seg().globalPosition().z() * TrkSlope;
            s += 1. / std::pow(sigma,2);
            sx += TrkSlope / std::pow(sigma,2);
            sxx += std::pow(TrkSlope,2) / std::pow(sigma,2);
            sy += TrkInter / std::pow(sigma,2);
            sxy += (TrkSlope * TrkInter) / std::pow(sigma,2);
        }
        d = s * sxx - std::pow(sx,2);
        if (d == 0.) {
            Amg::Vector3D MyVx(0., 0., 0.);  // return 0, no vertex was found.
            return MyVx;
        }

        double Rpos = (sxx * sy - sx * sxy) / d;
        double Zpos = (sx * sy - s * sxy) / d;

        Amg::Vector3D MyVx(Rpos, 0, Zpos);

        return MyVx;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    // vertex phi location -- determined from the RPC/TGC hits
    double MSVertexRecoTool::vxPhiFinder(const double theta, const double phi, const EventContext& ctx) const {
        double nmeas(0), sinphi(0), cosphi(0);
        if (theta == 0) {
            ATH_MSG_WARNING("vxPhiFinder() called with theta=" << theta << " and phi=" << phi << ", return 0");
            return 0;
        } else if (theta > M_PI) {
            ATH_MSG_WARNING("vxPhiFinder() called with theta=" << std::setprecision(15) << theta << " and phi=" << phi
                                                               << ", (theta>M_PI), return 0");
            return 0;
        }
        double tanThetaHalf = std::tan(0.5 * theta);
        if (tanThetaHalf <= 0) {
            ATH_MSG_WARNING("vxPhiFinder() called with theta=" << std::setprecision(15) << theta << " and phi=" << phi
                                                               << ", resulting in tan(0.5*theta)<=0, return 0");
            return 0;
        }
        double eta = -std::log(tanThetaHalf);
        if (std::abs(eta) < 1.5) {
            SG::ReadHandle<Muon::RpcPrepDataContainer> rpcTES(m_rpcTESKey, ctx);
            if (!rpcTES.isValid()) {
                ATH_MSG_WARNING("No RpcPrepDataContainer found in SG!");
                return 0;
            }
            for (const Muon::RpcPrepDataCollection* RPC_coll : *rpcTES){
                for (const Muon::RpcPrepData* rpc : *RPC_coll){
                    if (!m_idHelperSvc->rpcIdHelper().measuresPhi(rpc->identify())) continue;
                    double rpcEta = rpc->globalPosition().eta();
                    double rpcPhi = rpc->globalPosition().phi();
                    double DR = xAOD::P4Helpers::deltaR(eta, phi, rpcEta, rpcPhi);
                    if (DR >= 0.6) continue;
                    sinphi += std::sin(rpcPhi);
                    cosphi += std::cos(rpcPhi);
                    ++nmeas;
                }
            }
        }
        if (std::abs(eta) > 0.5) {
            SG::ReadHandle<Muon::TgcPrepDataContainer> tgcTES(m_tgcTESKey, ctx);
            if (!tgcTES.isValid()) {
                ATH_MSG_WARNING("No TgcPrepDataContainer found in SG!");
                return 0;
            }
            for (const Muon::TgcPrepDataCollection* TGC_coll : *tgcTES){
                for (const Muon::TgcPrepData* tgc : *TGC_coll){
                if (!m_idHelperSvc->tgcIdHelper().isStrip(tgc->identify())) continue;
                    double tgcEta = tgc->globalPosition().eta();
                    double tgcPhi = tgc->globalPosition().phi();
                    double DR = xAOD::P4Helpers::deltaR(eta, phi, tgcEta, tgcPhi);
                    if (DR >= 0.6) continue;
                    sinphi += std::sin(tgcPhi);
                    cosphi += std::cos(tgcPhi);
                    ++nmeas;
                }
            }
        }

        double vxphi = phi;
        if (nmeas > 0) vxphi = std::atan2(sinphi / nmeas, cosphi / nmeas);
        return vxphi;
    }

    //** ----------------------------------------------------------------------------------------------------------------- **//

    void MSVertexRecoTool::HitCounter(MSVertex* MSRecoVx, const EventContext& ctx) const {
        // count the hits (MDT, RPC & TGC) around the vertex and split them by layer
        int nHighOccupancy(0);
        int stationRegion(-1);
        // Amg::Vector3D.eta() will crash via floating point exception if both x() and y() are zero (eta=inf)
        // thus, check it manually here:
        const Amg::Vector3D msVtxPos = MSRecoVx->getPosition();
        if (msVtxPos.x() == 0 && msVtxPos.y() == 0 && msVtxPos.z() != 0) {
            ATH_MSG_WARNING("given MSVertex has position x=y=0 and z!=0, eta() method will cause FPE, returning...");
            return;
        }

        // MDTs -- count the number around the vertex
        SG::ReadHandle<Muon::MdtPrepDataContainer> mdtTES(m_mdtTESKey, ctx);
        if (!mdtTES.isValid()) ATH_MSG_ERROR("Unable to retrieve the MDT hits");
        int nmdt(0), nmdt_inwards(0), nmdt_I(0), nmdt_E(0), nmdt_M(0), nmdt_O(0);
        // loop on the MDT collections, a collection corresponds to a chamber
        for(const Muon::MdtPrepDataCollection* MDT_coll : *mdtTES){
            if (MDT_coll->empty()) continue;
            Muon::MdtPrepDataCollection::const_iterator mdtItr = MDT_coll->begin();
            Amg::Vector3D ChamberCenter = (*mdtItr)->detectorElement()->center();
            double deta = msVtxPos.eta() - ChamberCenter.eta();
            if (std::abs(deta) > m_nMDTHitsEta) continue;
            double dphi = xAOD::P4Helpers::deltaPhi(msVtxPos.phi(),ChamberCenter.phi());
            if (std::abs(dphi) > m_nMDTHitsPhi) continue;
    
            Identifier id = (*mdtItr)->identify();
            stationRegion = m_idHelperSvc->mdtIdHelper().stationRegion(id);
            auto [tubeLayerMin, tubeLayerMax] = m_idHelperSvc->mdtIdHelper().tubeLayerMinMax(id);
            auto [tubeMin, tubeMax] = m_idHelperSvc->mdtIdHelper().tubeMinMax(id);
            double nTubes = (tubeLayerMax - tubeLayerMin + 1) * (tubeMax - tubeMin + 1);

            int nChHits(0), nChHits_inwards(0);
            // loop on the MDT hits in the chamber
            for (const Muon::MdtPrepData* mdt : *MDT_coll){
                if (mdt->adc() < 50) continue;
                if (mdt->status() != 1) continue;
                if (mdt->localPosition()[Trk::locR] == 0.) continue;
                ++nChHits;
                if (mdt->globalPosition().mag() < msVtxPos.mag()) ++nChHits_inwards;
            }
            nmdt += nChHits;
            nmdt_inwards += nChHits_inwards;
            double ChamberOccupancy = nChHits / nTubes;
            if (ChamberOccupancy > m_ChamberOccupancyMin) ++nHighOccupancy;

            if (stationRegion == 0) nmdt_I += nChHits;
            else if (stationRegion == 1) nmdt_E += nChHits;
            else if (stationRegion == 2) nmdt_M += nChHits;
            else if (stationRegion == 3) nmdt_O += nChHits;
        }
        ATH_MSG_DEBUG("Found " << nHighOccupancy << " chambers near the MS vertex with occupancy greater than " << m_ChamberOccupancyMin);
        if (nHighOccupancy < m_minHighOccupancyChambers) return;

        // RPC -- count the number around the vertex
        SG::ReadHandle<Muon::RpcPrepDataContainer> rpcTES(m_rpcTESKey, ctx);
        if (!rpcTES.isValid()) ATH_MSG_ERROR("Unable to retrieve the RPC hits");
        int nrpc(0), nrpc_inwards(0), nrpc_I(0), nrpc_E(0), nrpc_M(0), nrpc_O(0);
        for (const Muon::RpcPrepDataCollection* RPC_coll : *rpcTES){
            Muon::RpcPrepDataCollection::const_iterator rpcItr = RPC_coll->begin();
            stationRegion = m_idHelperSvc->rpcIdHelper().stationRegion((*rpcItr)->identify());
            int nChHits(0), nChHits_inwards(0);
            for (const Muon::RpcPrepData* rpc : *RPC_coll){
                double rpcEta = rpc->globalPosition().eta();
                double rpcPhi = rpc->globalPosition().phi();
                double DR = xAOD::P4Helpers::deltaR(msVtxPos.eta(), msVtxPos.phi(), rpcEta, rpcPhi);
                if (DR < m_nTrigHitsdR) {
                    ++nChHits;
                    if (rpc->globalPosition().mag() < msVtxPos.mag()) ++nChHits_inwards;
                }
                if (DR > 1.2) break;
            }
            nrpc += nChHits;
            nrpc_inwards += nChHits_inwards;
            if (stationRegion == 0) nrpc_I += nChHits;
            else if (stationRegion == 1) nrpc_E += nChHits;
            else if (stationRegion == 2) nrpc_M += nChHits;
            else if (stationRegion == 3) nrpc_O += nChHits;
        }

        // TGC -- count the number around the vertex
        SG::ReadHandle<Muon::TgcPrepDataContainer> tgcTES(m_tgcTESKey, ctx);
        if (!tgcTES.isValid()) ATH_MSG_ERROR("Unable to retrieve the TGC hits");
        int ntgc(0), ntgc_inwards(0), ntgc_I(0), ntgc_E(0), ntgc_M(0), ntgc_O(0);
        for (const Muon::TgcPrepDataCollection* TGC_coll : *tgcTES){
            Muon::TgcPrepDataCollection::const_iterator tgcItr = TGC_coll->begin();
            stationRegion = m_idHelperSvc->tgcIdHelper().stationRegion((*tgcItr)->identify());
            int nChHits(0), nChHits_inwards(0);
            for (const Muon::TgcPrepData* tgc : *TGC_coll){
                double tgcEta = tgc->globalPosition().eta();
                double tgcPhi = tgc->globalPosition().phi();
                double DR = xAOD::P4Helpers::deltaR(msVtxPos.eta(), msVtxPos.phi(), tgcEta, tgcPhi);
                if (DR < m_nTrigHitsdR) {
                    ++nChHits;
                    if (tgc->globalPosition().mag() < msVtxPos.mag()) ++nChHits_inwards;
                }
                if (DR > 1.2) break;
            }
            ntgc += nChHits;
            ntgc_inwards += nChHits_inwards;
            if (stationRegion == 0) ntgc_I += nChHits;
            else if (stationRegion == 1) ntgc_E += nChHits;
            else if (stationRegion == 2) ntgc_M += nChHits;
            else if (stationRegion == 3) ntgc_O += nChHits;
        }

        // store the hit counts in the MSVertex object
        MSRecoVx->setNMDT(nmdt, nmdt_inwards, nmdt_I, nmdt_E, nmdt_M, nmdt_O);
        MSRecoVx->setNRPC(nrpc, nrpc_inwards, nrpc_I, nrpc_E, nrpc_M, nrpc_O);
        MSRecoVx->setNTGC(ntgc, ntgc_inwards, ntgc_I, ntgc_E, ntgc_M, ntgc_O);
   }

}  // namespace Muon
