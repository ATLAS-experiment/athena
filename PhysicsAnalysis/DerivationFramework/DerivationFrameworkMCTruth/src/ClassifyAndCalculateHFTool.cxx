/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// ClassifyAndCalculateHFTool.cxx                                       //
// Implementation file for class ClassifyAndCalculateHFTool             //
// Author: Adrian Berrocal Guardia <adrian.berrocal.guardia@cern.ch>    //
//                                                                      //
////////////////////////////////////////////////////////////////////////// 

#include "DerivationFrameworkMCTruth/ClassifyAndCalculateHFTool.h"

namespace DerivationFramework {

  /*
  ---------------------------------------------------------------------------------------------------------------------------------------
  ------------------------------------------------------- Constructor/Destructor --------------------------------------------------------
  ---------------------------------------------------------------------------------------------------------------------------------------
  */

  ClassifyAndCalculateHFTool::ClassifyAndCalculateHFTool(const std::string& t, const std::string& n, const IInterface* p) : AthAlgTool(t,n,p){
    declareInterface<DerivationFramework::ClassifyAndCalculateHFTool>(this);
  }

  ClassifyAndCalculateHFTool::~ClassifyAndCalculateHFTool(){
  }

  /*
  ---------------------------------------------------------------------------------------------------------------------------------------
  --------------------------------------------------------- Initialize/Finalize ---------------------------------------------------------
  ---------------------------------------------------------------------------------------------------------------------------------------
  */

  StatusCode ClassifyAndCalculateHFTool::initialize() {

    ATH_MSG_INFO("Initialize");
    
    // Print the string variables.

    ATH_MSG_INFO("Cut on the pt of the jets: "           << m_jetPtCut);
    ATH_MSG_INFO("Cut on the eta of the jets: "          << m_jetEtaCut);
    ATH_MSG_INFO("Cut on the pt of the leading hadron: " << m_leadingHadronPtCut);
    ATH_MSG_INFO("Cut on the ratio between the pt of the leading hadron and the pt of its associated jet: " << m_leadingHadronPtRatioCut);

    return StatusCode::SUCCESS;
  }

  StatusCode ClassifyAndCalculateHFTool::finalize(){
    return StatusCode::SUCCESS;
  }

  /*
  ---------------------------------------------------------------------------------------------------------------------------------------
  ------------------------------------------------------------- Hadron Type -------------------------------------------------------------
  ---------------------------------------------------------------------------------------------------------------------------------------
  */

  // Define the function isBHadron that determines if an hadron is a B-type.

  bool ClassifyAndCalculateHFTool::isBHadron(int pdgId) const{

    // Check if the pdgId is too large to protect against some ions that could pass the test below.

    if(pdgId>1e9) return false;  

    // Determine if the pdgId corresponds to a B-hadron.
    // The PDG Id of quark composite states has 7-digits.
    // The 3rd and 4th digits of the PDG Id starting from the end correspond to the largest PDG Id of the quarks.

    int rest1(abs(pdgId)%1000);  // Three last digits of the PDG Id.
    int rest2(abs(pdgId)%10000); // Four last digits of the PDF Id.

    // If the 3rd digit or 4th one is 5, then the hadron has a b-quark.

    if((rest2 >= 5000 && rest2 < 6000) || (rest1 >= 500 && rest1 < 600)) return true;

    return false;

  }

  // Define the function isCHadron that determines if an hadron is a C-type.

  bool ClassifyAndCalculateHFTool::isCHadron(int pdgId) const{
    
    // Check if the pdgId is too large to protect against some ions that could pass the test below
    
    if(pdgId>1e9) return false;  

    // Determine if the pdgId corresponds to a C-hadron.
    // The PDG Id of quark composite states has 7-digits.
    // The 3rd and 4th digits of the PDG Id starting from the end correspond to the largest PDG Id of the quarks.

    int rest1(abs(pdgId)%1000);  // Three last digits of the PDG Id.
    int rest2(abs(pdgId)%10000); // Four last digits of the PDF Id.

    // If the 3rd digit or 4th one is 4, then the hadron has a c-quark.
    // The function does not consider if the case where the hadron has also a b-quark.
    // Hence, the function isCHadron should only be called if the function isBHadron returns a false.

    if((rest2 >= 4000 && rest2 < 5000) || (rest1 >= 400 && rest1 < 500)) return true;

    return false;
  }

  /*
  ---------------------------------------------------------------------------------------------------------------------------------------
  -------------------------------------------------------------- Flag Jets --------------------------------------------------------------
  ---------------------------------------------------------------------------------------------------------------------------------------
  */

  void ClassifyAndCalculateHFTool::flagJets(const xAOD::JetContainer* jets,
                                            const std::map<const xAOD::Jet*, std::vector<xAOD::TruthParticleContainer::const_iterator>>& particleMatch,
                                            const std::map<const xAOD::TruthParticle*, DerivationFramework::HadronOriginClassifier::HF_id>&  hadronMap,
                                            const std::string& hfDecorationName) const{

    for(const xAOD::Jet* jet : *jets){

      // Check if the jet passes the cuts and if it does not, then skip it.

      if(jet->p4().Pt() < m_jetPtCut) continue;
      if(fabs(jet->p4().Eta()) > m_jetEtaCut) continue;

      // Create a set of integer variables to save the necessary variables for the HF classifier:
      //  -flav:  Flavour of the jet.
      //  -id:    Origin of the most initial hadron of the jet.
      //  -count: Number of associated hadrons of the jet that have the same flavour as the jet.

      int flav=0;
      int id=0;
      int count=0;

      // Create a set of integer variables to save information that is required to compute the necessary variables for the HF classifier.:
      //  -bcount:    It contains the number of B-hadrons.
      //  -ccount:    It contains the number of C-hadrons.
      //  -bcountcut: It contains the number of B-hadron that pass the cuts.
      //  -ccountcut: It contains the number of C-hadron that pass the cuts.
      //  -bid:       The greatest value of the variable "TopHadronOriginFlag" for B-hadrons (most initial B-hadron).
      //  -cid:       The greatest value of the variable "TopHadronOriginFlag" for C-hadrons (most initial B-hadron).

      int bcount=0;
      int ccount=0;
      int bcountcut=0;
      int ccountcut=0;

      int bid=0;
      int cid=0;

      // Get the hadrons associated with the jet that is being considered.

      auto it = particleMatch.find (jet);
      if (it != particleMatch.end()) {

        for(xAOD::TruthParticleContainer::const_iterator hf : it->second){
        
          // Create two integer variables:
          //  -hforigin: It will contain the origin of the hadron if it is a HF hadron. Otherwise, it will be 6.
          //  -pdgId:    It will contain the value of the variable "pdgId" of the hadron.

          int hforigin = 6;
          int pdgId    = (*hf)->pdgId(); 

          // Extract the origin of the hadron.

          auto h_it = hadronMap.find(*hf);
          if(h_it!=hadronMap.end()){
            hforigin= static_cast<int>(h_it->second);
          }

          // Check if hforigin is 6 and if it is the case, then hadron is not HF and it is skipped.

          if(6==hforigin) continue;

          // Compute the ratio between the pt of the hadron and the pt of its associated jet.

          float ptratio = (*hf)->p4().Pt()/jet->p4().Pt();

          // Determine if the hadron is a B-hadron or a C-hadron.

          int hftype = 0;
        
          if(ClassifyAndCalculateHFTool::isCHadron(pdgId)) hftype=4; // B-hadron
          if(ClassifyAndCalculateHFTool::isBHadron(pdgId)) hftype=5; // C-hadron.
        
          // Check if hftype is 4 or 5.

          if(5==hftype){

            // In this case, hftype is 5 so the hadron is a B-hadron.
            // Save hforigin in bid if it is greater than the current bid.

            if(bid<hforigin)bid=hforigin;
          
            // Add one to bcount and to bcountcut if hadron passes the cuts.
          
            ++bcount;

            if((*hf)->p4().Pt()>m_leadingHadronPtCut && ptratio>m_leadingHadronPtRatioCut){
              ++bcountcut;
            }
          }
          else if(4==hftype){

            // In this case, hftype is 4 so the hadron is a C-hadron.
            // Save hforigin in cid if it is greater than the current cid.

            if(cid>hforigin)cid=hforigin;

            // Add one to ccount and to ccountcut if hadron passes the cuts.

            ++ccount;

            if((*hf)->p4().Pt()>m_leadingHadronPtCut && ptratio>m_leadingHadronPtRatioCut){
              ++ccountcut;
            }
          
          }
          else{
          
            // In this case, hftype is not 4 neither 5 so print an error.

            ATH_MSG_ERROR("Hadron type '" << hftype << "' is not 4 or 5");

          }
        
        }
      }

      // Check if there is at least one B-hadron or a C-hadron that passes the cuts.

      if(bcountcut){
        
        // In this case, at least one B-hadron passes the cuts.
        // The "flavour" of the jet (flav) is set to 5.
        // As a id of the jet, the greatest value of the variable "TopHadronOriginFlag" for B-hadrons is used (origin of the most initial hadron).
        // The number of B-hadrons is saved in count.
 
        flav=5;
        id=bid;
        count=bcount;
      }
      else if(ccountcut){
        
        // In this case, no B-hadron passes the cuts, but at least one C-hadron does.
        // The "flavour" of the jet (flav) is set to 4.
        // As a id of the jet, the greatest value of the variable "TopHadronOriginFlag" for C-hadrons is used (origin of the most initial hadron).
        // The number of C-hadrons is saved in count.

        flav=4;
        id=cid;
        count=ccount;
      }

      // Dectorate the jet with the flav, id and count.

      SG::AuxElement::Decorator< int > decorator_flav(hfDecorationName + "_flav"); 
      decorator_flav(*jet) = flav;

      SG::AuxElement::Decorator< int > decorator_id(hfDecorationName + "_id"); 
      decorator_id(*jet) = id;

      SG::AuxElement::Decorator< int > decorator_count(hfDecorationName + "_count"); 
      decorator_count(*jet) = count;

    }

  }

  /*
  ---------------------------------------------------------------------------------------------------------------------------------------
  ---------------------------------------------------------- HF Classification ----------------------------------------------------------
  ---------------------------------------------------------------------------------------------------------------------------------------
  */

  int ClassifyAndCalculateHFTool::computeHFClassification(const xAOD::JetContainer* jets, const std::string& hfDecorationName) const{

    // Create a set of integer variables to save information that is required to compute the HF classifier.:
    //  -b:           Number of jets that has just one B-hadron that passes the cuts.
    //  -B:           Number of jets that has more than one B-hadron and at least one of them passes the cuts.
    //  -c:           Number of jets that has just one C-hadron that passes the cuts (and no B-hadron)
    //  -C:           Number of jets that has more than one C-hadron and at least one of them passes the cuts (and no B-hadron).
    //  -b_prompt:    Number of jets that has just one prompt B-hadron that passes the cuts.      
    //  -B_prompt:    Number of jets that has more than one prompt B-hadron and at least one of them passes the cuts.    
    //  -c_prompt:    Number of jets that has just one prompt C-hadron that passes the cuts (and no B-hadron).     
    //  -C_prompt:    Number of jets that has more than one prompt C-hadron and at least one of them passes the cuts (and no B-hadron).   
    //  -mpifsr_code: HF classifier for events with no prompt additional hadrons, just Multi-Parton Interaction (MPI) and/or Final State Radiation (FSR) hadrons.
    // Note: prompt hadrons in this context are hadrons that do not come from the direct decay of top, W or Higgs but they are not from MPI or FSR.

    int b=0, B=0, c=0, C=0;
    int b_prompt=0, B_prompt=0, c_prompt=0, C_prompt=0;

    int mpifsr_code=0;

    for(const xAOD::Jet* jet : *jets){

      // Check if the jet passes the cuts and if it does not, then skip it.

      if(jet->p4().Pt() < m_jetPtCut) continue;
      if(fabs(jet->p4().Eta()) > m_jetEtaCut) continue;
      
      // Get the flavour, the id and the number of hadrons of the considered jet.

      int flav  = 0;
      int id    = 0;
      int count = 0;

      if(jet->isAvailable<int>(hfDecorationName + "_flav")){
        flav=jet->auxdata<int>(hfDecorationName + "_flav");
      }else{
        ATH_MSG_WARNING("variable '" + hfDecorationName + "_flav' not found.");
        continue;
      }

      if(jet->isAvailable<int>(hfDecorationName + "_id")){
        id=jet->auxdata<int>(hfDecorationName + "_id");
      }else{
        ATH_MSG_WARNING("variable '" + hfDecorationName + "_id' not found.");
        continue;
      }

      if(jet->isAvailable<int>(hfDecorationName + "_count")){
        count=jet->auxdata<int>(hfDecorationName + "_count");
      }else{
        ATH_MSG_WARNING("variable '" + hfDecorationName + "_count' not found.");
        continue;
      }

      // Check the flavour and the id of the jet.

      if(flav==5 && id < 3){

        // In this case, the flavour is 5 and id is less than 3.
        // This means that the jet has at least one B-hadron that is not from direct decay of top, W or Higgs.
        // Hence, the jet is an additional one.
        
        // Check the number of B-hadrons.

        if(count > 1){

          // In this case, there is more than one B-hadron, so add 1 to B.

          B++;

        }
        else{
          
          // In this case, there is just one B-hadron, so add 1 to b.

          b++;

        }
      }
      if(flav==4 && (id==0 || id==-1 || id==-2)){

        // In this case, the flavour is 4 and id is 0, -1 or -2.
        // This means that the jet has no B-hadron and at least one C-hadron that is not from direct decay of top, W or Higgs.
        // Hence, the jet is an additional one.

        // Check the number of C-hadrons.

        if(count > 1){
          
          // In this case, there is more than one C-hadron, so add 1 to C.

          C++;
        }
        else{

          // In this case, there is just one C-hadron, so add 1 to c.

          c++;
        }
      }

      // Check the flavour and the id of the jet but considering only prompt particles (id=0).
      
      if(flav==5 && id==0){

        // In this case, the flavour is 5 and id is 0.
        // This means that the jet has at least one B-hadron that is not from direct decay of top, W or Higgs neither from MPI or FSR.
        
        // Check the number of B-hadrons.

        if(count > 1){
          
          // In this case, there is more than one B-hadron, so add 1 to B_prompt.

          B_prompt++;

        }
        else{
         
          // In this case, there is jut one B-hadron, so add 1 to b_prompt.

          b_prompt++;

        }
      }
      if(flav==4 && id==0){
        
        // In this case, the flavour is 4 and id is 0.
        // This means that the jet has no B-hadron and at least one C-hadron that is not from direct decay of top, W or Higgs neither from MPI or FSR.
        
        // Check the number of C-hadrons.

        if(count > 1){

          // In this case, there is more than one C-hadron, so add 1 to C_prompt.

          C_prompt++;
        }
        else{

          // In this case, there is just one C-hadron, so add 1 to C_prompt.

          c_prompt++;

        }
      }

      // In the case when there is no prompt hadrons, then the HF classifier is computed with non-promt hadrons.
      // This hadrons come from Multi-Parton Interactions (MPI) and the Final State Radiation (FSR).
      // Compute mpifsr_code which will contain the HF classifier when there is no prompt hadrons.
      // Each jet adds a different quantity to mpifsr_code depending on its flavour and its id.

      if(id==1 && flav==5){ // b MPI
        mpifsr_code -= 1000;
      } 
      else if(id==2 && flav==5){ // b FSR
        mpifsr_code -= 100;
      } 
      else if(id==-1 && flav==4){ // c MPI
        mpifsr_code -= 10;
      } 
      else if(id==-2 && flav==4) { // c FSR
        mpifsr_code -= 1;
      }
    }

    // Compute the ext_code using the number of additional jets with HF hadrons.
    // Compute the prompt_code using the number of additional jets with propmt HF hadrons.

    int ext_code    = 1000*b+100*B+10*c+1*C;
    int prompt_code = 1000*b_prompt+100*B_prompt+10*c_prompt+1*C_prompt;

    // Check if there are prompt hadrons and non-prompt hadrons using ext_code and prompt_code.

    if(prompt_code==0 && ext_code!=0){

      // In this case, there is no prompt hadrons but there are MPI and FSR hadrons.
      // Hence, return mpifsr_code  as a HF classifier, which is equal to ext_code but in negative sign.

      return mpifsr_code;
      
    }

    // In this case, there are prompt hadrons, so return ext_code as HF classifier.

    return ext_code;

  }

  int ClassifyAndCalculateHFTool::getSimpleClassification(int hfclassif) const{

    // Check the value of the HF classifier (hfclassif) which is computed with the function computeHFClassification.

    if(abs(hfclassif)>=100){

      // If the absolute value of hfclassif is greater than 100, then there is at least one jet with a B-hadron.
      // In this case, return 1. 

      return 1;


    }
    else if(hfclassif==0){

      // If hfclassif is 0, then there is no jet with a B-hadron or a C-hadron.
      // In this case, return 0. 

      return 0;
    }

    // If the absolute value of hfclassif is lower than 100 and non-zero, then there is no jet with a B-hadron but at least one with a C-hadron.
    // In this case, return -1. 

    return -1;
  }
}
