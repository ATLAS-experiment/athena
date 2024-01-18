/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
//
// ClassifiedTrackTaggerTool.h - Description
//
/*
   Tool to give b-tagging score based on the track information
   provided by the Track Classification Tool (TCT, InDetTrkInJetType)

   The three tracks with the highest probability of originating from a
   B-hadron decay are used for the classification through a binary BDT, 
   which uses the three weights of the TCT for each of the three tracks
   and the track multiplicity information through ptjet / ntrk

    Author: Katharina Voss
    e-mail: katharina.voss@cern.ch
*/
#ifndef Analysis_ClassifiedTrackTaggerTool_H
#define Analysis_ClassifiedTrackTaggerTool_H

#include <vector>
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODJet/JetContainer.h" 
//Interface of InDetTrkInJetType (inside .h file of InDetTrkInJetType class)
#include "InDetVKalVxInJetTool/InDetTrkInJetType.h"

#include "StoreGate/WriteDecorHandle.h"
//

class TLorentzVector;
class IChronoStatSvc;
namespace MVAUtils { class BDT; }
namespace InDet {  class IInDetTrkInJetType; }

namespace Analysis {

//------------------------------------------------------------------------
  class IClassifiedTrackTaggerTool : virtual public IAlgTool {
    public:
      DeclareInterfaceID( IClassifiedTrackTaggerTool, 1, 0 );
//---------------------------------------------------------------------------
//Interface

      virtual float bJetWgts( const std::vector<const xAOD::TrackParticle*> & , const xAOD::Vertex &, const TLorentzVector &) const =0;
      virtual void decorateJets(const std::vector<const xAOD::TrackParticle*> & , const xAOD::Vertex &, const xAOD::JetContainer & ) const =0;

  };




  class ClassifiedTrackTaggerTool : public extends<AthAlgTool, IClassifiedTrackTaggerTool>
  {
   public:
       /* Constructor */
      ClassifiedTrackTaggerTool(const std::string& type, const std::string& name, const IInterface* parent);
       /* Destructor */
      virtual ~ClassifiedTrackTaggerTool();


      virtual StatusCode initialize() override;
      virtual StatusCode finalize() override;

      /** Method to retrieve the classifier score of the ClassifiedTrackTagger (CTT) */
      virtual float bJetWgts( const std::vector<const xAOD::TrackParticle*> & , const xAOD::Vertex &, const TLorentzVector &) const override;
      /** Method to decorate the xAOD::Jet object with the CTT score */
      virtual void decorateJets(const std::vector<const xAOD::TrackParticle*> & , const xAOD::Vertex &, const xAOD::JetContainer & ) const override;

//------------------------------------------------------------------------------------------------------------------
// Private data and functions
//

   private:
    //debugging
    IChronoStatSvc* m_timingProfile{}; 
    //variable to check: before public method bJetWgts can be executed, check that initialize() has been run!
    int m_initialised{};

    //variables for retrieving the CTT classification score given the TCT weights
    std::unique_ptr<MVAUtils::BDT> m_CTTBDT;
    
    //TCT Tool to retrieve TCT weights, which are input to CTT BDT
    ToolHandle < InDet::IInDetTrkInJetType >       m_trackClassificator;

    //delta R cone size determining the considered tracks around the jet axis
    float m_deltaRConeSize;

    //use updated TCT tool and CTT model trained with TCT weights from updated TCT tool
    bool m_useFivePtJetBinTCT;

    //name of the BDT calibration file
    std::string m_calibFileName;

    //name of the jet collection
    std::string m_jetCollection;

    /** Private method for sorting tracks according to the highest wgtB */
    std::vector<int> GetSortedIndices(std::vector<std::vector<float>> unordered_vec) const;

    /** The write key for adding CTT score to the jets */
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_jetWriteDecorKey{this,"JetDecorKey","","WriteDecorHandleKey for adding CTT score to Jets"};

 };




}  //end namespace
#endif
