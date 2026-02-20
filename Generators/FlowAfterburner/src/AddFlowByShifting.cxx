/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// File:  Generators/FlowAfterburner/AddFlowByShifting.cxx
// Description:
//    This code is used to introduce particle flow
//    to particles from generated events
//
// AuthorList:
// Andrzej Olszewski: Initial Code February 2006
// 11.10.2006: Add predefined flow function by name

#include "FlowAfterburner/AddFlowByShifting.h"

#include <set>
#include <cmath>

// For the Athena-based random numbers
#include "AthenaKernel/RNGWrapper.h"
#include "CLHEP/Random/RandomEngine.h"
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandGaussQ.h"
#include "CLHEP/Vector/LorentzVector.h"
//
#include "GeneratorObjects/HijingEventParams.h"
#include "GeneratorObjects/McEventCollection.h"
#include "AtlasHepMC/Relatives.h" //descendant_vertices
// gnus scientific library
#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_roots.h>

#include "GaudiKernel/PhysicalConstants.h"

#include "CxxUtils/checker_macros.h"

#include "TGraph.h"

double AddFlowByShifting::vn_func(double x, void *params)
{
  float *par_float = (float*) params;
  double phi_0  = par_float[0];
  float *vn    = par_float+1;
  float *psi_n = vn+Harmonic::NumHar;
  double val=x   +2*(   vn[Harmonic::v1]*sin(1*(x-psi_n[Harmonic::v1]))/1.0 +
                        vn[Harmonic::v2]*sin(2*(x-psi_n[Harmonic::v2]))/2.0 +
                        vn[Harmonic::v3]*sin(3*(x-psi_n[Harmonic::v3]))/3.0 +
                        vn[Harmonic::v4]*sin(4*(x-psi_n[Harmonic::v4]))/4.0 +
                        vn[Harmonic::v5]*sin(5*(x-psi_n[Harmonic::v5]))/5.0 +
                        vn[Harmonic::v6]*sin(6*(x-psi_n[Harmonic::v6]))/6.0 );
  return val-phi_0;
}


AddFlowByShifting::AddFlowByShifting(const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator)
{
  m_flow_function= NULL;
  for(int ihar = 0; ihar< Harmonic::NumHar; ihar++){
    m_psi_n[ihar] =0.0;
    m_v_n  [ihar] =0.0;
    m_EbE_Multiplier_vn[ihar]=1.0;
  }
}


StatusCode AddFlowByShifting::initialize(){
  ATH_MSG_INFO(">>> AddFlowByShifting from Initialize <<<");

  ATH_CHECK(m_rndmSvc.retrieve());

  ATH_MSG_INFO("**********Settings for Afterburner************");
  ATH_MSG_INFO("McTruthKey         : " << m_inkey              );
  ATH_MSG_INFO("McFlowKey          : " << m_outkey             );

  ATH_MSG_INFO("FlowFunctionName   : " << m_flow_function_name );
  ATH_MSG_INFO("FlowInplementation : " << m_flow_implementation);
  ATH_MSG_INFO("FlowFluctuations   : " << m_flow_fluctuations  );

  ATH_MSG_INFO("RandomizePhi       : " << m_ranphi_sw          );

  ATH_MSG_INFO("FlowEtaSwitch      : " << m_floweta_sw         );
  ATH_MSG_INFO("FlowMinEtaCut      : " << m_flow_mineta        );
  ATH_MSG_INFO("FlowMaxEtaCut      : " << m_flow_maxeta        );

  ATH_MSG_INFO("FlowPtSwitch       : " << m_flowpt_sw          );
  ATH_MSG_INFO("FlowMinPtCut       : " << m_flow_minpt         );
  ATH_MSG_INFO("FlowMaxPtCut       : " << m_flow_maxpt         );

  ATH_MSG_INFO("FlowV1             : " << m_custom_v1          );
  ATH_MSG_INFO("FlowV2             : " << m_custom_v2          );
  ATH_MSG_INFO("FlowV3             : " << m_custom_v3          );
  ATH_MSG_INFO("FlowV4             : " << m_custom_v4          );
  ATH_MSG_INFO("FlowV5             : " << m_custom_v5          );
  ATH_MSG_INFO("FlowV6             : " << m_custom_v6          );
  ATH_MSG_INFO("FlowBSwitch        : " << m_flowb_sw           );
  ATH_MSG_INFO("********************************r*************");


  // Select the flow-implementing function based of the function-choice variable
  m_flow_function=&AddFlowByShifting::jjia_minbias_new;
  if     (m_flow_function_name=="jjia_minbias_new"       ) m_flow_function=&AddFlowByShifting::jjia_minbias_new;
  else if(m_flow_function_name=="jjia_minbias_new_v2only") m_flow_function=&AddFlowByShifting::jjia_minbias_new_v2only;
  else if(m_flow_function_name=="fixed_vn"               ) m_flow_function=&AddFlowByShifting::fixed_vn;
  else if(m_flow_function_name=="fixed_v2"               ) m_flow_function=&AddFlowByShifting::fixed_v2;
  else if(m_flow_function_name=="jjia_minbias_old"       ) m_flow_function=&AddFlowByShifting::jjia_minbias_old;
  else if(m_flow_function_name=="ao_test"                ) m_flow_function=&AddFlowByShifting::ao_test;
  else if(m_flow_function_name=="custom"                 ) m_flow_function=&AddFlowByShifting::custom_vn;
  else if(m_flow_function_name=="p_Pb_cent_eta_indep"    ) m_flow_function=&AddFlowByShifting::p_Pb_cent_eta_indep;
  else if(m_flow_function_name=="OO_eta_indep"           ) m_flow_function=&AddFlowByShifting::OO_eta_indep;
  else{
    ATH_MSG_ERROR("Unimplemented option for setting 'FlowFunctionName' : " << m_flow_function_name);
    return StatusCode::FAILURE;
  }


  m_flow_implementation_type=1;
  if     (m_flow_implementation=="approximate"){
    m_flow_implementation_type=0;
    ATH_MSG_WARNING("'FlowInplementation=\"approximate\"' is obsolete, please switch to 'FlowInplementation=\"exact\"' ");
  }
  else if(m_flow_implementation=="exact"      ){
    m_flow_implementation_type=1;
  }
  else{
    ATH_MSG_ERROR("Unimplemented option for  setting 'FlowInplementation' : " << m_flow_implementation);
    return StatusCode::FAILURE;
  }



  //TGraph storing the v2_RP/delta Vs b_imp values to be used in implementing the EbyE fluctuations
  //the values below are b_imp-low,b_imp-high, delta/v2_RP for different centralities
  //underflow and overflow bins are added for smooth extrapolation
  if(m_flow_fluctuations){
    //Fluctuations for Pb+Pb
    if(m_flow_function_name=="jjia_minbias_new"        ||
       m_flow_function_name=="jjia_minbias_new_v2only")
    {
      //The delta/v2_RP values are taken from Fig15 of EbE vn paper (arXiv:1305.2942)
      //                 <0  ,  0-1 ,  1-2 , 2-3  ,  3-4 ,  4-5 , 5-10 , 10-15, 15-20, 20-25, 25-30,
      float b_lo[21]={  -1.00, 0.000, 1.483, 2.098, 2.569, 2.966, 3.317, 4.687, 5.739, 6.627, 7.409,
                        8.117, 8.767, 9.373, 9.943,10.479,10.991,11.479,11.947,15.00 ,100.0};
      //                30-35, 35-40, 40-45, 45-50, 50-55, 55-60, 60-65, 65-70, 70-
      float b_hi[21]={ -1.00 , 1.483, 2.098, 2.569, 2.966, 3.317, 4.687, 5.739, 6.627, 7.409, 8.117,//bimp_high
                        8.767, 9.373, 9.943,10.479,10.991,11.479,11.947,12.399,15.00 ,100.0};
      float val [21]={  5.600, 5.600, 5.600,1.175 ,0.8253,0.7209,0.5324,0.4431,0.3984,0.3844,0.3847,
                       0.3935,0.4106,0.4310,0.4574,0.4674,0.4873,0.4796,0.4856,0.5130,0.5130};
      float bimp_vals[21];

      for(int i=0;i<21;i++){
        bimp_vals[i]=(b_lo[i]+b_hi[i])/2.0f;
        val      [i]=1.0f/val[i];//change to v2_RP/delta
        val      [i]=1.0/val[i];//change to v2_RP/delta
      }

      m_graph_fluc=new TGraph(21,bimp_vals,val);
    }

    //Fluctuations for O+O
    if(m_flow_function_name=="OO_eta_indep")
    {
      //The v2{2} and v2{4} values are taken from the OO paper (arXiv:2509.05171)
      //The v2{4} values beyond the 50-60% bin are put in by hand
      //The v2{2} values beyond the 75-80% bin are put in by hand
      //The impact parameters are obtained from HIJING

      //4-particle cumulant v2 (v2{4})
      //                       <0,       0-1,       1-2,       2-3,       3-4,       4-5,      5-10,     10-15,     15-20,     20-25,     25-30,
      //                    30-35,     35-40,     40-50,     50-60,     60-80,    80-100,      >100,
      float v2_4 [18]={ 0.0444322, 0.0444322, 0.0476009, 0.0495882, 0.0513053, 0.0527020, 0.0563533, 0.0614435, 0.0650522, 0.0675199, 0.0693970,
                        0.0705456, 0.0706748, 0.0686654, 0.0571383, 0.04     , 0.02     , 0.0      };//last three bins are put in by hand
      float bimp4[18]={-1        ,  0.481843, 0.886609 , 1.13462  , 1.33981  , 1.52064  , 1.95342  , 2.51881  , 2.97665  , 3.37416  , 3.72876  ,
                        4.05377  ,  4.35564 , 4.77463  , 5.30719  , 6.08296  , 7.6415   , 15       };


      // template-fit v2  (v2{2})
      //                       <0,       0-1,       1-2,       2-3,       3-4,       4-5,      5-10,     10-15,     15-20,     20-25,     25-30,
      //                    30-35,     35-40,     40-45,     45-50,     50-55,     55-60,     60-65,     65-70,     70-75,     75-80,    80-100,>100
      float v2_2 [23]={ 0.0749381, 0.0749381, 0.0782992, 0.0801742, 0.0814646, 0.0826611, 0.0853616, 0.0889407, 0.0913229, 0.0928072, 0.0933774,
                        0.0933299, 0.0926254, 0.0910148, 0.0889322, 0.086161 , 0.0828212, 0.0793132, 0.0755036, 0.0714726, 0.0668264, 0.055    , 0.05};//last two bins are put in by hand
      float bimp2[23]={-1        ,  0.481843, 0.886609 , 1.13462  , 1.33981  , 1.52064  , 1.95342  , 2.51881  , 2.97665  , 3.37416  , 3.72876  ,
                        4.05377  ,  4.35564 , 4.63868  , 4.91057  , 5.17773  , 5.43665  , 5.68739  , 5.94258  , 6.20668  , 6.49519  , 7.6415   , 15};


      //Evaluate ratio of v2{4}/delta 
      TGraph *graph_v2_4=new TGraph(18,bimp4,v2_4);
      TGraph *graph_v2_2=new TGraph(23,bimp2,v2_2);
      float ratio[23]={0};
      for(int i=0;i<23;i++){
        double vn_RP=graph_v2_4->Eval(bimp2[i]);
        double vn_2 =graph_v2_2->Eval(bimp2[i]);
        double delta=vn_2*vn_2 - vn_RP*vn_RP;
        if(delta>0){
          ratio[i]= vn_RP/sqrt(delta/2.0);
        }
        else{
          ATH_MSG_ERROR("vn{2}<vn{4} for b_imp = " << bimp2[i]);
          return StatusCode::FAILURE;
        }
      }

      m_graph_fluc=new TGraph(23,bimp2,ratio);
      delete graph_v2_4;
      delete graph_v2_2;
    }
    //Fluctuations for Ne+Ne (TODO)

    else{
       ATH_MSG_ERROR("Flow fluctuations are not implemented for the following case: " << m_flow_function_name);
    }
  }


  // Initialization terminated
  return StatusCode::SUCCESS;
}


CLHEP::HepRandomEngine* AddFlowByShifting::getRandomEngine(const std::string& streamName,
                                                           const EventContext& ctx) const
{
  ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, streamName);
  std::string rngName = name()+streamName;
  rngWrapper->setSeed( rngName, ctx );
  return rngWrapper->getEngine(ctx);
}


StatusCode AddFlowByShifting::execute() {
  ATH_MSG_INFO(">>> AddFlowByShifting from execute");

  const EventContext& ctx = Gaudi::Hive::currentContext();
  CLHEP::HepRandomEngine *rndmEngine = getRandomEngine("FLOW", ctx);
  // Get hijing event parameters
  const HijingEventParams *hijing_pars;
  if( evtStore()->retrieve(hijing_pars, "Hijing_event_params").isFailure() ) {
    ATH_MSG_ERROR("Could not retrieve Hijing_event_params");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("Event parameters: B = " << hijing_pars->get_b()<<
               "  BPhi = " << hijing_pars->get_bphi());


  // FIXME: changing data in the event store
  HijingEventParams *hijing_pars_nc = const_cast<HijingEventParams*> (hijing_pars);


  // Read Data from Transient Store
  const McEventCollection* mcCollptr;
  if ( evtStore()->retrieve(mcCollptr, m_inkey).isFailure() ) {
    ATH_MSG_ERROR("Could not retrieve truth McEventCollection");
    return StatusCode::FAILURE;
  }


  // Loop over all events in original McEventCollection and
  // Copy to a new (modifiable) collection
  McEventCollection::const_iterator citr;
  McEventCollection* mcFlowCollptr = new  McEventCollection();
  for (citr = mcCollptr->begin(); citr!=mcCollptr->end(); ++citr) {
    mcFlowCollptr->push_back(new HepMC::GenEvent(*(*citr)));
  }


  //Geneate the event-plane angles (some of them may or may not be used later on)
  //Store the angles into the hijing event parameters
  for(int ihar=0;ihar<6;ihar++){
    m_psi_n[ihar] =(CLHEP::RandFlat::shoot(rndmEngine)-0.5)*2*M_PI / (ihar+1);   //Principal value must be within -PI/n to PI/n
    hijing_pars_nc->set_psi(ihar+1,m_psi_n[ihar]);
  }
  m_psi_n[1]=hijing_pars->get_bphi()                   ;//the psi2 plane is aligned with the impact parameter
  m_psi_n[1]=std::atan2(std::sin(2*m_psi_n[1]),std::cos(2*m_psi_n[1]))/2.0;//ensure that Psi2 is within [-PI/2,PI/2]
  hijing_pars_nc->set_psi(2,m_psi_n[1]);
  ATH_MSG_DEBUG(" Psi2 for event : "<<(*hijing_pars).get_psi(2));


  // Add flow by phi angle shifting
  McEventCollection::iterator itr;
  for (itr = mcFlowCollptr->begin(); itr!=mcFlowCollptr->end(); ++itr) {
    ATH_MSG_DEBUG("Next event in the bag ...");



#ifdef HEPMC3
    auto mainvtx=(*itr)->vertices().front();
    if(m_flow_fluctuations) Set_EbE_Fluctuation_Multipliers(mainvtx,hijing_pars->get_b(),rndmEngine);
    int particles_in_event = (*itr)->particles().size();
    m_particles_processed = 0;
    for ( auto parent: mainvtx->particles_out())
#else
    auto mainvtx=*((*itr)->vertices_begin());
    if(m_flow_fluctuations) Set_EbE_Fluctuation_Multipliers(mainvtx,hijing_pars->get_b(),rndmEngine);
      int particles_in_event = (*itr)->particles_size();
    m_particles_processed = 0;
    for ( auto parent: *mainvtx)
#endif
      {
        // Process particles from main vertex
        CLHEP::HepLorentzVector momentum(parent->momentum().px(),
                                         parent->momentum().py(),
                                         parent->momentum().pz(),
                                         parent->momentum().e());
        ATH_MSG_DEBUG("Parent particle: " << parent        <<
                      " Eta = "           << momentum.pseudoRapidity()<<
                      " Phi = "           << momentum.phi()            );

        //skip particle if eta is outside implementation range
        if(m_floweta_sw){
          float eta=std::abs(momentum.pseudoRapidity());
          if (eta<m_flow_mineta || eta> m_flow_maxeta) continue;
        }

        //skip particle if pT is outside implementation range
        if(m_flowpt_sw){
          float pT=momentum.perp();
          if (pT<m_flow_minpt || pT> m_flow_maxpt) continue;
        }

        // Randomize phi if explicitely requested
        if(m_ranphi_sw) {
          double phishift = SetParentToRanPhi(parent, rndmEngine);
          MoveDescendantsToParent(parent, phishift)  ;// adjust descendants to parent position
        }

        // Add flow to particles from main vertex
        double phishift = AddFlowToParent(parent, hijing_pars);
        MoveDescendantsToParent(std::move(parent), phishift);// adjust descendants to parent position
      }

    // correct for double counting
    if(m_ranphi_sw) m_particles_processed /= 2;
    // correct for incoming particles
    ATH_MSG_INFO( " Particles in event: " << particles_in_event <<
                  " Processed for flow: " << m_particles_processed+2);
    if(particles_in_event != (m_particles_processed+2)){
      ATH_MSG_WARNING( " Particles in event: " << particles_in_event <<
                       " Processed for flow: " << m_particles_processed+2);
    }
  }

  if(evtStore()->record(mcFlowCollptr, m_outkey).isFailure()){
    ATH_MSG_ERROR("Could not record flow McEventCollection");
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}


StatusCode AddFlowByShifting::finalize() {
  ATH_MSG_INFO(">>> AddFlowByShifting from finalize <<<");
  if(m_graph_fluc){
    delete m_graph_fluc ;
    m_graph_fluc=nullptr;
  }

  // End of finalization step
  return StatusCode::SUCCESS;
}


double AddFlowByShifting::SetParentToRanPhi(HepMC::GenParticlePtr parent,
                                            CLHEP::HepRandomEngine *rndmEngine)
{
  // Set particle to random phi
  // Return phi shift
  m_particles_processed++;

  double phi, phishift;
  CLHEP::HepLorentzVector momentum(parent->momentum().px(),
                                   parent->momentum().py(),
                                   parent->momentum().pz(),
                                   parent->momentum().e());
  phi = momentum.phi();

  double rannum = CLHEP::RandFlat::shoot(rndmEngine);
  double ranphi = (rannum-0.5)*2*M_PI;
  phishift = ranphi - phi;

  momentum.setPhi(ranphi*Gaudi::Units::rad);
  parent->set_momentum(  HepMC::FourVector(momentum.px(),momentum.py(),momentum.pz(),momentum.e()) );

  ATH_MSG_DEBUG("Parent phi randomized = " << momentum.phi());

  return phishift;
}


void AddFlowByShifting::MoveDescendantsToParent
(HepMC::GenParticlePtr parent, double phishift)
{
  // Move the branch of descendant vertices and particles
  // by phishift to parent particle position
  auto endvtx = parent->end_vertex();
  if ( endvtx ) {
    ATH_MSG_DEBUG("Processing branch of parent particle "<< parent);

    //Added October 2025
    // --- Rotate the parent’s end vertex itself ---
    if (std::abs(phishift) > 1e-7) {
      CLHEP::HepLorentzVector pos(endvtx->position().x(),
                                  endvtx->position().y(),
                                  endvtx->position().z(),
                                  endvtx->position().t());
      pos.rotateZ(phishift * Gaudi::Units::rad);
      endvtx->set_position(HepMC::FourVector(pos.x(), pos.y(), pos.z(), pos.t()));
    }


    //Added October 2025
    // --- Rotate the parent’s immediate daughters (outgoing of endvtx) ---
    #ifdef HEPMC3
    for (auto child : endvtx->particles_out()) 
    #else
    for (auto child : *endvtx) 
    #endif
    {
      CLHEP::HepLorentzVector p(child->momentum().px(),
                                child->momentum().py(),
                                child->momentum().pz(),
                                child->momentum().e());
      if (std::abs(phishift) > 1e-7) {
        p.rotateZ(phishift * Gaudi::Units::rad);
        child->set_momentum(HepMC::FourVector(p.px(), p.py(), p.pz(), p.e()));
      }
      ++m_particles_processed;
    }


    // now rotate descendant vertices
    #ifdef HEPMC3
    for (HepMC::GenVertexPtr descvtx:  HepMC::descendant_vertices(endvtx)) { //}
    #else
    for ( HepMC::GenVertex::vertex_iterator
            descvtxit = endvtx->vertices_begin(HepMC::descendants);
            descvtxit != endvtx->vertices_end(HepMC::descendants);
            ++descvtxit) {
      auto descvtx = (*descvtxit);
    #endif

      ATH_MSG_DEBUG("Processing vertex " << descvtx);

      // rotate vertex
      if(std::abs(phishift) > 1e-7) {
        CLHEP::HepLorentzVector position(descvtx->position().x(),
                                         descvtx->position().y(),
                                         descvtx->position().z(),
                                         descvtx->position().t());
        position.rotateZ(phishift*Gaudi::Units::rad);
        descvtx->set_position(HepMC::FourVector( position.x(),position.y(),position.z(),position.t()) );
      }

      // now rotate their associated particles
      #ifdef HEPMC3
      for (auto descpart: descvtx->particles_out())
      #else
      for (auto descpart: *descvtx)
      #endif
      {
        CLHEP::HepLorentzVector momentum(descpart->momentum().px(),
                                          descpart->momentum().py(),
                                          descpart->momentum().pz(),
                                          descpart->momentum().e());
        ATH_MSG_DEBUG("Descendant particle: " << descpart <<
                      " Eta = "               << descpart->momentum().pseudoRapidity() <<
                      " Phi = "               << descpart->momentum().phi() );
        m_particles_processed++;
        // rotate particle
        if(std::abs(phishift) > 1e-7) {
          momentum.rotateZ(phishift*Gaudi::Units::rad);
          descpart->set_momentum( HepMC::FourVector(momentum.px(),momentum.py(),momentum.pz(),momentum.e()) );
          ATH_MSG_DEBUG(" Phi shift =   " << phishift<<
                        " Phi shifted = " << momentum.phi());
        }
      }
    }
  }
  return;
}


double AddFlowByShifting::AddFlowToParent (HepMC::GenParticlePtr parent, const HijingEventParams *hijing_pars)
{
  m_particles_processed++;
  CLHEP::HepLorentzVector momentum(parent->momentum().px(),
                                   parent->momentum().py(),
                                   parent->momentum().pz(),
                                   parent->momentum().e());
  double pt    = parent->momentum().perp();
  double eta   = parent->momentum().pseudoRapidity();
  double phi_0 = parent->momentum().phi();

  int error_=0;
  if(pt    !=pt)    {ATH_MSG_ERROR("ERROR pt  of track  is not defined");error_=1;} //true if pt==nan
  if(eta   !=eta)   {ATH_MSG_ERROR("ERROR eta of track  is not defined");error_=1;}
  if(phi_0 !=phi_0) {ATH_MSG_ERROR("ERROR phi of track  is not defined");error_=1;}
  if(error_==1){
    ATH_MSG_ERROR("Original Particle Momentum(px,py,pz,e,m)="<<parent->momentum().px()<<"  "
                  <<parent->momentum().py()<<"  "
                  <<parent->momentum().pz()<<"  "
                  <<parent->momentum().e() <<"  "
                  <<parent->momentum().m() <<"  ");
  }




  //Call the appropriate function to set the vn values
  for(int ihar = 0; ihar< Harmonic::NumHar; ihar++){m_v_n  [ihar]=0.0;} //reset the vn for this particle
  double b = hijing_pars->get_b();
  (*this.*m_flow_function)(b,eta,pt);//Set the vn for this particle

  //add EbE fluctuations
  if(m_flow_fluctuations){
    for(int ihar = 0; ihar< Harmonic::NumHar; ihar++){
      m_v_n[ihar] *= m_EbE_Multiplier_vn[ihar];
      if(m_v_n[ihar]>0.5){
        ATH_MSG_WARNING(" Vn Too large  "<<ihar+1<<"  "<<m_EbE_Multiplier_vn[ihar]<<"  "<<m_v_n[ihar]);m_v_n[ihar]=0.5;
      }
    }
  }

  double phishift=0;

  // Old fashioned rotation(approximate)
  if (m_flow_implementation_type==0){
    float phi=phi_0;
    phishift=  -2*( m_v_n[Harmonic::v1]*sin(1*(phi-m_psi_n[Harmonic::v1]))/1.0 + 
                    m_v_n[Harmonic::v2]*sin(2*(phi-m_psi_n[Harmonic::v2]))/2.0 +
                    m_v_n[Harmonic::v3]*sin(3*(phi-m_psi_n[Harmonic::v3]))/3.0 + 
                    m_v_n[Harmonic::v4]*sin(4*(phi-m_psi_n[Harmonic::v4]))/4.0 +
                    m_v_n[Harmonic::v5]*sin(5*(phi-m_psi_n[Harmonic::v5]))/5.0 + 
                    m_v_n[Harmonic::v6]*sin(6*(phi-m_psi_n[Harmonic::v6]))/6.0 );

  }

  // New fashioned rotation(exact)
  else if (m_flow_implementation_type==1){
    // Thread-safe according to https://www.gnu.org/software/gsl/doc/html/roots.html
    const gsl_root_fsolver_type *T ATLAS_THREAD_SAFE = gsl_root_fsolver_brent;
    gsl_root_fsolver *s = gsl_root_fsolver_alloc (T);
    double x_lo=-2*M_PI,x_hi=2*M_PI;
    float params[13];
    for(int ipar=0;ipar<13;ipar++) {params[ipar]=0;}
    gsl_function F;
    F.function = &(AddFlowByShifting::vn_func);
    F.params =&params;
    gsl_root_fsolver_set (s, &F, x_lo, x_hi);
    int iter=0;
    params[ 0]=phi_0;
    params[ 1]=m_v_n  [Harmonic::v1];
    params[ 2]=m_v_n  [Harmonic::v2];
    params[ 3]=m_v_n  [Harmonic::v3];
    params[ 4]=m_v_n  [Harmonic::v4];
    params[ 5]=m_v_n  [Harmonic::v5];
    params[ 6]=m_v_n  [Harmonic::v6];
    params[ 7]=m_psi_n[Harmonic::v1];
    params[ 8]=m_psi_n[Harmonic::v2];
    params[ 9]=m_psi_n[Harmonic::v3];
    params[10]=m_psi_n[Harmonic::v4];
    params[11]=m_psi_n[Harmonic::v5];
    params[12]=m_psi_n[Harmonic::v6];
    int status;
    double phi=phi_0;
    do
      {
        iter++;
        status = gsl_root_fsolver_iterate (s);
        phi = gsl_root_fsolver_root (s);
        x_lo = gsl_root_fsolver_x_lower (s);
        x_hi = gsl_root_fsolver_x_upper (s);
        status = gsl_root_test_interval (x_lo, x_hi,0, 0.00001);
      }
    while (status == GSL_CONTINUE && iter < 1000);
    gsl_root_fsolver_free (s);

    if (iter>=1000) return 0;

    phishift = phi-phi_0;
  }

  if(std::abs(phishift) > 1e-7) {
    momentum.rotateZ(phishift*Gaudi::Units::rad);
    parent->set_momentum( HepMC::FourVector(momentum.px(),momentum.py(),momentum.pz(),momentum.e()) );
  }
  ATH_MSG_DEBUG( "Parent particle:"   <<
                 " V1 = "             << m_v_n[Harmonic::v1] <<
                 " V2 = "             << m_v_n[Harmonic::v2] <<
                 " V3 = "             << m_v_n[Harmonic::v3] <<
                 " V4 = "             << m_v_n[Harmonic::v4] <<
                 " V5 = "             << m_v_n[Harmonic::v5] <<
                 " V6 = "             << m_v_n[Harmonic::v6] <<
                 " Phi shift = "      << phishift            <<
                 " Phi shifted = "    << momentum.phi() );

  return phishift;
}



// New parameterization for Pb+Pb vn
void AddFlowByShifting::jjia_minbias_new(double b, double eta, double pt)
{
  pt=pt/1000.0; //convert to GeV

  float a1,a2,a3,a4;
  a1=0.4397*std::exp(-(b-4.526)*(b-4.526)/72.0) + 0.636;
  a2=1.916/(b+2) +0.1;
  a3=4.79*0.0001*(b-0.621)*(b-10.172)*(b-23)+1.2;   // this is >0 for b>0
  a4=0.135*std::exp(-0.5*(b-10.855)*(b-10.855)/4.607/4.607) +0.0120;

  float temp1 = std::pow(pt    , a1) / (1+std::exp( (pt-3.0)/a3));
  float temp2 = std::pow(pt+0.1,-a2) / (1+std::exp(-(pt-4.5)/a3));
  float temp3 =  0.01           / (1+std::exp(-(pt-4.5)/a3));

  m_v_n[Harmonic::v2] = ( a4*(temp1+temp2) + temp3 )* std::exp(-0.5* eta*eta /6.27/6.27) ;

  float fb=0.97 +1.06*std::exp(-0.5*b*b/3.2/3.2);
  m_v_n[Harmonic::v3]=std::pow(fb*std::sqrt(m_v_n[1]),3);

  float gb= 1.096 +1.36 *std::exp(-0.5*b*b/3.0/3.0);
  gb=gb*sqrt(m_v_n[1]);
  m_v_n[Harmonic::v4]=pow(gb,4);
  m_v_n[Harmonic::v5]=pow(gb,5);
  m_v_n[Harmonic::v6]=pow(gb,6);
  m_v_n[Harmonic::v1]=0;
}


// New parameterization for Pb+Pb vn (v2 only)
void AddFlowByShifting::jjia_minbias_new_v2only(double b, double eta, double pt)
{
  jjia_minbias_new(b, eta, pt);
  //Set all harmonics except v2 to 0
  m_v_n[Harmonic::v1] = 0;
  m_v_n[Harmonic::v3] = 0;
  m_v_n[Harmonic::v4] = 0;
  m_v_n[Harmonic::v5] = 0;
  m_v_n[Harmonic::v6] = 0;
}


// Fixed vn
void AddFlowByShifting::fixed_vn(double /*b*/, double /*eta*/, double /*pt*/)
{
  m_v_n[Harmonic::v1]=0.0000;
  m_v_n[Harmonic::v2]=0.0500;
  m_v_n[Harmonic::v3]=0.0280;
  m_v_n[Harmonic::v4]=0.0130;
  m_v_n[Harmonic::v5]=0.0045;
  m_v_n[Harmonic::v6]=0.0015;
}


// Fixed 5% v2 (other vn=0)
void AddFlowByShifting::fixed_v2(double /*b*/, double /*eta*/, double /*pt*/)
{
  for(int ihar=0;ihar<Harmonic::NumHar;ihar++){
    m_v_n[ihar]=0.0;
  }
  m_v_n[Harmonic::v2]=0.0500;
}


// Old parameterization for Pb+Pb v2
void AddFlowByShifting::jjia_minbias_old(double b, double eta, double pt)
{
  m_v_n[Harmonic::v1] = 0;
  m_v_n[Harmonic::v2] = 0.03968 * b
    * (1 - 2.1/(1 + std::exp(1.357*(pt/1000))))
    * std::exp(-(eta*eta)/(2*6.37*6.37));
  m_v_n[Harmonic::v3]=0.0000;
  m_v_n[Harmonic::v4]=0.0000;
  m_v_n[Harmonic::v5]=0.0000;
  m_v_n[Harmonic::v6]=0.0000;
}


void AddFlowByShifting::ao_test (double b, double /*eta*/, double pt)
{
  for(int ihar=0;ihar<Harmonic::NumHar;ihar++){
    m_v_n[ihar]=0.0;
  }

  pt/=1000;
  if(pt>2) pt = 2; // flat max at pt > 2

  m_v_n[Harmonic::v2] = 0.02 * b * pt;
}


void AddFlowByShifting::custom_vn (double /*b*/, double /*eta*/, double /*pt*/)
{
  m_v_n[Harmonic::v1]=m_custom_v1;
  m_v_n[Harmonic::v2]=m_custom_v2;
  m_v_n[Harmonic::v3]=m_custom_v3;
  m_v_n[Harmonic::v4]=m_custom_v4;
  m_v_n[Harmonic::v5]=m_custom_v5;
  m_v_n[Harmonic::v6]=m_custom_v6;
}

// p_Pb vn
void AddFlowByShifting::p_Pb_cent_eta_indep(double /*b*/, double /*eta*/, double pt)
{
  pt=pt/1000.0; //convert to GeV

  float an_val[4][3];

  an_val[0][0] = 0.1149;
  an_val[0][1] = 1.181;
  an_val[0][2] = 0.3767;

  an_val[1][0] = 0.0498;
  an_val[1][1] = 1.688;
  an_val[1][2] = 0.5046;

  an_val[2][0] = 0.02095;
  an_val[2][1] = 2.196;
  an_val[2][2] = 0.6259;

  an_val[3][0] = 0.00682*0.5;//added in 0.5 factor by hand
  an_val[3][1] = 4.938;
  an_val[3][2] = 1.237;

  m_v_n[Harmonic::v1]=0;
  m_v_n[Harmonic::v2]=an_val[0][0]*std::pow(pt,an_val[0][1])*std::exp(-an_val[0][2]*pt);
  m_v_n[Harmonic::v3]=an_val[1][0]*std::pow(pt,an_val[1][1])*std::exp(-an_val[1][2]*pt);
  m_v_n[Harmonic::v4]=an_val[2][0]*std::pow(pt,an_val[2][1])*std::exp(-an_val[2][2]*pt);
  m_v_n[Harmonic::v5]=an_val[3][0]*std::pow(pt,an_val[3][1])*std::exp(-an_val[3][2]*pt);
  m_v_n[Harmonic::v6]=0;
}

//OO
void AddFlowByShifting::OO_eta_indep(double b, double /*eta*/, double pt){
  pt=pt/1000.0; //convert to GeV


  //For Testing
  /*
  m_v_n[Harmonic::v1]=0;
  m_v_n[Harmonic::v2]=0.1;
  m_v_n[Harmonic::v3]=0.05;
  m_v_n[Harmonic::v4]=0.03;
  m_v_n[Harmonic::v5]=0;
  m_v_n[Harmonic::v6]=0;
  return;
  */
  

  //pT differential vn for 0-5% centrality
  static TGraph gr_v2pt;
  static TGraph gr_v3pt;
  static TGraph gr_v4pt;
  //centrality dependent scale-factors for other centralities
  static TGraph gr_v2_cent_scale;
  static TGraph gr_v3_cent_scale;
  static TGraph gr_v4_cent_scale;

  static bool is_initialized=false;
  if(is_initialized==false){
    //----------------------------------------------------------------------------------
    //The pt Dependent v2,v3,v4 for the 0-5% centrality interval from template-fit method
    //https://atlas.web.cern.ch/Atlas/GROUPS/PHYSICS/PAPERS/HION-2025-02/fig_05.png
    const int NumPtBins=23;
    float v2_pt[NumPtBins]={
      0.0      ,//extrapolation
      0.049533 ,0.0622719,0.0776591,0.0906141,0.101341 ,0.110451 ,0.117545,
      0.123217 ,0.126643 ,0.129142 ,0.129176 ,0.128795 ,0.126009 ,0.120966,
      0.109416 ,0.0966881,0.0825195,0.0708585,0.0615164,0.0508748,
      0.015     ,0.015  //extrapolation to  1.5% v2 at high-pT
    }; 
    float v3_pt[NumPtBins]={
      0.0      ,//extrapolation
      0.0186328,0.0262968,0.0366394,0.0467804,0.0562739,0.0639566,0.0714195,
      0.0783429,0.0838629,0.0886202,0.0907969,0.0922756,0.0948961,0.0947266,
      0.0898434,0.0839913,0.0801961,0.0685197,0.0648406,0.0600871,
      0.00     ,0.00//extrapolation
    };
    //----------------------------------------------------
    //For v3, we use the average of the template and 2PC values
    float v3_pt_2PC[NumPtBins]={
      0.0      ,//extrapolation
      0.0182106,0.0255873,0.0355349,0.0451583,0.0537545,0.0611133,0.0675791,
      0.0732434,0.0778371,0.0812549,0.0823093,0.0826907,0.082954 ,0.0800712,
      0.0719686,0.062255 ,0.0498296,0.0370541,0.027638 ,0.0139785,
      0.00     ,0.00//extrapolation
    };
    for(int i=0;i<NumPtBins;i++) v3_pt[i]=(v3_pt[i]+v3_pt_2PC[i])/2.0;
    //----------------------------------------------------

    float v4_pt[NumPtBins]={
      0.0       ,//extrapolation
      0.00446155,0.00710366,0.0117637,0.0158203,0.01931  ,0.0234234,0.0268995,
      0.0299693 ,0.0321662 ,0.0335182,0.0358789,0.0374195,0.0359359,0.0327255,
      0.0254762 ,0.0273239 ,0.0212901,0.0152591, -0      ,-0       ,//-ve values replaced by -0
      0.0       , 0.00//extrapolation
    };
    float pt_bins[NumPtBins]={
      0.0      ,//extrapolation
      0.55     ,0.7      ,0.9      ,1.1      ,      1.3,      1.5,     1.7,
      1.9      ,2.1      ,2.3      ,2.5      ,      2.7,      2.9,    3.25,
      3.75     ,4.25     ,4.75     ,5.25     ,     5.75,      6.5,    
      10.0     ,10000    //extrapolation
    };
    gr_v2pt=TGraph(NumPtBins,pt_bins,v2_pt);
    gr_v3pt=TGraph(NumPtBins,pt_bins,v3_pt);
    gr_v4pt=TGraph(NumPtBins,pt_bins,v4_pt);
    gr_v2pt.SetBit(TGraph::kIsSortedX);//For faster return From Eval()
    gr_v3pt.SetBit(TGraph::kIsSortedX);//For faster return From Eval()
    gr_v4pt.SetBit(TGraph::kIsSortedX);//For faster return From Eval()
    //----------------------------------------------------------------------------------
  
  
    //----------------------------------------------------------------------------------
    //centrality dependent v2, v3, v4 (template-fit method)
    //https://atlas.web.cern.ch/Atlas/GROUPS/PHYSICS/PAPERS/HION-2025-02/figaux_03.png
    const int NumCentBins=23;
    float v2_cent[NumCentBins]={
      0.0749381,//underflow
      0.0749381,0.0782992,0.0801742,0.0814646,0.0826611,0.0853616,0.0889407,
      0.0913229,0.0928072,0.0933774,0.0933299,0.0926254,0.0910148,0.0889322,
      0.086161 ,0.0828212,0.0793132,0.0755036,0.0714726,0.0668264,
      0.055    , 0.05 //80-100 and overflow, put by hand ; using 5% v2 asymptotically
    };
    float v3_cent[NumCentBins]={
      0.0409694,//underflow
      0.0409694,0.0399241,0.0394262,0.0388545,0.0385726,0.0375281,0.0360179,
      0.0345174,0.0330085,0.0314149,0.0298081,0.0282141,0.02664  ,0.0253094,
      0.0239221,0.0220779,0.0213821,0.0203176,0.0175276,0.0151146,
      0.010    , 0.0 //80-100 and overflow, put by hand; using 0% v3 asymptotically 
    };
    float v4_cent[NumCentBins]={
      0.0131927 ,//underflow
      0.0131927 ,0.0129838,0.0135232 ,0.0132689 ,0.0130435 ,0.0134254 ,0.013706 ,
      0.0136456 ,0.0136656,0.013245  ,0.0130861 ,0.0125356 ,0.0118006 ,0.0110316,
      0.00987005,0.0106963,0.00933145,0.00784534,0.00393584,0.00308131,
      0.003    , 0.0 //80-100 and overflow, put by hand; using 0% v4 asymptotically
    };
    //b_imp values for differen centralities
    float b_imp[NumCentBins]={
      -1        ,  //underflow
      0.481843  ,0.886609 , 1.13462  , 1.33981  , 1.52064  , 1.95342  , 2.51881  , 
      2.97665   ,3.37416  , 3.72876  , 4.05377  ,  4.35564 , 4.63868  , 4.91057  , 
      5.17773   ,5.43665  , 5.68739  , 5.94258  , 6.20668  , 6.49519  , 
      7.6415   , 15 , //80-100 and overflow
    };

    //Integrated v2, v3, v4, for 0-5% centrality bin
    float vn_0_5[3]={0.078963056, 0.039746672, 0.013202649};
    //divide by integrated vn for 0-5% centrality
    for(int i=0;i<NumCentBins;i++){
      v2_cent[i]/=vn_0_5[0];
      v3_cent[i]/=vn_0_5[1]; 
      v4_cent[i]/=vn_0_5[2];
    }
    gr_v2_cent_scale=TGraph(NumCentBins, b_imp, v2_cent);
    gr_v3_cent_scale=TGraph(NumCentBins, b_imp, v3_cent);
    gr_v4_cent_scale=TGraph(NumCentBins, b_imp, v4_cent);
    gr_v2_cent_scale.SetBit(TGraph::kIsSortedX);//For faster return From Eval()
    gr_v3_cent_scale.SetBit(TGraph::kIsSortedX);//For faster return From Eval()
    gr_v4_cent_scale.SetBit(TGraph::kIsSortedX);//For faster return From Eval()
    //----------------------------------------------------------------------------------
    is_initialized=true;
  }


  m_v_n[Harmonic::v1]=0;
  m_v_n[Harmonic::v2]=gr_v2pt.Eval(pt) * gr_v2_cent_scale.Eval(b);
  m_v_n[Harmonic::v3]=gr_v3pt.Eval(pt) * gr_v3_cent_scale.Eval(b);
  m_v_n[Harmonic::v4]=gr_v4pt.Eval(pt) * gr_v4_cent_scale.Eval(b);
  m_v_n[Harmonic::v5]=0;
  m_v_n[Harmonic::v6]=0;
}

void AddFlowByShifting::Set_EbE_Fluctuation_Multipliers(HepMC::GenVertexPtr /*mainvtx*/,float b, CLHEP::HepRandomEngine *rndmEngine){
  for(int ihar=0;ihar<Harmonic::NumHar;ihar++){
    m_EbE_Multiplier_vn[ihar]=1.0;
  }


  for(int ihar=0;ihar<Harmonic::NumHar;ihar++){
    float vn_rp=0,delta=0;//BG parameterizations

    //in the following we assume that the vn in the event is sqrt(<vn^2>)
    //This is because the vn(pT) were tuned to 2PC/EP measurements
    //then : vn_evt=vn_rp^2 + 2*(delta^2)
    //which is used together with the "alpha" to get the "vn_rp" and "delta"
 
    //No EbE fluctuation for v1
    if     (ihar==Harmonic::v1) continue; 
    //v2
    else if(ihar==Harmonic::v2){    
      //alpha stores ratio of V2_RP over delta
      float alpha=m_graph_fluc->Eval(b);
      delta=1.0f/sqrt(2.0f+alpha*alpha); //stores delta  /v2{2}
      vn_rp=alpha*delta              ; //stores v2^{RP}/v2{2}       
    }
    //v3-v6
    else{ 
      vn_rp =0;
      delta=1.0f/sqrt(2.0f);
    }
    float X=CLHEP::RandGaussQ::shoot(rndmEngine,vn_rp,delta);
    float Y=CLHEP::RandGaussQ::shoot(rndmEngine,0.0  ,delta);
    m_EbE_Multiplier_vn[ihar]=sqrt(X*X+ Y*Y);
    ATH_MSG_INFO("EbE_Multiplier_v"<<ihar+1<<"="<<m_EbE_Multiplier_vn[ihar]);
  }
}

