/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
//Author: Lianyou Shan <lianyou.shan@cern.ch>

#include "TrkVertexTools/SecVertexMergingTool.h"
#include "AthContainers/Accessor.h"
#include "AthContainers/Decorator.h"
#include "TrkVertexFitterInterfaces/IVertexWeightCalculator.h"
#include "VxVertex/VxTrackAtVertex.h"
#include <cmath>
#include <vector>

namespace Trk{

   //constructor
  SecVertexMergingTool::SecVertexMergingTool ( const std::string& t, const std::string& n, const IInterface*  p )
          : AthAlgTool ( t,n,p ),
            m_Compatidime(1),
            m_minDist(3),
	    m_iVertexFitter("Trk::AdaptiveVertexFitter", this )
  {
    declareInterface<IVertexMergingTool> ( this );
    declareProperty("VertexFitterTool", m_iVertexFitter);
    declareProperty("CompatibilityDimension", m_Compatidime, "0 for z0, 1 for d0, 2 for all" ) ;
    declareProperty("MininumDistance", m_minDist, "in sigma" ) ;
  }

  //destructor
  SecVertexMergingTool::~SecVertexMergingTool()= default;

//initialize
  StatusCode SecVertexMergingTool::initialize()
  {

    if ( m_iVertexFitter.retrieve().isFailure() ) {
      ATH_MSG_ERROR("Failed to retrieve tool " << m_iVertexFitter);
      return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Re-merging tool initialization successful");
    return StatusCode::SUCCESS;
   }///EndOfInitialize

   StatusCode SecVertexMergingTool::finalize()
   {
     return StatusCode::SUCCESS;
   }

   std::pair<xAOD::VertexContainer*, xAOD::VertexAuxContainer*>
   SecVertexMergingTool::mergeVertexContainer(
     const xAOD::VertexContainer& MyVxCont) const

   {

     ATH_MSG_DEBUG("Run vertex remerging");

     // new output containers to be filled
     xAOD::VertexContainer* NewContainer = new xAOD::VertexContainer();
     xAOD::VertexAuxContainer* auxNewContainer = new xAOD::VertexAuxContainer();
     NewContainer->setStore(auxNewContainer);

     static const SG::Decorator<float> mDecor_sumPt2("sumPt2");
     static const SG::Decorator<float> mDecor_mass("mass");
     static const SG::Decorator<float> mDecor_energy("ee");
     static const SG::Decorator<int> mDecor_nrobbed("nrobbed");
     static const SG::Decorator<int> mDecor_intrk("NumInputTrk");

     static const SG::Accessor<float> mAcc_sumPt2("sumPt2");
     static const SG::Accessor<int> mAcc_momdir ("MomentaDirection");
     static const SG::Accessor<float> mAcc_mass("mass");
     static const SG::Accessor<float> mAcc_energy("ee");
     static const SG::Accessor<int> mAcc_intrk("NumInputTrk");
     static const SG::Accessor<float> mAcc_radpat("radiiPattern");
     static const SG::Accessor<std::vector<float> > mAcc_trkwt("trkWeight");
     static const SG::Accessor<int> mAcc_numtav("NumTrkAtVtx");
     static const SG::Accessor<std::vector<float> > mAcc_trkdoe("trkDistOverError");

     bool moreDeco = mAcc_momdir.isAvailable(*MyVxCont.front());

     if (!moreDeco)
       ATH_MSG_DEBUG("Missing decoration !!! ");

     /**
         SG::Decorator<std::vector<float> > mDecor_trkWght(
     "trkWeight" ) ; SG::Decorator<float> mDecor_trkDOE(
     "trkDistOverError" ) ; SG::Decorator<float> mDecor_direction(
     "MomentaDirection" ); SG::Decorator< float > mDecor_HitsFilter(
     "radiiPattern" );
     **/

     // add remerged flags to all
     std::vector<bool> remerged(MyVxCont.size(), false);

     xAOD::VertexContainer::const_iterator beginIter = MyVxCont.begin();
     xAOD::VertexContainer::const_iterator endIter = MyVxCont.end();
     unsigned int Ni = 0;
     for (xAOD::VertexContainer::const_iterator i = beginIter; i != endIter;
          ++i, Ni++) {
       xAOD::Vertex* vx = new xAOD::Vertex(**i);

       if (remerged[Ni])
         continue; // skip vertices already merged into another

       std::vector<const xAOD::TrackParticle*> combinedTracks;
       std::vector<ElementLink<xAOD::TrackParticleContainer>> tpLinks1 =
         vx->trackParticleLinks();
       if (!tpLinks1.empty()) {
         for (const auto& tp_EL : tpLinks1) {
           const xAOD::TrackParticle* trk = *tp_EL;
           combinedTracks.push_back(trk);
         }

         unsigned int Nj = Ni + 1;
         bool newmerge = false;
         for (xAOD::VertexContainer::const_iterator j = i + 1; j != endIter;
              ++j, Nj++) {
           const xAOD::Vertex* mergeCand = (*j);
           if (remerged[Nj])
             continue;

           if (newmerge) {
             combinedTracks.clear();
             tpLinks1 = vx->trackParticleLinks();
             if (tpLinks1.empty())
               break;
             for (const auto& tp_EL : tpLinks1) {
               const xAOD::TrackParticle* trk = *tp_EL;
               combinedTracks.push_back(trk);
             }
             newmerge = false;
           }

           // not dummy and not already merged into earlier vertex, so consider
           // it as merging candidate
           if (!checkCompatibility(vx, mergeCand))
             continue;

           ATH_MSG_DEBUG("To merge vertices " << Ni << " and " << Nj);
           // get all the track particles to fit

           const std::vector<ElementLink<xAOD::TrackParticleContainer>>
             tpLinks2 = mergeCand->trackParticleLinks();
           if (tpLinks2.empty())
             continue;

           for (const auto& tp_EL : tpLinks2) {
             const xAOD::TrackParticle* trk = *tp_EL;
             combinedTracks.push_back(trk);
           }

           ATH_MSG_DEBUG("Tracks input : " << tpLinks1.size() << " + "
                                           << tpLinks2.size());

           // call the fitter -> using xAOD::TrackParticle it should set the
           // track links for us
           xAOD::Vertex* mergedVtx = nullptr;
           // no interface for no constraint and no starting point, so use
           // starting point of original vertex
           Amg::Vector3D start(0.5 * (vx->position() + mergeCand->position()));
           mergedVtx = m_iVertexFitter->fit(combinedTracks, start);

           ATH_MSG_DEBUG("Merged vertices " << mergedVtx->nTrackParticles());

           remerged[Nj] = true;
           remerged[Ni] = true;
           newmerge = true;

           // update the decors
           float pt1 = sqrt(mAcc_sumPt2(*vx));
           float pt2 = sqrt(mAcc_sumPt2(*mergeCand));
           float ntrk1 = 1.0 * ((vx->trackParticleLinks()).size());
           float ntrk2 = 1.0 * ((mergeCand->trackParticleLinks()).size());
           float wght1 =
             0.6 * pt1 / (pt1 + pt2) + 0.4 * ntrk1 / (ntrk1 + ntrk2);
           float wght2 =
             0.6 * pt2 / (pt1 + pt2) + 0.4 * ntrk2 / (ntrk1 + ntrk2);

           xAOD::VxType::VertexType typ1 = vx->vertexType();
           xAOD::VxType::VertexType typ2 = mergeCand->vertexType();
           float mas1 = mAcc_mass(*vx);
           float mas2 = mAcc_mass(*mergeCand);
           float e1 = mAcc_energy(*vx);
           float e2 = mAcc_energy(*mergeCand);
           int inNtrk1 = mAcc_intrk(*vx);
           int inNtrk2 = mAcc_intrk(*mergeCand);

           int ntrks = 0;
           float md1 = 0., md2 = 0., hf1 = 0., hf2 = 0.;
           std::vector<float> trkW1, trkW2, doe1, doe2;
           if (moreDeco) {
             doe1 = mAcc_trkdoe(*vx);
             doe2 = mAcc_trkdoe(*mergeCand);
             doe2.insert(doe2.end(), doe1.begin(), doe1.end());
             md1 = mAcc_momdir(*vx);
             md2 = mAcc_momdir(*mergeCand);
             hf1 = mAcc_radpat(*vx);
             hf2 = mAcc_radpat(*mergeCand);
             trkW1 = mAcc_trkwt(*vx);
             trkW2 = mAcc_trkwt(*mergeCand);
             trkW2.insert(trkW2.end(), trkW1.begin(), trkW1.end());
             ntrks = mAcc_numtav(*vx) + mAcc_numtav(*mergeCand);
           }

           // delete copy of first vertex and then overwrite with merged vertex
           delete vx;
           vx = mergedVtx;

           if (wght1 >= wght2)
             vx->setVertexType(typ1);
           else
             vx->setVertexType(typ2);

           if (moreDeco) {
             mAcc_trkdoe(*vx) = doe2;
             mAcc_momdir(*vx) = wght1 * md1 + wght2 * md2;
             mAcc_radpat(*vx) = wght1 * hf1 + wght2 * hf2;
             mAcc_trkwt(*vx) = trkW2;
             mAcc_numtav(*vx) = ntrks;
           }

           mDecor_sumPt2(*vx) = pt1 * pt1 + pt2 * pt2;
           mDecor_mass(*vx) = wght1 * mas1 + wght2 * mas2;
           mDecor_energy(*vx) = wght1 * e1 + wght2 * e2;
           mDecor_nrobbed(*vx) = 0;
           mDecor_intrk(*vx) = (int)(wght1 * inNtrk1 + wght1 * inNtrk2);

         } // loop over j
       }   // if vx found partner in compatibility

       // whether we merged or not, can add vx to the container
       if (vx != nullptr){
	 ATH_MSG_DEBUG("Merged sumPt2 " << mAcc_sumPt2(*vx));
         NewContainer->push_back(vx);
       }
     }

     return std::make_pair(NewContainer, auxNewContainer);

  }

  bool
  SecVertexMergingTool::checkCompatibility(const xAOD::Vertex* v1,
                                           const xAOD::Vertex* v2) const
  {

    float sigma = 100 ;

    Amg::Vector3D vdif = v1->position() - v2->position() ;
    AmgSymMatrix(3) vErrs = v1->covariancePosition() + v2->covariancePosition() ;
    vErrs = vErrs.inverse().eval();

    if ( m_Compatidime == 2 )  //  3 dimension
    {
      sigma = sqrt( vdif.dot( vErrs * vdif ) ) ;
    } else if (  m_Compatidime == 1 )  // d0
    {
      sigma = vdif(0)*vdif(0)*vErrs(0,0) + vdif(1)*vdif(1)*vErrs(1,1) + 2*vdif(0)*vdif(1)*vErrs(0,1) ;
      sigma = std::sqrt( sigma ) ;

    } else {  // z0

      sigma = vdif(2)*sqrt( vErrs(2,2) );
    }

//    ATH_MSG_DEBUG(" Compatibility/significance when merging vertices : " << sigma );
    ATH_MSG_DEBUG(" Compatibility/significance when merging vertices : " << sigma );

    return sigma < m_minDist;

  }

}///End trk namespace
