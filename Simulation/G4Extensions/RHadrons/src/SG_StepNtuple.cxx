/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "SG_StepNtuple.h"

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/SmartDataPtr.h"

#include "TruthUtils/HepMCHelpers.h"

#include "SimHelpers/ServiceAccessor.h"

#include "G4Step.hh"
#include "G4Event.hh"

#include <iostream>


namespace G4UA
{

  SG_StepNtuple::SG_StepNtuple(const std::vector<int>& pdgids)
    : AthMessaging(Gaudi::svcLocator()->service< IMessageSvc >( "MessageSvc" ),
                   "SG_StepNtuple")
  {
    // Load up the PDG IDs
    for (int i : pdgids ){
      m_rhs.insert(i);
    }
  }

  void SG_StepNtuple::BeginOfRunAction(const G4Run*)
  {
    NTupleFilePtr file1(ntupleSvc(), "/NTUPLES/FILE1");

    SmartDataPtr<NTuple::Directory>
      ntdir(ntupleSvc(),"/NTUPLES/FILE1/StepNtuple");

    if ( !ntdir ) ntdir = ntupleSvc()->createDirectory(file1,"StepNtuple");
    //if ( !ntdir ) log << MSG::ERROR << " failed to get ntuple directory" << endmsg;
    NTuplePtr nt(ntupleSvc(), "/NTUPLES/FILE1/StepNtuple/10");
    if ( !nt ) {    // Check if already booked
      nt = ntupleSvc()->book (ntdir.ptr(), 10,CLID_ColumnWiseTuple, "GEANT4 Step NTuple");
      if ( nt ) {

        ATH_MSG_INFO("booked step ntuple ");

        if( nt->addItem ("nstep", m_nsteps,0 ,50000)!=StatusCode::SUCCESS // WARNING!! Force limit to 50k tracks
            || nt->addItem ("pdg", m_nsteps, m_pdg)!=StatusCode::SUCCESS
            || nt->addItem ("charge", m_nsteps, m_charge)!=StatusCode::SUCCESS
            || nt->addItem ("mass", m_nsteps, m_mass)!=StatusCode::SUCCESS
            || nt->addItem ("baryon", m_nsteps, m_baryon)!=StatusCode::SUCCESS
            || nt->addItem ("x1", m_nsteps, m_x1)!=StatusCode::SUCCESS
            || nt->addItem ("y1", m_nsteps, m_y1)!=StatusCode::SUCCESS
            || nt->addItem ("z1", m_nsteps, m_z1)!=StatusCode::SUCCESS
            || nt->addItem ("t1", m_nsteps, m_t1)!=StatusCode::SUCCESS
            || nt->addItem ("x2", m_nsteps, m_x2)!=StatusCode::SUCCESS
            || nt->addItem ("y2", m_nsteps, m_y2)!=StatusCode::SUCCESS
            || nt->addItem ("z2", m_nsteps, m_z2)!=StatusCode::SUCCESS
            || nt->addItem ("t2", m_nsteps, m_t2)!=StatusCode::SUCCESS
            || nt->addItem ("dep", m_nsteps, m_dep)!=StatusCode::SUCCESS
            || nt->addItem ("ke1", m_nsteps, m_ke1)!=StatusCode::SUCCESS
            || nt->addItem ("ke2", m_nsteps, m_ke2)!=StatusCode::SUCCESS
            || nt->addItem ("rh", m_nsteps, m_rh)!=StatusCode::SUCCESS
            || nt->addItem ("rhid", m_nsteps, m_rhid)!=StatusCode::SUCCESS
            || nt->addItem ("step", m_nsteps, m_step)!=StatusCode::SUCCESS
            || nt->addItem ("pt1", m_nsteps, m_pt1)!=StatusCode::SUCCESS
            || nt->addItem ("pt2", m_nsteps, m_pt2)!=StatusCode::SUCCESS
            || nt->addItem ("minA",m_nsteps, m_minA)!=StatusCode::SUCCESS
            || nt->addItem ("v2",m_nsteps, m_v2)!=StatusCode::SUCCESS
            || nt->addItem ("vthresh",m_nsteps, m_vthresh)!=StatusCode::SUCCESS
            || nt->addItem ("vbelowthresh",m_nsteps, m_vbelowthresh)!=StatusCode::SUCCESS
            || nt->addItem ("evtid", m_evtid)!=StatusCode::SUCCESS)

          ATH_MSG_ERROR("Could not configure branches ");

      }

      else ATH_MSG_ERROR("Could not book step ntuple!! ");
    }

    //set initial values
    m_nevents=0;

  }

  void SG_StepNtuple::BeginOfEventAction(const G4Event*)
  {
    m_nsteps=0;
    m_rhadronIndex=0;//the rhadron index (either the first or second rhadon, usually)
    m_nevents++;
    m_evtid=m_nevents;//since it gets cleared out after every fill...
  }

  void SG_StepNtuple::EndOfEventAction(const G4Event*)
  {
    if(! ntupleSvc()->writeRecord("/NTUPLES/FILE1/StepNtuple/10").isSuccess()) {
      ATH_MSG_ERROR( " failed to write record for this event" );
    }

    //this also seems to zero out all the arrays... so beware!
  }

  void SG_StepNtuple::UserSteppingAction(const G4Step* aStep)
  {
    if(m_nsteps<50000){
      const int pdg_id = aStep->GetTrack()->GetDefinition()->GetPDGEncoding();
      bool rhad=false;
      if (std::find(m_rhs.begin(),m_rhs.end(),std::abs(pdg_id))!=m_rhs.end()) {
        rhad=true;
      }

      //
      if (!rhad && (MC::isSquarkLH(pdg_id) ||
                    pdg_id == 1000021 || // gluino
                    MC::isRHadron(pdg_id))) {
        ATH_MSG_DEBUG (" TruthUtils classifies "<<pdg_id<<" as an R-Hadron, gluino or LH Squark!");
        rhad=true;
      }
      //

      if (rhad){

        //
        G4Material * mat = aStep->GetTrack()->GetMaterial();
        double minA=1500000.;
        for (unsigned int i=0;i<mat->GetNumberOfElements();++i){
          if (mat->GetElement(i) && minA>mat->GetElement(i)->GetN()){
            minA=mat->GetElement(i)->GetN();
          }
        }
        //

        bool firstslow = aStep->GetPostStepPoint()->GetVelocity()<0.15*std::pow(minA,-2./3.)*CLHEP::c_light;
        //just save the first slow step for the rhadron
        for (int i=0; i<m_nsteps; ++i){
          if (m_rhid[i]==m_rhadronIndex && m_vbelowthresh[i]>0) firstslow=false;
        }
        if (firstslow || aStep->GetTrack()->GetCurrentStepNumber()<=1 || aStep->GetPostStepPoint()->GetKineticEnergy()==0.){

          //
          if (MC::isSquarkLH(pdg_id) ||
           pdg_id == 1000021 || // gluino
           MC::isRHadron(pdg_id)) {
            m_rh[m_nsteps] = 1;// TruthUtils classifies this particle as an R-Hadron, gluino or LH squark
          }
          else{
            m_rh[m_nsteps] = 0;// particle only passes the RHadronPDGIDList property check
          }
          //

          if (aStep->GetPreStepPoint()->GetGlobalTime()==0) m_rhadronIndex++;
          m_rhid[m_nsteps]=m_rhadronIndex;

          m_pdg[m_nsteps]=pdg_id;
          m_charge[m_nsteps]=aStep->GetTrack()->GetDefinition()->GetPDGCharge();
          m_dep[m_nsteps]=aStep->GetTotalEnergyDeposit();
          m_mass[m_nsteps]=aStep->GetTrack()->GetDefinition()->GetPDGMass();
          m_baryon[m_nsteps]=aStep->GetTrack()->GetDefinition()->GetBaryonNumber();
          m_t1[m_nsteps]=aStep->GetPreStepPoint()->GetGlobalTime();
          m_t2[m_nsteps]=aStep->GetPostStepPoint()->GetGlobalTime();
          G4ThreeVector pos1=aStep->GetPreStepPoint()->GetPosition();
          m_x1[m_nsteps]=pos1.x();
          m_y1[m_nsteps]=pos1.y();
          m_z1[m_nsteps]=pos1.z();
          G4ThreeVector pos2=aStep->GetPostStepPoint()->GetPosition();
          m_x2[m_nsteps]=pos2.x();
          m_y2[m_nsteps]=pos2.y();
          m_z2[m_nsteps]=pos2.z();
          m_ke1[m_nsteps]=aStep->GetPreStepPoint()->GetKineticEnergy();
          m_ke2[m_nsteps]=aStep->GetPostStepPoint()->GetKineticEnergy();
          m_pt1[m_nsteps]=aStep->GetPreStepPoint()->GetMomentum().perp();
          m_pt2[m_nsteps]=aStep->GetPostStepPoint()->GetMomentum().perp();

          //
          m_minA[m_nsteps]=minA;
          m_v2[m_nsteps]=aStep->GetPostStepPoint()->GetVelocity();
          m_vthresh[m_nsteps]=0.15*std::pow(minA,-2./3.)*CLHEP::c_light;
          m_vbelowthresh[m_nsteps]=(firstslow?1:0);
          //

          m_step[m_nsteps]=m_nsteps;
          ++m_nsteps;
          //std::cout<<"stepping, size is "<<m_nsteps<<std::endl;
        } // writing the step because it stopped or is the start of the "track"
      } //rhad true
      else {

        //KILL the particles here, so we don't waste time in GEANT4 tracking what happens to it!
        if (MC::isBSM(pdg_id)) { //  flag SUSY/BSM particles which are skipped
          ATH_MSG_DEBUG ("UserSteppingAction(): Killing uninteresting track with pdg_id "<<pdg_id);
        }
        aStep->GetTrack()->SetTrackStatus(fKillTrackAndSecondaries);
        const G4TrackVector *tv = aStep->GetSecondary();
        //if ((*tv).size()>0) std::cout<<" ... and its "<<(*tv).size()<<" secondaries"<<std::endl;
        for (unsigned int i=0;i<tv->size();i++){
          G4Track *t = (*tv)[i];
          t->SetTrackStatus(fKillTrackAndSecondaries);
        }

      } // not an rhad

    } //m_nsteps<50000
  }

} // namespace G4UA
