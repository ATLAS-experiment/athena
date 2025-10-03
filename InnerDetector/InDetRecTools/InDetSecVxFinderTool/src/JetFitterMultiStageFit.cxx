/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#include "InDetSecVxFinderTool/JetFitterMultiStageFit.h"

#include "VxJetVertex/VxJetCandidate.h"
#include "VxJetVertex/VxVertexOnJetAxis.h"
#include "VxJetVertex/VxClusteringTable.h"
#include "VxJetVertex/PairOfVxVertexOnJetAxis.h"
#include "VxVertex/VxTrackAtVertex.h"


using namespace InDet;

/*
 * For the Hackathon, an example of a new algorithm tool
 */


JetFitterMultiStageFit::JetFitterMultiStageFit(const std::string &t, const std::string &n, const IInterface *p)
        : AthAlgTool(t, n, p)
{
    // constructor, declare all necessary things
    declareInterface< JetFitterMultiStageFit >(this);
}


JetFitterMultiStageFit::~JetFitterMultiStageFit() = default;

StatusCode JetFitterMultiStageFit::initialize() {

    ATH_CHECK( m_helper.retrieve() );
    ATH_CHECK( m_initializationHelper.retrieve() );
    ATH_CHECK( m_routines.retrieve() );
    ATH_CHECK( m_jetFitterUtils.retrieve() );

    return StatusCode::SUCCESS;
}


Trk::VxJetCandidate* JetFitterMultiStageFit::doTwoStageFit(const Trk::RecVertex & primaryVertex,
                                                        const TLorentzVector & jetMomentum,
                                                        const std::vector<const Trk::ITrackLink*> & firstInputTracks,
                                                        const std::vector<const Trk::ITrackLink*> & secondInputTracks,
                                                        const Amg::Vector3D & vtxSeedDirection) const {

    Amg::Vector3D myDirection(jetMomentum.X(),jetMomentum.Y(),jetMomentum.Z());

    std::vector<std::vector<const Trk::ITrackLink*> > bunchesOfTracks; // vector of vector of tracks

    std::vector<const Trk::ITrackLink*> tracksToAdd;

    std::vector<const Trk::ITrackLink*>::const_iterator tracks2Begin=firstInputTracks.begin();
    std::vector<const Trk::ITrackLink*>::const_iterator tracks2End=firstInputTracks.end();
    for (std::vector<const Trk::ITrackLink*>::const_iterator tracks2Iter=tracks2Begin;
         tracks2Iter!=tracks2End;++tracks2Iter) {
        tracksToAdd.push_back(*tracks2Iter);
    }

    bunchesOfTracks.push_back(tracksToAdd);
    tracksToAdd.clear();

    std::vector<const Trk::ITrackLink*>::const_iterator tracks3Begin=secondInputTracks.begin();
    std::vector<const Trk::ITrackLink*>::const_iterator tracks3End=secondInputTracks.end();
    for (std::vector<const Trk::ITrackLink*>::const_iterator tracks3Iter=tracks3Begin;
         tracks3Iter!=tracks3End;++tracks3Iter) {
        tracksToAdd.push_back(*tracks3Iter);
    }

    if (!tracksToAdd.empty())
    {
        bunchesOfTracks.push_back(tracksToAdd);
    }
    tracksToAdd.clear();

    //now it just uses these bunches...
    //now I have just to make sure that no clustering is done at first iteration
    //while it needs to be done at second iteration (there will be only two iterations)


    std::vector<std::vector<const Trk::ITrackLink*> >::const_iterator BunchesBegin=bunchesOfTracks.begin();
    std::vector<std::vector<const Trk::ITrackLink*> >::const_iterator BunchesEnd=bunchesOfTracks.end();

    std::vector<const Trk::ITrackLink*>::const_iterator tracksToAddBegin;
    std::vector<const Trk::ITrackLink*>::const_iterator tracksToAddEnd;
    std::vector<const Trk::ITrackLink*>::const_iterator tracksToAddIter;


    Trk::VxJetCandidate* myJetCandidate=nullptr;

    for (std::vector<std::vector<const Trk::ITrackLink*> >::const_iterator BunchesIter=BunchesBegin;
         BunchesIter!=BunchesEnd;++BunchesIter) {

        if (BunchesIter == BunchesBegin) { // this simply means we are only using tracksToUseInFirstFit
            ATH_MSG_VERBOSE(" initial fit with  " << (*BunchesIter).size() << " tracks ");
            myJetCandidate = m_initializationHelper->initializeJetCandidate(*BunchesIter, &primaryVertex, &myDirection,
                                                                            &vtxSeedDirection);
            m_routines->initializeToMinDistancesToJetAxis(myJetCandidate);
            if (!(*BunchesIter).empty()) {
                doTheFit(myJetCandidate, true);
                // Im confused, didnt the comment above say no clustering done in first iteration?
                // yet performClustering = true in the call above??
            }
        }

        // second stage using all tracks, why is it done this way with an if-else in a for loop...
        else {
            ATH_MSG_VERBOSE(" other fit with " << (*BunchesIter).size() << " tracks ");
            std::vector<Trk::VxVertexOnJetAxis*> setOfVertices=myJetCandidate->getVerticesOnJetAxis();
            std::vector<Trk::VxTrackAtVertex*>* setOfTracks=myJetCandidate->vxTrackAtVertex();
            tracksToAddBegin=(*BunchesIter).begin();
            tracksToAddEnd=(*BunchesIter).end();

            for (tracksToAddIter=tracksToAddBegin;tracksToAddIter!=tracksToAddEnd;++tracksToAddIter) {
                std::vector<Trk::VxTrackAtVertex*> temp_vector_tracksAtVertex;
                Trk::VxTrackAtVertex* newVxTrack=new Trk::VxTrackAtVertex((*tracksToAddIter)->clone());
                temp_vector_tracksAtVertex.push_back(newVxTrack);
                setOfTracks->push_back(newVxTrack);
                //add the new tracks to the candidate's track collection
                setOfVertices.push_back(new Trk::VxVertexOnJetAxis(temp_vector_tracksAtVertex));
                //add new vertex with all the *BunchesIter tracks attached to it
            }

            ATH_MSG_VERBOSE(" new overall number of tracks (vertices?) to fit : " << setOfVertices.size());
            myJetCandidate->setVerticesOnJetAxis(setOfVertices);
            Trk::JetFitterInitializationHelper::updateTrackNumbering(myJetCandidate);
            //question: should this be done???
            m_routines->initializeToMinDistancesToJetAxis(myJetCandidate);
            // we re-initialize, is this wise? We've merged vertices in the previous iteration...
            doTheFit(myJetCandidate);
        }
    }



    ATH_MSG_DEBUG(" returning jet candidate");
    return myJetCandidate;
}


void JetFitterMultiStageFit::doTheFit(Trk::VxJetCandidate* myJetCandidate,
              bool performClustering) const {

    int numClusteringLoops=0;
    bool noMoreVerticesToCluster(false);

    do {//regards clustering

        int numLoops = 0;
        bool noMoreTracksToDelete(false);
        do {//regards eliminating incompatible tracks...

            m_routines->performTheFit(myJetCandidate, 15, false, 30, 0.001);

            const std::vector<Trk::VxVertexOnJetAxis *> &vertices = myJetCandidate->getVerticesOnJetAxis();

            std::vector<Trk::VxVertexOnJetAxis *>::const_iterator verticesBegin = vertices.begin();
            std::vector<Trk::VxVertexOnJetAxis *>::const_iterator verticesEnd = vertices.end();


            //delete incompatible tracks...
            float max_prob(1.);
            Trk::VxVertexOnJetAxis *worseVertex(nullptr);
            for (std::vector<Trk::VxVertexOnJetAxis *>::const_iterator verticesIter = verticesBegin;
                 verticesIter != verticesEnd; ++verticesIter) {
                if (*verticesIter == nullptr) {
                    ATH_MSG_WARNING("One vertex is empy. Problem when trying to delete incompatible vertices. No further vertices deleted.");
                } else {
                    const Trk::FitQuality &fitQuality = (*verticesIter)->fitQuality();
                    if (TMath::Prob(fitQuality.chiSquared(), (int) std::floor(fitQuality.numberDoF() + 0.5)) <
                        max_prob) {
                        max_prob = TMath::Prob(fitQuality.chiSquared(), (int) std::floor(fitQuality.numberDoF() + 0.5));
                        worseVertex = *verticesIter;
                    }
                }
            }
            if (max_prob < m_vertexProbCut) {
                    ATH_MSG_DEBUG("Deleted vertex " << worseVertex->getNumVertex() << " with probability " << max_prob);
                //	  std::cout << "Deleted vertex " << worseVertex->getNumVertex() << " with probability " << max_prob << std::endl;
                if (worseVertex == myJetCandidate->getPrimaryVertex()) {
                    ATH_MSG_VERBOSE(" It's the primary");
                }

                m_routines->deleteVertexFromJetCandidate(worseVertex, myJetCandidate);

            }
            else {
                noMoreTracksToDelete = true;
                ATH_MSG_VERBOSE("No tracks to delete: maximum probability is " << max_prob);
            }

            numLoops += 1;
        } while (numLoops < m_maxNumDeleteIterations && !(noMoreTracksToDelete));

        if (!performClustering) break;

        // m_useFastClustering is false, so possibly can be removed ??
        if (!m_useFastClustering && (int)myJetCandidate->getVerticesOnJetAxis().size()<m_maxTracksForDetailedClustering) {
            m_routines->fillTableWithFullProbOfMerging(myJetCandidate,8,false,10,0.01);
        }

        else {
            m_routines->fillTableWithFastProbOfMerging(myJetCandidate);
        }
        const Trk::VxClusteringTable* clusteringTablePtr(myJetCandidate->getClusteringTable());


        if (clusteringTablePtr==nullptr) {
            ATH_MSG_WARNING(" No Clustering Table while it should have been calculated... no more clustering performed during vertexing ");
            noMoreVerticesToCluster=true;
        }
        // why else here? why not just continue in if above?
        else {
            ATH_MSG_VERBOSE(" clustering table is " << *clusteringTablePtr); //suppress this?

            //now iterate over the full map and decide wether you want to do the clustering OR not...
            float probVertex(0.);

            // probVertex is passed by reference, below function modifies it...
            Trk::PairOfVxVertexOnJetAxis pairOfVxVertexOnJetAxis=clusteringTablePtr->getMostCompatibleVertices(probVertex);
            //a PairOfVxVertexOnJetAxis is a std::pair<VxVertexOnJetAxis*,VxVertexOnJetAxis*>

            float probVertexExcludingPrimary(0.);
            Trk::PairOfVxVertexOnJetAxis pairOfVxVertexOnJetAxisExcludingPrimary=clusteringTablePtr->getMostCompatibleVerticesExcludingPrimary(probVertexExcludingPrimary);

            // essentially below ask if the two vals are the same, to a tolerance of 1e-6 ...
            bool firstProbIsWithPrimary= ( fabs(probVertex-probVertexExcludingPrimary)>1e-6 );
            // is there a better way of establishing this?

            // merging vertex with primary (if cond satisfied
            if (probVertex>0.&&probVertex>m_vertexClusteringProbabilityCut&&firstProbIsWithPrimary) {
                ATH_MSG_VERBOSE(" merging vtx number " << (*pairOfVxVertexOnJetAxis.first).getNumVertex() <<
                                                " and " << (*pairOfVxVertexOnJetAxis.second).getNumVertex() << " (should be PV).");  // what do these numVertex mean? e.g. -10, 0, 1, ...?

                m_helper->mergeVerticesInJetCandidate(*pairOfVxVertexOnJetAxis.first,
                                                      *pairOfVxVertexOnJetAxis.second,
                                                      *myJetCandidate);

                //now you need to update the numbering scheme
                Trk::JetFitterInitializationHelper::updateTrackNumbering(myJetCandidate);//maybe this should be moved to a lower level...
                continue;
            }

            // determine whether to merge vertices that are compatible, where neither are the primary
            if (probVertexExcludingPrimary>0.) {

                //GP suggested by Marco Battaglia, use vertex mass in order to decide wether to split or not, so derive vertex masses first
                const Trk::VxVertexOnJetAxis *firstVertex = pairOfVxVertexOnJetAxisExcludingPrimary.first;
                const Trk::VxVertexOnJetAxis *secondVertex = pairOfVxVertexOnJetAxisExcludingPrimary.second;

                CLHEP::HepLorentzVector massVector1 = InDet::InDetJetFitterUtils::fourMomentumAtVertex(*firstVertex);//MeV
                CLHEP::HepLorentzVector massVector2 = InDet::InDetJetFitterUtils::fourMomentumAtVertex(*secondVertex);//MeV

                CLHEP::HepLorentzVector sumMassVector = massVector1 + massVector2;

                double massTwoVertex = sumMassVector.mag();//MeV

                bool doMerge(false);

		int bin = getIndexByMass(massTwoVertex);
                double vertexClusteringProbabilityCutWithMass =
		  m_vertexClusteringProbabilityCutWithMasses[bin];

		ATH_MSG_DEBUG("m="<<massTwoVertex<<" prob="<<vertexClusteringProbabilityCutWithMass);

                if (probVertexExcludingPrimary > vertexClusteringProbabilityCutWithMass) {
                    doMerge = true; // why not just have doMerge = probVertexExcludingPrimary > vertexClusteringProbabilityCutWithMass ?
                }

                if (doMerge)
                {

                    ATH_MSG_VERBOSE(" merging vtx number " << (*pairOfVxVertexOnJetAxis.first).getNumVertex() <<
                                                    " and " << (*pairOfVxVertexOnJetAxis.second).getNumVertex() << " mass merged vertex: " << massTwoVertex);

                    m_helper->mergeVerticesInJetCandidate(*pairOfVxVertexOnJetAxisExcludingPrimary.first,
                                                          *pairOfVxVertexOnJetAxisExcludingPrimary.second,
                                                          *myJetCandidate);

                    Trk::JetFitterInitializationHelper::updateTrackNumbering(myJetCandidate);//maybe this should be moved to a lower level...
                    continue;//go to next cycle, after a succesful merging
                }
            }

            noMoreVerticesToCluster=true;

        }

        numClusteringLoops+=1; // when will this ever be greater than 1?
        // noMoreVerticesToCluster has been set to true by the time this line is reached
        // so numClusteringLoops will equal 1 and thats the end of the while loop (see below)...

    } while (numClusteringLoops<m_maxClusteringIterations&&!(noMoreVerticesToCluster));



}



int JetFitterMultiStageFit::getIndexByMass(const double mass) const{
  // Set mass to min/max value in case of under/overflow
  double m = std::clamp(mass, m_massBins.front(), m_massBins.back());
  const auto pVal = std::lower_bound(m_massBins.begin(), m_massBins.end(), m);
  const int bin = std::distance(m_massBins.begin(), pVal);
  ATH_MSG_DEBUG("Checking (m/bin) = (" << m << "," << bin << ")");
  return bin;
}
