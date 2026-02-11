#include <IPPerformance/TrackTruthHelper.h>

TrackTruthHelpers::TrackTruthHelpers(double pt, double eta, double truthMatchProb)
  : m_PtCut(pt), m_EtaCut(eta), m_truthmatchprobability(truthMatchProb) {
    printCutflow = false;

}

/*
TrackTruthHelpers::TrackTruthHelpers(double pt, double eta, double truthMatchProb, std::string name, EL::Worker *worker)
  : m_PtCut(pt), m_EtaCut(eta), m_truthmatchprobability(truthMatchProb) {
    printCutflow = true;
    BookHistograms(name, worker);
}
TrackTruthHelpers::TrackTruthHelpers(std::string name, EL::Worker *worker)
  : m_PtCut(500.), m_EtaCut(2.5), m_truthmatchprobability(0.5) {
    printCutflow = true;
    BookHistograms(name, worker);
}
*/


bool TrackTruthHelpers::isStableParticle( const xAOD::TruthParticle* truth )
{
  //Check to see if its a stable particle
  if( truth->status() != 1 )
    return false;

  //Clearly for tracking we don't care about Neutrals
  if( const_cast<xAOD::TruthParticle*>(truth)->isNeutral() )
    return false;

  return true;
}

// -------------------------------------------------------------------
// Primary track definition
// -------------------------------------------------------------------
bool TrackTruthHelpers::isPrimary( const xAOD::TrackParticle* track )
{
  //Get associated truth particle
  const xAOD::TruthParticle* truth =  truthParticle( track );
  if( !truth  ){
    return false;
  }

  //Is the this a primary truth particle?
  if (!isPrimaryParticle( truth ))
    return false;

  // Don't use tracks with low truth math probability
  TruthMatchProbabilityCut truthMatchProbabilityCut(m_truthmatchprobability);
  if( !truthMatchProbabilityCut.accept(track, NULL) )
    return false;
  if(printCutflow) h_primary->Fill(6);

  return true;
}

bool TrackTruthHelpers::isPrimaryParticle( const xAOD::TruthParticle* truth )
{
  // All truth particles
  if(printCutflow) h_primary->Fill(0);

  //Check to see if its a stable particle
  if( truth->status() != 1 )
    return false;
  if(printCutflow) h_primary->Fill(1);

  //Clearly for tracking we don't care about Neutrals
  if( const_cast<xAOD::TruthParticle*>(truth)->isNeutral() )
    return false;
  if(printCutflow) h_primary->Fill(2);

  //Barcode of zero indicates there was no truth paticle found for this track
  if( truth->barcode() == 0 || truth->barcode() >= 200e3)
    return false;
  if(printCutflow) h_primary->Fill(3);

  //Particle is in phase space
  if( !passAcceptance(truth) )
    return false;
  if(printCutflow) h_primary->Fill(4);

  //Particle is not a strange baryon
  if( truth->isStrangeBaryon() )
    return false;
  if(printCutflow) h_primary->Fill(5);

  return true;
}


// -------------------------------------------------------------------
// Secondary track definition
// -------------------------------------------------------------------

bool TrackTruthHelpers::isSecondary( const xAOD::TrackParticle* track)
{
  //Get associated truth particle
  const xAOD::TruthParticle* truth =  truthParticle( track );
  if( !truth  ){
    return false;
  }

  //Is the this a secondary truth particle?
  if( !isSecondaryParticle( truth ) )
    return false;

  // Don't use tracks with low truth math probability
  TruthMatchProbabilityCut truthMatchProbabilityCut(m_truthmatchprobability);
  if( !truthMatchProbabilityCut.accept(track, NULL) )
    return false;
  if(printCutflow) h_secondary->Fill(4);

  return true;
}

bool TrackTruthHelpers::isSecondaryParticle( const xAOD::TruthParticle* truth )
{
  // All truth particles
  if(printCutflow) h_secondary->Fill(0);

  //Check to see if its a stable particle
  if( truth->status() != 1 )
    return false;
  if(printCutflow) h_secondary->Fill(1);

  //Clearly for tracking we don't care about Neutrals
  if( const_cast<xAOD::TruthParticle*>(truth)->isNeutral() )
    return false;
  if(printCutflow) h_secondary->Fill(2);

  //Larger than this indicates secondary  particles
  if( truth->barcode() < 200e3 )
    return false;
  if(printCutflow) h_secondary->Fill(3);

  return true;
}



// -------------------------------------------------------------------
// Strange baryon track definition
// -------------------------------------------------------------------

bool TrackTruthHelpers::isStrangeBaryon( const xAOD::TrackParticle* track)
{
  //Get associated truth particle
  const xAOD::TruthParticle* truth =  truthParticle( track );
  if( !truth  ){
    return false;
  }

  //Is the this a secondary truth particle?
  if( !isStrangeBaryonParticle( truth ) )
    return false;

  // Don't use tracks with low truth math probability
  TruthMatchProbabilityCut truthMatchProbabilityCut(m_truthmatchprobability);
  if( !truthMatchProbabilityCut.accept(track, NULL) )
    return false;
  if(printCutflow) h_strangeBaryon->Fill(5);

  return true;
}

bool TrackTruthHelpers::isStrangeBaryonParticle( const xAOD::TruthParticle* truth )
{
  // All truth particles
  if(printCutflow) h_strangeBaryon->Fill(0);

  //Check to see if its a stable particle
  if( truth->status() != 1 )
    return false;
  if(printCutflow) h_strangeBaryon->Fill(1);

  //Clearly for tracking we don't care about Neutrals
  if( const_cast<xAOD::TruthParticle*>(truth)->isNeutral() )
    return false;
  if(printCutflow) h_strangeBaryon->Fill(2);

  //No fakes and no secondaries
  if( truth->barcode() == 0 || truth->barcode() >= 200e3)
    return false;
  if(printCutflow) h_strangeBaryon->Fill(3);

  //Particle is a strange baryon
  if( !truth->isStrangeBaryon() )
    return false;
  if(printCutflow) h_strangeBaryon->Fill(4);

  return true;
}


// -------------------------------------------------------------------
// Fake definition
// -------------------------------------------------------------------

bool TrackTruthHelpers::isFake( const xAOD::TrackParticle* track )
{
  // All
  if(printCutflow) h_fake->Fill(0);

  //Get associated truth particle
  const xAOD::TruthParticle* truth = truthParticle( track );

  //Track with no associated truth particle are fakes
  if ( !truth ) {
    if(printCutflow) h_fake->Fill(1);
    return true;
  }

  //All tracks with low truth math probability are fakes
  TruthMatchProbabilityCut truthMatchProbabilityCut;
  if( !truthMatchProbabilityCut.accept(track, NULL) ) {
    if(printCutflow) h_fake->Fill(2);
    return true;
  }

  //Tracks with barcode = 0 are fakes
  if ( truth->barcode()==0 ) {
    if(printCutflow) h_fake->Fill(3);
    return true;
  }

  //Track is associated to neutral particle
  if (truth->isNeutral()){
    if(printCutflow) h_fake->Fill(4);
    return true;
  }

  return false;
}

bool TrackTruthHelpers::isOOPS( const xAOD::TrackParticle* track )
{
  // All
  if(printCutflow) h_OOPS->Fill(0);

  //Get associated truth particle
  const xAOD::TruthParticle* truth =  truthParticle( track );
  if( !truth  ){
    return false;
  }

  //Is the this a primary truth particle?

  //Check to see if its a stable particle
  if( truth->status() != 1 )
    return false;
  if(printCutflow) h_OOPS->Fill(1);

  //Clearly for tracking we don't care about Neutrals
  if( const_cast<xAOD::TruthParticle*>(truth)->isNeutral() )
    return false;
  if(printCutflow) h_OOPS->Fill(2);

  //Barcode of zero indicates there was no truth paticle found for this track
  if( truth->barcode() == 0 || truth->barcode() >= 200e3)
    return false;
  if(printCutflow) h_OOPS->Fill(3);

  //Outside acceptance?
  if( passAcceptance(truth) )
    return false;
  if(printCutflow) h_OOPS->Fill(4);

  //Particle is not a strange baryon
  if( truth->isStrangeBaryon() )
    return false;
  if(printCutflow) h_OOPS->Fill(5);

  // Don't use tracks with low truth math probability
  TruthMatchProbabilityCut truthMatchProbabilityCut;
  if( !truthMatchProbabilityCut.accept(track, NULL) )
    return false;
  if(printCutflow) h_OOPS->Fill(6);

  return true;
}

const xAOD::TruthParticle* TrackTruthHelpers::truthParticle(const xAOD::TrackParticle *track)
{
  typedef ElementLink< xAOD::TruthParticleContainer > Link_t;
  //static const char* NAME = "truthParticleLink";
  static const SG::Accessor<Link_t> mAcc_link("truthParticleLink");
  /*if( ! track->isAvailable< Link_t >( NAME ) ) {
    return 0;
  }*/
  
  //const Link_t& link = track->auxdata< Link_t >( NAME );
  const Link_t& link = mAcc_link(*track);
  if( ! link.isValid() ) {
    return 0;
  }

  return *link;
}


bool TrackTruthHelpers::passAcceptance(const xAOD::TruthParticle* truth) {
  return ( truth->pt() > m_PtCut and std::fabs(truth->eta()) < m_EtaCut );
}


void TrackTruthHelpers::printSummary() {
  int iCut = 0;
  Info("CutflowSummary", "Primary track cutflow:");
  for(std::string cutName : m_PrimaryCuts) {
    iCut += 1;
    Info("CutflowSummary", "%20s %10.0f", cutName.c_str(), h_primary->GetBinContent(iCut));
  }
  iCut = 0;
  Info("CutflowSummary", "Secondary track cutflow:");
  for(std::string cutName : m_SecondaryCuts) {
    iCut += 1;
    Info("CutflowSummary", "%20s %10.0f", cutName.c_str(), h_secondary->GetBinContent(iCut));
  }
  iCut = 0;
  Info("CutflowSummary", "Strange baryon track cutflow:");
  for(std::string cutName : m_StrangeBaryonCuts) {
    iCut += 1;
    Info("CutflowSummary", "%20s %10.0f", cutName.c_str(), h_strangeBaryon->GetBinContent(iCut));
  }
  iCut = 0;
  Info("CutflowSummary", "Fake track cutflow:");
  for(std::string cutName : m_FakeCuts) {
    iCut += 1;
    Info("CutflowSummary", "%20s %10.0f", cutName.c_str(), h_fake->GetBinContent(iCut));
  }
  iCut = 0;
  Info("CutflowSummary", "OOPS track cutflow:");
  for(std::string cutName : m_OOPSCuts) {
    iCut += 1;
    Info("CutflowSummary", "%20s %10.0f", cutName.c_str(), h_OOPS->GetBinContent(iCut));
  }
}

/*
void TrackTruthHelpers::BookHistograms(std::string n, EL::Worker *worker) {
  std::string name = "Cutflow/"+n+"/";

  h_primary = new TH1D( (name+"Primary").c_str(),";; # Tracks", 7, -0.5, 6.5 );
  h_primary->Sumw2();
  worker->addOutput( h_primary );
  int iCut = 0;
  for(std::string cutName : m_PrimaryCuts) {
    iCut += 1;
    h_primary->GetXaxis()->SetBinLabel(iCut, cutName.c_str());
  }

  h_secondary = new TH1D( (name+"Secondary").c_str(),";; # Tracks", 6, -0.5, 5.5 );
  h_secondary->Sumw2();
  worker->addOutput( h_secondary );
  iCut = 0;
  for(std::string cutName : m_SecondaryCuts) {
    iCut += 1;
    h_secondary->GetXaxis()->SetBinLabel(iCut, cutName.c_str());
  }

  h_strangeBaryon = new TH1D( (name+"StrangeBaryon").c_str(),";; # Tracks", 7, -0.5, 6.5 );
  h_strangeBaryon->Sumw2();
  worker->addOutput( h_strangeBaryon );
  iCut = 0;
  for(std::string cutName : m_StrangeBaryonCuts) {
    iCut += 1;
    h_strangeBaryon->GetXaxis()->SetBinLabel(iCut, cutName.c_str());
  }

  h_fake = new TH1D( (name+"Fake").c_str(),";; # Tracks", 5, -0.5, 4.5 );
  h_fake->Sumw2();
  worker->addOutput( h_fake );
  iCut = 0;
  for(std::string cutName : m_FakeCuts) {
    iCut += 1;
    h_fake->GetXaxis()->SetBinLabel(iCut, cutName.c_str());
  }

  h_OOPS = new TH1D( (name+"OOPS").c_str(),";; # Tracks", 7, -0.5, 6.5 );
  h_OOPS->Sumw2();
  worker->addOutput( h_OOPS );
  iCut = 0;
  for(std::string cutName : m_OOPSCuts) {
    iCut += 1;
    h_OOPS->GetXaxis()->SetBinLabel(iCut, cutName.c_str());
  }
}
*/


void TrackTruthHelpers::fillCutflow( const xAOD::TrackParticle* track ) {
  isPrimary(track);
  isSecondary(track);
  isStrangeBaryon(track);
  isFake(track);
  isOOPS(track);
}
