/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkTrackClusterAssValidation.h"


#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "InDetPrepRawData/SCT_ClusterContainer.h"
#include "InDetPrepRawData/PixelClusterContainer.h"

#include "StoreGate/ReadHandle.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"
#include "TruthUtils/HepMCHelpers.h"


///////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////

ITk::TrackClusterAssValidation::TrackClusterAssValidation
(const std::string& name,ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name,pSvcLocator)
{
}

ITk::TrackClusterAssValidation::~TrackClusterAssValidation() =default;


///////////////////////////////////////////////////////////////////
// Initialisation
///////////////////////////////////////////////////////////////////

StatusCode ITk::TrackClusterAssValidation::initialize()
{

  StatusCode sc;

  if (m_rapcut == 0) {
    m_tcut = 0;
  }
  else {
    double den = tan(2.*atan(exp(-m_rapcut)));
    if (den > 0) {
      m_tcut = 1./den;
    }
    else {
      m_tcut = std::numeric_limits<double>::max();
    }
  }

  // Erase statistic information
  //
  m_pdg          = std::abs(m_pdg)         ;

  m_trackCollectionStat.resize(m_tracklocation.size());
  m_eventStat = InDet::EventStat_t();

  // Read Handle Key
  ATH_CHECK(m_truth_locationStrip.initialize(m_useStrip));
  ATH_CHECK(m_clustersStripname.initialize(m_useStrip));
  ATH_CHECK(m_spacepointsStripname.initialize(m_useStrip));

  ATH_CHECK( m_clustersPixelname.initialize(m_usePix));
  ATH_CHECK( m_spacepointsPixelname.initialize(m_usePix));
  ATH_CHECK( m_truth_locationPixel.initialize(m_usePix));

  ATH_CHECK( m_spacepointsOverlapname.initialize(m_useStrip));

  ATH_CHECK( m_tracklocation.initialize());

  // Read Cond Handle Key
  ATH_CHECK(m_pixelDetEleCollKey.initialize());
  ATH_CHECK(m_StripDetEleCollKey.initialize());

  if (msgLvl(MSG::DEBUG)) {
    dumptools(msg(),MSG::DEBUG);
    msg() << endmsg;
  }

  return sc;
}

///////////////////////////////////////////////////////////////////
// Execute
///////////////////////////////////////////////////////////////////

StatusCode ITk::TrackClusterAssValidation::execute(const EventContext& ctx) const
{

  if(!m_usePix && !m_useStrip) return StatusCode::SUCCESS;
  EventData_t event_data(m_tracklocation.size() );

  std::vector<SG::ReadHandle<PRD_MultiTruthCollection> > read_handle;
  read_handle.reserve(3);
  if(m_usePix) {
    read_handle.emplace_back(m_truth_locationPixel,ctx);
    if (not read_handle.back().isValid()) {
      ATH_MSG_FATAL( "Could not find TruthPix" );
      return StatusCode::FAILURE;
    }
    event_data.m_truthPix = &(*read_handle.back());
  }

  if(m_useStrip) {
    read_handle.emplace_back(m_truth_locationStrip,ctx);
    if (not read_handle.back().isValid()) {
      ATH_MSG_FATAL( "Could not find TruthStrip" );
      return StatusCode::FAILURE;
    }
    event_data.m_truthStrip = &(*read_handle.back());
  }

  newClustersEvent      (ctx,event_data);
  newSpacePointsEvent   (ctx,event_data);
  event_data.m_nqtracks = qualityTracksSelection(event_data);
  tracksComparison      (ctx,event_data);
  if(!event_data.m_particles[0].empty()) {

    efficiencyReconstruction(event_data);
    if(msgLvl(MSG::DEBUG)) noReconstructedParticles(event_data);

  }

  {
    std::lock_guard<std::mutex> lock(m_statMutex);
    assert( event_data.m_trackCollectionStat.size() == m_trackCollectionStat.size());
    for (unsigned int i=0; i< m_trackCollectionStat.size(); ++i ) {
      m_trackCollectionStat[i] += event_data.m_trackCollectionStat[i];
    }
    m_eventStat += event_data.m_eventStat;
  }

  if (msgLvl(MSG::DEBUG)) {
    dumpevent(msg(),event_data);
    msg() << endmsg;
  }
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////
// Finalize
///////////////////////////////////////////////////////////////////

StatusCode ITk::TrackClusterAssValidation::finalize() {
  if(m_eventStat.m_events<=0) return StatusCode::SUCCESS;
  const auto & w13 = std::setw(13);
  const auto & p5 = std::setprecision(5);
  const auto topNtail=[](const std::string & str){return "|" + str + "|";};
  const std::string lineSeparator(83,'-');
  const std::string spaceSeparator(83,' ');
  std::stringstream out;
  out<<std::fixed;
  out<<topNtail(lineSeparator)<<"\n";
  out<<"|        TrackClusterAssValidation statistic for charge truth particles with        |\n";
  out<<topNtail(spaceSeparator)<<"\n";

  out<<"|        eta bins for eta dependent variables = [0.0, ";
  for (unsigned int etabin = 1; etabin<(m_etabins.size()-1); etabin++)
    out << std::setw(2) << std::setprecision(1) << m_etabins.value().at(etabin) << ", ";
  out << std::setw(2) << m_etabins.value().back() << "]                |\n";
  out<<"|                    eta dependent pT [MeV]  >= [";
  for (unsigned int ptbin = 0; ptbin<(m_ptcuts.size()-1); ptbin++)
    out<<std::setw(6)<<std::setprecision(2)<<m_ptcuts.value().at(ptbin)<<", ";
  out<<std::setw(6)<<std::setprecision(2)<<m_ptcuts.value().back()<<"]            |\n";
  if(m_ptcutmax < 1000000.) {
    out<<"|                    pT                      <="<<w13<<p5<<m_ptcutmax<<" MeV"<<"                    |\n";
  }
  out<<"|                    |rapidity|              <="<<w13<<p5<<m_rapcut<<"                        |\n";
  out<<"|                    max vertex radius       <="<<w13<<p5<<m_rmax<<" mm"<<"                     |\n";
  out<<"|                    min vertex radius       >="<<w13<<p5<<m_rmin<<" mm"<<"                     |\n";
  out<<"|                    particles pdg            ="<<std::setw(8)<<m_pdg<<"                             |\n";
  
  auto yesNo=[](const bool predicate){return predicate ? "yes" : "no ";};
  out<<"|                    use Pixels information          "<<yesNo(m_usePix)<<"                            |\n";
  out<<"|                    use Strip  information          "<<yesNo(m_useStrip)<<"                            |\n";
  out<<"|                    take into account outliers      "<<yesNo(m_useOutliers)<<"                            |\n";
  out<<topNtail(spaceSeparator)<<"\n";
  if(!m_usePix && !m_useStrip) return StatusCode::SUCCESS;
  enum Regions{Barrel, Transition, Endcap, Forward, NRegions};
  auto incrementArray=[](auto & array, const int idx){for (int j{};j!=NRegions;++j) array[idx][j] += array[idx+1][j];};
  for(int i=48; i>=0; --i) {
    m_eventStat.m_particleClusters      [i]   +=m_eventStat.m_particleClusters      [i+1];
    incrementArray(m_eventStat.m_particleClustersBTE, i);
    m_eventStat.m_particleSpacePoints   [i]   +=m_eventStat.m_particleSpacePoints   [i+1];
    incrementArray(m_eventStat.m_particleSpacePointsBTE, i);
  }
  auto coerceToOne=[](const double & initialVal)->double {return (initialVal<1.) ? 1. : initialVal; };
  /* Note that in all cases below, the dimension of the target (i.e. result) array is 10
   * whereas the dimension of the source, or input, array is 50. The first index of the source
   * is used for totals, to act as a denominator(coerced to 1 if necessary). Bin 49 is used for overflows.
   * Stats for single clusters (index 1) appear to be unused in printout.
  */
  //all
  double pa   =  coerceToOne(m_eventStat.m_particleClusters[0]); 
  std::array<double, 10> pc2ff{};
  size_t clusterIdx = 2;
  for (auto & thisCluster: pc2ff){
    thisCluster = double(m_eventStat.m_particleClusters[ clusterIdx++ ])/ pa;
  }
  //barrel
  pa           = coerceToOne(m_eventStat.m_particleClustersBTE[0][Barrel]); 
  std::array<double, 10> pcBarrel2ff{};
  size_t clusterBarrelIdx = 2;
  for (auto & thisClusterB: pcBarrel2ff){
    thisClusterB = double(m_eventStat.m_particleClustersBTE[ clusterBarrelIdx++ ][Barrel])/ pa;
  }
  //transition
  pa           = coerceToOne(m_eventStat.m_particleClustersBTE[0][Transition]); 
  std::array<double, 10> pcTransition2ff{};
  size_t clusterTransitionIdx = 2;
  for (auto & thisClusterT: pcTransition2ff){
    thisClusterT = double(m_eventStat.m_particleClustersBTE[ clusterTransitionIdx++ ][Transition])/ pa;
  }
  //endcap
  pa           = coerceToOne(m_eventStat.m_particleClustersBTE[0][Endcap]); 
  std::array<double, 10> pcEndcap2ff{};
  size_t clusterEndcapIdx = 2;
  for (auto & thisClusterE: pcEndcap2ff){
    thisClusterE = double(m_eventStat.m_particleClustersBTE[ clusterEndcapIdx++ ][Endcap])/ pa;
  }
  //fwd
  pa           = coerceToOne(m_eventStat.m_particleClustersBTE[0][3]); 
  std::array<double, 10> pcFwd2ff{};
  size_t clusterFwdIdx = 2;
  for (auto & thisClusterD: pcFwd2ff){
    thisClusterD = double(m_eventStat.m_particleClustersBTE[ clusterFwdIdx++ ][Forward])/ pa;
  }
  //
  //*** SPACE POINTS ***
  //
  //all
  pa   =  coerceToOne(m_eventStat.m_particleSpacePoints[0]); 
  std::array<double, 10> sp2ff{};
  size_t spacepointIdx = 2;
  for (auto & thisSpacepoint: sp2ff){
    thisSpacepoint = double(m_eventStat.m_particleSpacePoints[ spacepointIdx++ ])/ pa;
  }
  //barrel
  pa           = coerceToOne(m_eventStat.m_particleSpacePointsBTE[0][Barrel]); 
  std::array<double, 10> spBarrel2ff{};
  size_t spacepointBarrelIdx = 2;
  for (auto & thisSpacepoint: spBarrel2ff){
    thisSpacepoint = double(m_eventStat.m_particleSpacePointsBTE[ spacepointBarrelIdx++ ][Barrel])/ pa;
  }
  //transition
  pa           = coerceToOne(m_eventStat.m_particleSpacePointsBTE[0][Transition]); 
  std::array<double, 10> spTransition2ff{};
  size_t spacepointTransitionIdx = 2;
  for (auto & thisSpacepoint: spTransition2ff){
    thisSpacepoint = double(m_eventStat.m_particleSpacePointsBTE[ spacepointTransitionIdx++ ][Transition])/ pa;
  }
  //endcap
  pa           = coerceToOne(m_eventStat.m_particleSpacePointsBTE[0][Endcap]); 
  std::array<double, 10> spEndcap2ff{};
  size_t spacepointEndcapIdx = 2;
  for (auto & thisSpacepoint: spEndcap2ff){
    thisSpacepoint = double(m_eventStat.m_particleSpacePointsBTE[ spacepointEndcapIdx++ ][Endcap])/ pa;
  }
  //Fwd
  pa           = coerceToOne(m_eventStat.m_particleSpacePointsBTE[0][Forward]); 
  std::array<double, 10> spFwd2ff{};
  size_t spacepointFwdIdx = 2;
  for (auto & thisSpacepoint: spFwd2ff){
    thisSpacepoint = double(m_eventStat.m_particleSpacePointsBTE[ spacepointFwdIdx++ ][Forward])/ pa;
  }
  auto w8=std::setw(8);
  out<<"|         Probability for such charge particles to have some number silicon                          |\n";
  out<<"|                     clusters                         |             space points                        |\n";
  out<<"|           Total   Barrel  Transi  Endcap   Forward   |  Total   Barrel  Transi  Endcap   Forward       |\n";

   for (size_t idx{0};idx != 10;++idx){
     out<<"|  >= "<<idx+2<< std::string((idx<8)?"  ":" ")
       <<w8<<p5<<pc2ff[idx]
       <<w8<<p5<<pcBarrel2ff[idx]
       <<w8<<p5<<pcTransition2ff[idx]
       <<w8<<p5<<pcEndcap2ff[idx]
       <<w8<<p5<<pcFwd2ff[idx]<<"  |  "

       <<w8<<p5<<sp2ff[idx]
       <<w8<<p5<<spBarrel2ff[idx]
       <<w8<<p5<<spTransition2ff[idx]
       <<w8<<p5<<spEndcap2ff[idx]
       <<w8<<p5<<spFwd2ff[idx]
       <<"       |\n";
   }

  out<<topNtail(spaceSeparator)<<"\n";
  out<<"|               Additional cuts for truth particles are                             |\n";
  out<<"|   eta dependent number of silicon clusters >= [";
  for (unsigned int clbin = 0; clbin<(m_clcuts.size()-1); clbin++)
    out<<std::setw(2)<<m_clcuts.value().at(clbin)<<", ";
  out<<std::setw(2)<<m_clcuts.value().back()<<"]                        |\n";
  out<<"|                    number  space    points >="<<w13<<m_spcut.value()<<"                        |\n";

  pa  = coerceToOne(m_eventStat.m_particleClusters[0]); 
  out<<"|           Probability find truth particles with this cuts is "<<w8<<p5<<double(m_eventStat.m_events)/pa<<"             |\n";
  pa  = coerceToOne(m_eventStat.m_particleClustersBTE[0][Barrel]); 
  out<<"|                                        For barrel     region "<<w8<<p5<<double(m_eventStat.m_eventsBTE[Barrel])/pa<<"             |\n";
  pa  = coerceToOne(m_eventStat.m_particleClustersBTE[0][Transition]);
  out<<"|                                        For transition region "<<w8<<p5<<double(m_eventStat.m_eventsBTE[Transition])/pa<<"             |\n";
  pa  = coerceToOne(m_eventStat.m_particleClustersBTE[0][Endcap]); 
  out<<"|                                        For endcap     region "<<w8<<p5<<double(m_eventStat.m_eventsBTE[Endcap])/pa<<"             |\n";
  pa  = coerceToOne(m_eventStat.m_particleClustersBTE[0][Forward]); 
  out<<"|                                        For forward    region "<<w8<<p5<<double(m_eventStat.m_eventsBTE[Forward])/pa<<"             |\n";
  out<<"|                                                                                   |\n";
  pa            = coerceToOne(m_eventStat.m_nclustersNegBP); 
  double ratio  = double(m_eventStat.m_nclustersPosBP)/pa;
  double eratio = std::sqrt(ratio*(1.+ratio)/pa);
  out<<"|      Ratio barrel pixels clusters for +/- particles ="<<w8<<p5<<ratio<<" +-"<<w8<<p5<<eratio<<"          |\n";
  pa            = coerceToOne(m_eventStat.m_nclustersNegEP); 
  ratio         = double(m_eventStat.m_nclustersPosEP)/pa;
  eratio        = std::sqrt(ratio*(1.+ratio)/pa);
  out<<"|      Ratio endcap pixels clusters for +/- particles ="<<w8<<p5<<ratio<<" +-"<<w8<<p5<<eratio<<"          |\n";
  pa            = coerceToOne(m_eventStat.m_nclustersNegBS);
  ratio         = double(m_eventStat.m_nclustersPosBS)/pa;
  eratio        = std::sqrt(ratio*(1.+ratio)/pa);
  out<<"|      Ratio barrel   Strip  clusters for +/- particles ="<<w8<<p5<<ratio<<" +-"<<w8<<p5<<eratio<<"          |\n";
  pa            = coerceToOne(m_eventStat.m_nclustersNegES);
  ratio         = double(m_eventStat.m_nclustersPosES)/pa;
  eratio        = std::sqrt(ratio*(1.+ratio)/pa);
  out<<"|      Ratio endcap   Strip  clusters for +/- particles ="<<w8<<p5<<ratio<<" +-"<<w8<<p5<<eratio<<"          |\n";
  pa            = double(m_eventStat.m_eventsNEG);      if(pa < 1.) pa = 1.;
  ratio         = double(m_eventStat.m_eventsPOS)/pa;
  eratio        = std::sqrt(ratio*(1.+ratio)/pa);
  out<<"|      Number truth particles and +/- ratio ="<<std::setw(10)<<m_eventStat.m_events<<w8<<p5<<ratio<<" +-"<<w8<<p5<<eratio<<"          |\n";
	ratio = 0.;
	if(m_eventStat.m_nclustersPTOT!=0) ratio = double(m_eventStat.m_nclustersPTOTt)/double(m_eventStat.m_nclustersPTOT);

  out<<"| Number pix clusters, truth clusters and ratio = "
	   <<std::setw(10)<<m_eventStat.m_nclustersPTOT
	   <<std::setw(10)<<m_eventStat.m_nclustersPTOTt
	   <<std::setw(12)<<std::setprecision(5)<<ratio<<"  |\n";
  ratio = 0.;
  if(m_eventStat.m_nclustersSTOT!=0) ratio = double(m_eventStat.m_nclustersSTOTt)/double(m_eventStat.m_nclustersSTOT);
   out<<"| Number strip clusters, truth clusters and ratio = "
	   <<std::setw(10)<<m_eventStat.m_nclustersSTOT
	   <<std::setw(10)<<m_eventStat.m_nclustersSTOTt
	   <<std::setw(12)<<std::setprecision(5)<<ratio<<"  |\n";
	   
  out<<"|-----------------------------------------------------------------------------------|\n\n";

  SG::ReadHandleKeyArray<TrackCollection>::const_iterator t=m_tracklocation.begin(),te=m_tracklocation.end();
  int nc = 0;
  for(; t!=te; ++t) {
    int   n     = 47-(t->key().size());
    std::string s1; for(int i=0; i<n; ++i) s1.append(" "); s1.append("|");


    out<<"|-----------------------------------------------------------------------------------|\n";
    out<<"|                      Statistic for "<<(t->key())<<s1<<"\n";

    double ne = double(m_eventStat.m_events);  if(ne < 1.) ne = 1.;
    double ef [6]; for(int i=0; i!=6; ++i) ef [i] = double(m_trackCollectionStat[nc].m_efficiency   [i])   /ne;
    double ef0[6]; for(int i=0; i!=6; ++i) ef0[i] = double(m_trackCollectionStat[nc].m_efficiencyN  [i][0])/ne;
    double ef1[6]; for(int i=0; i!=6; ++i) ef1[i] = double(m_trackCollectionStat[nc].m_efficiencyN  [i][1])/ne;
    double ef2[6]; for(int i=0; i!=6; ++i) ef2[i] = double(m_trackCollectionStat[nc].m_efficiencyN  [i][2])/ne;


    using EffArray_t = std::array<double, 6>;
    //
    auto makeEffArray = [](const auto & threeDimArray, const size_t secondIdx, const size_t thirdIdx, const double denom){
      EffArray_t result{};
      size_t idx{0};
      auto invDenom = 1./denom;
      for (auto & entry: result){
        entry = threeDimArray[idx++][secondIdx][thirdIdx]*invDenom;
      }
      return result;
    };
    //
    const auto & efficiencyArrayInput = m_trackCollectionStat[nc].m_efficiencyBTE;
    //
    double neBTE = coerceToOne(m_eventStat.m_eventsBTE[Barrel]);
    const EffArray_t efB0 = makeEffArray(efficiencyArrayInput,0,Barrel,neBTE);
    const EffArray_t efB1 = makeEffArray(efficiencyArrayInput,1,Barrel,neBTE);
    const EffArray_t efB2 = makeEffArray(efficiencyArrayInput,2,Barrel,neBTE);
    const EffArray_t efB3 = makeEffArray(efficiencyArrayInput,3,Barrel,neBTE);
    const EffArray_t efB4 = makeEffArray(efficiencyArrayInput,4,Barrel,neBTE);
    //
    neBTE = coerceToOne(m_eventStat.m_eventsBTE[Transition]);
    const EffArray_t efT0 = makeEffArray(efficiencyArrayInput,0,Transition,neBTE);
    const EffArray_t efT1 = makeEffArray(efficiencyArrayInput,1,Transition,neBTE);
    const EffArray_t efT2 = makeEffArray(efficiencyArrayInput,2,Transition,neBTE);
    const EffArray_t efT3 = makeEffArray(efficiencyArrayInput,3,Transition,neBTE);
    const EffArray_t efT4 = makeEffArray(efficiencyArrayInput,4,Transition,neBTE);
    //
    neBTE = coerceToOne(m_eventStat.m_eventsBTE[Endcap]);
    const EffArray_t efE0 = makeEffArray(efficiencyArrayInput,0,Endcap,neBTE);
    const EffArray_t efE1 = makeEffArray(efficiencyArrayInput,1,Endcap,neBTE);
    const EffArray_t efE2 = makeEffArray(efficiencyArrayInput,2,Endcap,neBTE);
    const EffArray_t efE3 = makeEffArray(efficiencyArrayInput,3,Endcap,neBTE);
    const EffArray_t efE4 = makeEffArray(efficiencyArrayInput,4,Endcap,neBTE);
    //
    neBTE = coerceToOne(m_eventStat.m_eventsBTE[Forward]);
    const EffArray_t efD0 = makeEffArray(efficiencyArrayInput,0,Forward,neBTE);
    const EffArray_t efD1 = makeEffArray(efficiencyArrayInput,1,Forward,neBTE);
    const EffArray_t efD2 = makeEffArray(efficiencyArrayInput,2,Forward,neBTE);
    const EffArray_t efD3 = makeEffArray(efficiencyArrayInput,3,Forward,neBTE);
    const EffArray_t efD4 = makeEffArray(efficiencyArrayInput,4,Forward,neBTE);


    double efrec  = ef0[0]+ef0[1]+ef0[2]+ef1[0]+ef1[1]+ef2[0];
    double efrecB = efB0[0]+efB0[1]+efB0[2]+efB1[0]+efB1[1]+efB2[0];
    double efrecT = efT0[0]+efT0[1]+efT0[2]+efT1[0]+efT1[1]+efT2[0];
    double efrecE = efE0[0]+efE0[1]+efE0[2]+efE1[0]+efE1[1]+efE2[0];
    double efrecD = efD0[0]+efD0[1]+efD0[2]+efD1[0]+efD1[1]+efD2[0];

    ne        = coerceToOne(m_eventStat.m_eventsPOS); 
    double efP[6]; for(int i=0; i!=6; ++i) efP[i] = double(m_trackCollectionStat[nc].m_efficiencyPOS[i])/ne;
    ne        = coerceToOne(m_eventStat.m_eventsNEG);
    double efN[6]; for(int i=0; i!=6; ++i) efN[i] = double(m_trackCollectionStat[nc].m_efficiencyNEG[i])/ne;

    out<<"|-----------------------------------------------------------------------------------|\n";
    out<<"| Probability to lose       0        1        2        3        4    >=5 clusters   |\n";
    out<<"|-----------------------------------------------------------------------------------|\n";
    
    auto formattedOutput=[&out](auto & effArray){ 
      for (size_t i{};i!=6;++i){
        out<<std::setw(9)<<std::setprecision(4)<<effArray[i];
      }
      out<<"        |\n";
    };
    
    out<<"| For all particles   ";
	  formattedOutput(ef);
    out<<"| For  +  particles   ";
	  formattedOutput(efP);
    out<<"| For  -  particles   ";
	  formattedOutput(efN);
    out<<"|-----------------------------------------------------------------------------------|\n";
    out<<"| Barrel region                                                                     |\n";
    out<<"|   0 wrong clusters  ";
	  formattedOutput(efB0);
    out<<"|   1 wrong clusters  ";
	  formattedOutput(efB1);
    out<<"|   2 wrong clusters  ";
	  formattedOutput(efB2);
    out<<"|   3 wrong clusters  ";
	  formattedOutput(efB3);
    out<<"| >=4 wrong clusters  ";
	  formattedOutput(efB4);
    out<<"|-----------------------------------------------------------------------------------|\n";
    out<<"| Transition region                                                                 |\n";
    out<<"|   0 wrong clusters  ";
	  formattedOutput(efT0);
    out<<"|   1 wrong clusters  ";
	  formattedOutput(efT1);
    out<<"|   2 wrong clusters  ";
	  formattedOutput(efT2);
    out<<"|   3 wrong clusters  ";
	  formattedOutput(efT3);
    out<<"| >=4 wrong clusters  ";
	  formattedOutput(efT4);
    out<<"|-----------------------------------------------------------------------------------|\n";
    out<<"| Endcap region                                                                     |\n";
    out<<"|   0 wrong clusters  ";
    formattedOutput(efE0);
    out<<"|   1 wrong clusters  ";
	  formattedOutput(efE1);
    out<<"|   2 wrong clusters  ";
	  formattedOutput(efE2);
    out<<"|   3 wrong clusters  ";
	  formattedOutput(efE3);
    out<<"| >=4 wrong clusters  ";
	  formattedOutput(efE4);
    out<<"|-----------------------------------------------------------------------------------|\n";
    out<<"| Forward region                                                                    |\n";
    out<<"|   0 wrong clusters  ";
    formattedOutput(efD0);
    out<<"|   1 wrong clusters  ";
    formattedOutput(efD1);
    out<<"|   2 wrong clusters  ";
    formattedOutput(efD2);
    out<<"|   3 wrong clusters  ";
    formattedOutput(efD3);
    out<<"| >=4 wrong clusters  ";
    formattedOutput(efD4);

   out<<"|-----------------------------------------------------------------------------------|\n";
   pa  = coerceToOne(m_eventStat.m_particleClusters[0]);
   out<<"| Efficiency reconstruction (number lose+wrong < 3) = "
	    <<std::setw(9)<<std::setprecision(5)<<efrec
	    <<" ("
	    <<std::setw(9)<<std::setprecision(5)<<efrec*double(m_eventStat.m_events)/pa
	    <<" ) "
	    <<"       |\n";
   pa  = coerceToOne(m_eventStat.m_particleClustersBTE[0][Barrel]);
   out<<"|                             For barrel     region = "
	    <<std::setw(9)<<std::setprecision(5)<<efrecB
	    <<" ("
	    <<std::setw(9)<<std::setprecision(5)<<efrecB*double(m_eventStat.m_eventsBTE[Barrel])/pa
	    <<" ) "
	    <<"       |\n";
   pa  = coerceToOne(m_eventStat.m_particleClustersBTE[0][Transition]); 
   out<<"|                             For transition region = "
	    <<std::setw(9)<<std::setprecision(5)<<efrecT
	    <<" ("
	    <<std::setw(9)<<std::setprecision(5)<<efrecT*double(m_eventStat.m_eventsBTE[Transition])/pa
	    <<" ) "
	    <<"       |\n";
   pa  = coerceToOne(m_eventStat.m_particleClustersBTE[0][Endcap]); 
   out<<"|                             For endcap     region = "
	    <<std::setw(9)<<std::setprecision(5)<<efrecE
	    <<" ("
	    <<std::setw(9)<<std::setprecision(5)<<efrecE*double(m_eventStat.m_eventsBTE[Endcap])/pa
	    <<" ) "
	    <<"       |\n";
   pa  = coerceToOne(m_eventStat.m_particleClustersBTE[0][Forward]); 
   out<<"|                             For forward    region = "
            <<std::setw(9)<<std::setprecision(5)<<efrecD
            <<" ("
            <<std::setw(9)<<std::setprecision(5)<<efrecD*double(m_eventStat.m_eventsBTE[Forward])/pa
            <<" ) "
            <<"       |\n";

    out<<"|-----------------------------------------------------------------------------------|\n";
    out<<"| Reconstructed tracks         +          -    +/-ratio     error                   |\n";
    out<<"|-----------------------------------------------------------------------------------|\n";

    pa     = coerceToOne(m_trackCollectionStat[nc].m_ntracksNEGB); 
    ratio  = double(m_trackCollectionStat[nc].m_ntracksPOSB)/pa;
    eratio = std::sqrt(ratio*(1.+ratio)/pa);

    out<<"| Barrel               "
	     <<std::setw(10)<<m_trackCollectionStat[nc].m_ntracksPOSB
	     <<std::setw(11)<<m_trackCollectionStat[nc].m_ntracksNEGB
	     <<std::setw(11)<<std::setprecision(5)<<ratio
	     <<std::setw(11)<<std::setprecision(5)<<eratio<<"                  |\n";
    pa     = coerceToOne(m_trackCollectionStat[nc].m_ntracksNEGE); 
    ratio  = double(m_trackCollectionStat[nc].m_ntracksPOSE)/pa;
    eratio = std::sqrt(ratio*(1.+ratio)/pa);

    out<<"| Endcap               "
	     <<std::setw(10)<<m_trackCollectionStat[nc].m_ntracksPOSE
	     <<std::setw(11)<<m_trackCollectionStat[nc].m_ntracksNEGE
	     <<std::setw(11)<<std::setprecision(5)<<ratio
	     <<std::setw(11)<<std::setprecision(5)<<eratio<<"                  |\n";
    pa     = coerceToOne(m_trackCollectionStat[nc].m_ntracksNEGFWD); 
    ratio  = double(m_trackCollectionStat[nc].m_ntracksPOSFWD)/pa;
    eratio = std::sqrt(ratio*(1.+ratio)/pa);

    out<<"| Forward              "
             <<std::setw(10)<<m_trackCollectionStat[nc].m_ntracksPOSFWD
             <<std::setw(11)<<m_trackCollectionStat[nc].m_ntracksNEGFWD
             <<std::setw(11)<<std::setprecision(5)<<ratio
             <<std::setw(11)<<std::setprecision(5)<<eratio<<"                  |\n";



    int nt=0;
    int ft=0;
    int kf=0;
    for(int k = 0; k!=50; ++k) {
      nt+=m_trackCollectionStat[nc].m_total[k];
      ft+=m_trackCollectionStat[nc].m_fake [k];
      if(!kf && nt) kf = k;
    }

    if(kf) {

      out<<"|-----------------------------------------------------------------------------------|\n";
      out<<"|             Fake tracks rate for different number of clusters on track            |\n";
      out<<"|-----------------------------------------------------------------------------------|\n";

      for(int k = kf; k!=kf+6; ++k) {
	out<<"|     >= "<<std::setw(2)<<k<<"   ";
      }
      out<<"|\n";

      for(int k = kf; k!=kf+6; ++k) {
	double eff = 0.; if(nt>0) eff = double(ft)/double(nt);
	out<<"|"<<std::setw(12)<<std::setprecision(5)<<eff<<" ";
	nt-=m_trackCollectionStat[nc].m_total[k];
	ft-=m_trackCollectionStat[nc].m_fake [k];
      }
      out<<"|\n";
      out<<"|-----------------------------------------------------------------------------------|\n";
    }
    ++nc;
  }
  ATH_MSG_INFO("\n"<<out.str());
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////
// Dumps conditions information into the MsgStream
///////////////////////////////////////////////////////////////////

MsgStream& ITk::TrackClusterAssValidation::dumptools( MsgStream& out, MSG::Level level) const
{
  SG::ReadHandleKeyArray<TrackCollection>::const_iterator t=m_tracklocation.begin(),te=m_tracklocation.end();

  int n;
  out << level <<"\n";
  out<<"|--------------------------------------------------------------------------------------------------------------------|\n";
  for(; t!=te; ++t) {
    n     = 65-t->key().size();
    std::string s1; for(int i=0; i<n; ++i) s1.append(" "); s1.append("|");
    out<<"| Location of input tracks                        | "<<t->key()<<s1<<"\n";
  }
  auto padString = [](const std::string & s){
   const int n = 65 - s.size();
   return s + std::string(n, ' ');
  };
  std::string s2 = padString(m_spacepointsPixelname.key());
  std::string s3 = padString(m_spacepointsStripname.key());
  std::string s4 = padString(m_spacepointsOverlapname.key());
  std::string s5 = padString(m_clustersPixelname.key());
  std::string s6 = padString(m_clustersStripname.key());
  //
  std::string s7 = padString(m_truth_locationPixel.key());
  std::string s8 = padString(m_truth_locationStrip.key());

  out<<"| Pixel    space points                           | "<<s2<<"|\n";
  out<<"| Strip      space points                         | "<<s3<<"|\n";
  out<<"| Overlap  space points                           | "<<s4<<"|\n";
  out<<"| Pixel    clusters                               | "<<s5<<"|\n";
  out<<"| Strip      clusters                             | "<<s6<<"|\n";
  out<<"| Truth location  for pixels                      | "<<s7<<"|\n";
  out<<"| Truth location  for strips                      | "<<s8<<"|\n";
  out<<"|   max   pT cut                                  | "
     <<std::setw(14)<<std::setprecision(5)<<m_ptcutmax
     <<"                                                   |\n";
  out<<"|   rapidity cut                                  | "
     <<std::setw(14)<<std::setprecision(5)<<m_rapcut
     <<"                                                   |\n";
  out<<"| min Radius                                      | "
     <<std::setw(14)<<std::setprecision(5)<<m_rmin
     <<"                                                   |\n";
  out<<"| max Radius                                      | "
     <<std::setw(14)<<std::setprecision(5)<<m_rmax
     <<"                                                   |\n";
  out<<"| Min. number sp.points     for generated track   | "
     <<std::setw(14)<<std::setprecision(5)<<m_spcut
     <<"                                                   |\n";
  out<<"|--------------------------------------------------------------------------------------------------------------------|\n";
  return out;
}

///////////////////////////////////////////////////////////////////
// Dumps event information into the ostream
///////////////////////////////////////////////////////////////////

MsgStream& ITk::TrackClusterAssValidation::dumpevent( MsgStream& out, const ITk::TrackClusterAssValidation::EventData_t &event_data ) 
{
  out << MSG::DEBUG << "\n";
  auto formatOutput = [&out](const auto val){
    out<<std::setw(12)<<val
     <<"                              |\n";
  };
  out<<"|---------------------------------------------------------------------|\n";
  out<<"| m_nspacepoints          | ";
  formatOutput(event_data.m_nspacepoints);
  out<<"| m_nclusters             | ";
  formatOutput(event_data.m_nclusters);
  out<<"| Kine-Clusters    size   | ";
  formatOutput(event_data.m_kinecluster.size());
  out<<"| Kine-SpacePoints size   | ";
  formatOutput(event_data.m_kinespacepoint.size());
  out<<"| Number good kine tracks | ";
  formatOutput(event_data.m_nqtracks);
  out<<"|---------------------------------------------------------------------|\n";
  return out;
}


///////////////////////////////////////////////////////////////////
// New event for clusters information
///////////////////////////////////////////////////////////////////

void ITk::TrackClusterAssValidation::newClustersEvent(const EventContext& ctx,ITk::TrackClusterAssValidation::EventData_t &event_data) const
{
  std::lock_guard<std::mutex> lock(m_statMutex);

  // Get pixel clusters container
  //
  std::unique_ptr<SG::ReadHandle<InDet::SiClusterContainer> >       pixelcontainer;
  std::unique_ptr<SG::ReadHandle<InDet::SiClusterContainer> >       stripcontainer;

  if(m_usePix) {
    pixelcontainer = std::make_unique<SG::ReadHandle<InDet::SiClusterContainer> >(m_clustersPixelname,ctx);
    if (!pixelcontainer->isValid()) ATH_MSG_DEBUG("Failed to create Pixel clusters container read handle with key " << m_clustersPixelname.key());
  }

  // Get strip  clusters container
  //
  if(m_useStrip) {
    stripcontainer = std::make_unique<SG::ReadHandle<InDet::SiClusterContainer> >(m_clustersStripname,ctx);
    if (!stripcontainer->isValid()) ATH_MSG_DEBUG("Failed to create Strip clusters container read handle with key " << m_clustersStripname.key());
  }

  int Kine[1000];

  event_data.m_clusterHandles.reserve(3);
  // Loop through all pixel clusters
  //
  if(pixelcontainer && pixelcontainer->isValid()) {
    InDet::SiClusterContainer::const_iterator w  =  (*pixelcontainer)->begin();
    InDet::SiClusterContainer::const_iterator we =  (*pixelcontainer)->end  ();

    for(; w!=we; ++w) {

      InDet::SiClusterCollection::const_iterator c  = (*w)->begin();
      InDet::SiClusterCollection::const_iterator ce = (*w)->end  ();

      for(; c!=ce; ++c) {

	++event_data.m_nclusters;
	++m_eventStat.m_nclustersPTOT;
	if(isTruth(event_data,(*c))) ++m_eventStat.m_nclustersPTOTt;  


	int nk = kine(event_data,(*c),Kine,999);
	for(int i=0; i!=nk; ++i) {
	  if(!isTheSameDetElement(event_data,Kine[i],(*c))) {
	    event_data.m_kinecluster.insert(std::make_pair(Kine[i],(*c)));
	  }
	}
      }
    }
    event_data.m_clusterHandles.push_back(std::move(pixelcontainer));

  }

  // Loop through all strip clusters
  //
  if(stripcontainer && stripcontainer->isValid()) {
    InDet::SiClusterContainer::const_iterator w  =  (*stripcontainer)->begin();
    InDet::SiClusterContainer::const_iterator we =  (*stripcontainer)->end  ();

    for(; w!=we; ++w) {

      InDet::SiClusterCollection::const_iterator c  = (*w)->begin();
      InDet::SiClusterCollection::const_iterator ce = (*w)->end  ();

      for(; c!=ce; ++c) {

	++event_data.m_nclusters;
	++m_eventStat.m_nclustersSTOT;
	if(isTruth(event_data,(*c))) ++m_eventStat.m_nclustersSTOTt;	
	
	int nk = kine(event_data,(*c),Kine,999);
	for(int i=0; i!=nk; ++i) {
	  if(!isTheSameDetElement(event_data,Kine[i],(*c))) event_data.m_kinecluster.insert(std::make_pair(Kine[i],(*c)));
	}
      }
    }
    event_data.m_clusterHandles.push_back(std::move(stripcontainer));
  }

}

///////////////////////////////////////////////////////////////////
// New event for space points information
///////////////////////////////////////////////////////////////////

void ITk::TrackClusterAssValidation::newSpacePointsEvent(const EventContext& ctx, ITk::TrackClusterAssValidation::EventData_t &event_data) const
{

  int Kine[1000];

  if(m_usePix && !m_spacepointsPixelname.key().empty()) {
    event_data.m_spacePointContainer.emplace_back(m_spacepointsPixelname,ctx);
    if (!event_data.m_spacePointContainer.back().isValid()) {
      ATH_MSG_DEBUG( "Invalid Pixels space points container read handle for key " << m_spacepointsPixelname.key()  );
    }
    else  {
      SpacePointContainer::const_iterator spc  =  event_data.m_spacePointContainer.back()->begin();
      SpacePointContainer::const_iterator spce =  event_data.m_spacePointContainer.back()->end  ();
      for(; spc != spce; ++spc) {
        SpacePointCollection::const_iterator sp  = (*spc)->begin();
        SpacePointCollection::const_iterator spe = (*spc)->end  ();

        for(; sp != spe; ++sp) {

          ++event_data.m_nspacepoints;
          int nk = kine(event_data,(*sp)->clusterList().first,Kine,999);
          for(int i=0; i!=nk; ++i) {

            if(!isTheSameDetElement(event_data,Kine[i],(*sp))) {
              event_data.m_kinespacepoint.insert(std::make_pair(Kine[i],(*sp)));
            }
          }
        }
      }
    }
  }

  // Get strip space points containers from store gate
  //
  if(m_useStrip && !m_spacepointsStripname.key().empty()) {
    event_data.m_spacePointContainer.emplace_back(m_spacepointsStripname,ctx);
    if (!event_data.m_spacePointContainer.back().isValid()) {
      ATH_MSG_DEBUG( "Invalid Strip space points container read handle for key " << m_spacepointsStripname.key() );
    }
    else  {
      SpacePointContainer::const_iterator spc  =  event_data.m_spacePointContainer.back()->begin();
      SpacePointContainer::const_iterator spce =  event_data.m_spacePointContainer.back()->end  ();

      for(; spc != spce; ++spc) {

        SpacePointCollection::const_iterator sp  = (*spc)->begin();
        SpacePointCollection::const_iterator spe = (*spc)->end  ();

        for(; sp != spe; ++sp) {


          ++event_data.m_nspacepoints;
          int nk = kine(event_data,(*sp)->clusterList().first,(*sp)->clusterList().second,Kine,999);
          for(int i=0; i!=nk; ++i) {
            if(!isTheSameDetElement(event_data,Kine[i],(*sp))) {
              event_data.m_kinespacepoint.insert(std::make_pair(Kine[i],(*sp)));
            }
          }
        }
      }
    }
  }

  // Get strip overlap space points containers from store gate
  //
  if(m_useStrip && !m_spacepointsOverlapname.key().empty()) {
    event_data.m_spacepointsOverlap=std::make_unique< SG::ReadHandle<SpacePointOverlapCollection> >(m_spacepointsOverlapname,ctx);
    if (!event_data.m_spacepointsOverlap->isValid()) {
      ATH_MSG_DEBUG( "Invalid overlap space points container read handle for key " << m_spacepointsOverlapname.key() );
    }
    else  {
      SpacePointOverlapCollection::const_iterator sp  = (*(event_data.m_spacepointsOverlap))->begin();
      SpacePointOverlapCollection::const_iterator spe = (*(event_data.m_spacepointsOverlap))->end  ();

      for (; sp!=spe; ++sp) {

        ++event_data.m_nspacepoints;
        int nk = kine(event_data,(*sp)->clusterList().first,(*sp)->clusterList().second,Kine,999);
        for(int i=0; i!=nk; ++i) {
          if(!isTheSameDetElement(event_data,Kine[i],(*sp))) {
            event_data.m_kinespacepoint.insert(std::make_pair(Kine[i],(*sp)));
          }
        }
      }
    }
  }
}
///////////////////////////////////////////////////////////////////
// Good kine tracks  selection
///////////////////////////////////////////////////////////////////

int ITk::TrackClusterAssValidation::qualityTracksSelection(ITk::TrackClusterAssValidation::EventData_t &event_data) const
{
  std::multimap<int,const Trk::PrepRawData*>::iterator c = event_data.m_kinecluster   .begin();
  std::multimap<int,const Trk::SpacePoint*>::iterator  s = event_data.m_kinespacepoint.begin();

  if( c == event_data.m_kinecluster.end()) {
    return 0;
  }

  if( s == event_data.m_kinespacepoint.end()) {
    return 0;
  }

  std::list<int> worskine;

  int          rp = 0;
  double      eta = 0.;
  int          t  = 0;
  int          k0 = (*c).first;
  int          q0 = k0*charge(event_data,(*c),rp);
  unsigned int nc = 1         ;
  
  auto coerceTo49 = [] (const size_t idx){
   return (idx<50) ? idx : 49;
  };
  
  for(++c; c!=event_data.m_kinecluster.end(); ++c) {

    if((*c).first==k0) {++nc; continue;}
    q0 = charge(event_data,(*c),rp,eta)*k0;
    //
    const size_t clusterIdx =coerceTo49(nc);
    ++event_data.m_eventStat.m_particleClusters   [clusterIdx];
    ++event_data.m_eventStat.m_particleClustersBTE[clusterIdx][rp];
    //
    int ns = event_data.m_kinespacepoint.count(k0);
    const size_t spacepointIdx =coerceTo49(ns);
    ++event_data.m_eventStat.m_particleSpacePoints   [spacepointIdx];
    ++event_data.m_eventStat.m_particleSpacePointsBTE[spacepointIdx][rp];

    if     (nc                        < minclusters(eta)   ) worskine.push_back(k0);
    else if(event_data.m_kinespacepoint.count(k0)< m_spcut   ) worskine.push_back(k0);
    else {
      event_data.m_particles[0].push_back(InDet::PartPropCache(q0,rp)); ++t;
    }

    k0 = (*c).first;
    q0 =charge(event_data,(*c),rp,eta)*k0;
    nc = 1         ;
  }

  ++event_data.m_eventStat.m_particleClusters   [coerceTo49(nc)];
  ++event_data.m_eventStat.m_particleClustersBTE[coerceTo49(nc)][rp];
  int ns = event_data.m_kinespacepoint.count(k0);
  ++event_data.m_eventStat.m_particleSpacePoints   [coerceTo49(ns)];
  ++event_data.m_eventStat.m_particleSpacePointsBTE[coerceTo49(ns)][rp];

  if     (nc                        < minclusters(eta)   ) worskine.push_back(k0);
  else if(event_data.m_kinespacepoint.count(k0)< m_spcut   ) worskine.push_back(k0);
  else {
    event_data.m_particles[0].push_back(InDet::PartPropCache(q0,rp)); ++t;
  }
  for(auto & pThisCluster: worskine) {
    event_data.m_kinecluster   .erase(pThisCluster);
    event_data.m_kinespacepoint.erase(pThisCluster);
  }

  for(c = event_data.m_kinecluster.begin(); c!= event_data.m_kinecluster.end(); ++c) {
    const Trk::PrepRawData*
    d = (*c).second;
    const InDetDD::SiDetectorElement*
    de= dynamic_cast<const InDetDD::SiDetectorElement*>(d->detectorElement());
    if (not de) continue;
    int q  = charge(event_data,*c,rp);

    if     (q<0) {
      if(de->isBarrel()) {
	      de->isPixel() ? ++event_data.m_eventStat.m_nclustersNegBP : ++event_data.m_eventStat.m_nclustersNegBS;
      }
      else                                     {
	      de->isPixel() ? ++event_data.m_eventStat.m_nclustersNegEP : ++event_data.m_eventStat.m_nclustersNegES;
      }

    }
    else if(q>0) {
      if(de->isBarrel()) {
      	de->isPixel() ? ++event_data.m_eventStat.m_nclustersPosBP : ++event_data.m_eventStat.m_nclustersPosBS;
      }
      else                                     {
      	de->isPixel() ? ++event_data.m_eventStat.m_nclustersPosEP : ++event_data.m_eventStat.m_nclustersPosES;
      }
    }
  }



  for(const auto& p: event_data.m_particles[0]) {
    for(SG::ReadHandleKeyArray<TrackCollection>::size_type nc=1; nc<m_tracklocation.size(); ++nc) event_data.m_particles[nc].push_back(p);
  }
  return t;
}

///////////////////////////////////////////////////////////////////
// Recontructed track comparison with kine information
///////////////////////////////////////////////////////////////////

void ITk::TrackClusterAssValidation::tracksComparison(const EventContext& ctx, ITk::TrackClusterAssValidation::EventData_t &event_data) const
{
  if(!event_data.m_nqtracks) return;


  int nc = -1;
  event_data.m_trackcontainer.reserve(m_tracklocation.size());
  for(const SG::ReadHandleKey<TrackCollection> &track_key : m_tracklocation ) {
    if(++nc >= 100) return;
    event_data.m_tracks[nc].clear();

    event_data.m_trackcontainer.emplace_back(track_key,ctx );
    if (!event_data.m_trackcontainer.back().isValid()) {
      continue;
    }

    // Loop through all found tracks
    //
    TrackCollection::const_iterator t,te = event_data.m_trackcontainer.back()->end();

    int KINE[200],NKINE[200];

    for (t=event_data.m_trackcontainer.back()->begin(); t!=te; ++t) {

      Trk::TrackStates::const_iterator s = (*t)->trackStateOnSurfaces()->begin(),
                                       se = (*t)->trackStateOnSurfaces()->end();

      int  NK  = 0;
      int  NC  = 0;
      int  N0  = 0;
      int  nkm = 0;
      bool qp  = false;

      const Trk::TrackParameters* tpf = (*s)->trackParameters();  if(!tpf) continue;
      const AmgVector(5)&         Vpf = tpf ->parameters     ();
      double                      pTf = std::abs(std::sin(Vpf[3])/Vpf[4]);
      double                     etaf = std::abs(log(tan(.5*Vpf[3])));
      bool                        qTf = pTf > minpT(etaf);
      for(; s!=se; ++s) {

	if(!qp) {

	  const Trk::TrackParameters* tp = (*s)->trackParameters();

	  if(tp) {
	    qp = true;
	    const AmgVector(5)& Vp = tp->parameters();
	    double pT  = std::sin(Vp[3])/Vp[4]  ;
	    double rap = std::abs(std::log(std::tan(.5*Vp[3])));
	    double minpt = minpT(rap);
	    if     (pT >  minpt && pT <  m_ptcutmax) {
	      if     (rap <      1. ) ++event_data.m_trackCollectionStat[nc].m_ntracksPOSB;
	      else if(rap < 3.0) ++event_data.m_trackCollectionStat[nc].m_ntracksPOSE;
	      else if(rap < m_rapcut) ++event_data.m_trackCollectionStat[nc].m_ntracksPOSFWD;
	    }
	    else if(pT < -minpt && pT > -m_ptcutmax) {
	      if     (rap <      1. ) ++event_data.m_trackCollectionStat[nc].m_ntracksNEGB;
              else if(rap < 3.0) ++event_data.m_trackCollectionStat[nc].m_ntracksNEGE;
	      else if(rap < m_rapcut) ++event_data.m_trackCollectionStat[nc].m_ntracksNEGFWD;
	    }
	  }
	}

	if(!m_useOutliers && !(*s)->type(Trk::TrackStateOnSurface::Measurement)) continue;

	const Trk::MeasurementBase* mb = (*s)->measurementOnTrack();
	if(!mb) continue;

	const Trk::RIO_OnTrack*     ri = dynamic_cast<const Trk::RIO_OnTrack*>(mb);
	if(!ri) continue;

	const Trk::PrepRawData*     rd = ri->prepRawData();
	if(!rd) continue;

	const InDet::SiCluster*     si = dynamic_cast<const InDet::SiCluster*>(rd);
	if(!si) continue;

	if(!m_usePix && dynamic_cast<const InDet::PixelCluster*>(si)) continue;
	if(!m_useStrip && dynamic_cast<const InDet::SCT_Cluster*> (si)) continue;


	int Kine[1000], nk=kine0(event_data,rd,Kine,999); ++NC; if(!nk) ++N0;

	for(int k = 0; k!=nk; ++k) {

	  int n = 0;
	  for(; n!=NK; ++n) {if(Kine[k]==KINE[n]) {++NKINE[n]; break;}}
	  if(n==NK) {KINE[NK] = Kine[k]; NKINE[NK] = 1; if (NK < 200) ++NK;}
	}
	for(int n=0; n!=NK; ++n) {if(NKINE[n]>nkm) nkm = NKINE[n];}
      }

      for(int n=0; n!=NK; ++n) {
	if(NKINE[n]==nkm) {
	  int NQ = 1000*NKINE[n]+(NC-NKINE[n]);

	  event_data.m_tracks[nc].insert(std::make_pair(KINE[n],NQ));
	  if(qTf) {
	    if(NC-N0 > 2) {
	      ++event_data.m_trackCollectionStat[nc].m_total[NC]; if(NC-NKINE[n] > 2) {++event_data.m_trackCollectionStat[nc].m_fake[NC];}
	    }
	  }
	}
      }
    }

  }
}

///////////////////////////////////////////////////////////////////
// Particles and reconstructed tracks comparision
///////////////////////////////////////////////////////////////////

void ITk::TrackClusterAssValidation::efficiencyReconstruction(ITk::TrackClusterAssValidation::EventData_t &event_data) const
{
  for(SG::ReadHandleKeyArray<TrackCollection>::size_type nc = 0; nc!=m_tracklocation.size(); ++nc) {

    event_data.m_difference[nc].clear();
    auto p = event_data.m_particles[nc].begin(), pe =event_data.m_particles[nc].end();
    if(p==pe) return;
    std::multimap<int,int>::const_iterator t, te = event_data.m_tracks[nc].end();

    while (p!=pe) {

      const int uniqueID = HepMC::uniqueID(*p);
      int n = event_data.m_kinecluster.count(uniqueID);
      int m = 0;
      int w = 0;
      t = event_data.m_tracks[nc].find(uniqueID);
      for(; t!=te; ++t) {
	if((*t).first!=uniqueID) break;
	int ts = (*t).second/1000;
	int ws = (*t).second%1000;
	if     (ts > m         ) {m = ts; w = ws;}
	else if(ts==m && w > ws) {        w = ws;}
      }
      int d = n-m; if(d<0) d = 0; else if(d > 5) d=5; if(w>4) w = 4;
      if(m) {
	++event_data.m_trackCollectionStat[nc].m_efficiency [d];
	++event_data.m_trackCollectionStat[nc].m_efficiencyN[d][w];
      }
      int ch = (*p).charge();
      if(m) {
	++event_data.m_trackCollectionStat[nc].m_efficiencyBTE[d][w][(*p).rapidity()];
	ch > 0 ? ++event_data.m_trackCollectionStat[nc].m_efficiencyPOS[d] : ++event_data.m_trackCollectionStat[nc].m_efficiencyNEG[d];
      }
      if(nc==0) {
	++event_data.m_eventStat.m_events; ch > 0 ? ++event_data.m_eventStat.m_eventsPOS : ++event_data.m_eventStat.m_eventsNEG;
	++event_data.m_eventStat.m_eventsBTE[(*p).rapidity()];
      }
      if(d==0) event_data.m_particles[nc].erase(p++);
      else {event_data.m_difference[nc].push_back(n-m);  ++p;}
    }
  }
}

///////////////////////////////////////////////////////////////////
// Pointer to particle production for space point
///////////////////////////////////////////////////////////////////

int ITk::TrackClusterAssValidation::kine
(const ITk::TrackClusterAssValidation::EventData_t &event_data,const Trk::PrepRawData* d1,const Trk::PrepRawData* d2,int* Kine,int nmax) const
{
  int nkine = 0;
  int Kine1[1000],Kine2[1000];
  int n1 = kine(event_data,d1,Kine1,nmax); if(!n1) return nkine;
  int n2 = kine(event_data,d2,Kine2,nmax); if(!n2) return nkine;

  for(int i = 0; i!=n1; ++i) {
    for(int j = 0; j!=n2; ++j) {
      if(Kine1[i]==Kine2[j]) {Kine[nkine++] = Kine1[i];  break;}
    }
  }
  return nkine;
}

///////////////////////////////////////////////////////////////////
// Pointer to particle production for cluster
///////////////////////////////////////////////////////////////////

int ITk::TrackClusterAssValidation::kine
(const ITk::TrackClusterAssValidation::EventData_t &event_data,const Trk::PrepRawData* d,int* Kine,int nmax) const
{

  PRD_MultiTruthCollection::const_iterator mce;
  PRD_MultiTruthCollection::const_iterator mc = findTruth(event_data,d,mce);

  Identifier ID    = d->identify();
  int        nkine = 0;

  for(; mc!=mce; ++mc) {

    if( (*mc).first != ID ) return nkine;

    const int uniqueID = HepMC::uniqueID((*mc).second); if(uniqueID<=0) continue;

    const HepMC::ConstGenParticlePtr pa = (*mc).second.cptr();
    if(!pa or !pa->production_vertex()) continue;

    int pdg = std::abs(pa->pdg_id()); if(m_pdg && m_pdg != pdg ) continue;
    if (MC::isNucleus(pdg)) continue; // ignore nuclei from hadronic interactions
    if ( std::abs(MC::charge(pdg)) < .5 ) continue;

    // pT cut
    //
    double           px = pa->momentum().px();
    double           py = pa->momentum().py();
    double           pz = pa->momentum().pz();
    double           pt = std::sqrt(px*px+py*py);
    if( pt < m_ptcut || pt > m_ptcutmax) continue;

    // Rapidity cut
    //
    double           t  = std::abs(pz)/pt;
    if( t  > m_tcut ) continue;

    // Radius cut
    //
    double           vx = pa->production_vertex()->position().x();
    double           vy = pa->production_vertex()->position().y();
    double           r = std::sqrt(vx*vx+vy*vy);
    if( r < m_rmin || r > m_rmax) continue;

    Kine[nkine] = uniqueID; if(++nkine >= nmax) break;
  }
  return nkine;
}

///////////////////////////////////////////////////////////////////
// Pointer to particle production for cluster
///////////////////////////////////////////////////////////////////

int ITk::TrackClusterAssValidation::kine0
(const ITk::TrackClusterAssValidation::EventData_t &event_data,const Trk::PrepRawData* d,int* Kine,int nmax) 
{

  PRD_MultiTruthCollection::const_iterator mce;
  PRD_MultiTruthCollection::const_iterator mc = findTruth(event_data, d,mce);

  Identifier ID    = d->identify();
  int        nkine = 0;

  for(; mc!=mce; ++mc) {

    if( (*mc).first != ID ) return nkine;

    const int uniqueID = HepMC::uniqueID((*mc).second); if(uniqueID<=0) continue;
    Kine[nkine] = uniqueID; if(++nkine >= nmax) break;
  }
  return nkine;
}

///////////////////////////////////////////////////////////////////
// Test for cluster association with truth particles
///////////////////////////////////////////////////////////////////

bool ITk::TrackClusterAssValidation::isTruth
(const ITk::TrackClusterAssValidation::EventData_t &event_data,const Trk::PrepRawData* d) 
{
  PRD_MultiTruthCollection::const_iterator mce;
  PRD_MultiTruthCollection::const_iterator mc = findTruth(event_data,d,mce);
  return mc!=mce;
}

///////////////////////////////////////////////////////////////////
// Test detector element
///////////////////////////////////////////////////////////////////

bool ITk::TrackClusterAssValidation::isTheSameDetElement
(const ITk::TrackClusterAssValidation::EventData_t &event_data, int K,const Trk::PrepRawData* d) 
{
  std::multimap<int,const Trk::PrepRawData*>::const_iterator k = event_data.m_kinecluster.find(K);
  for(; k!=event_data.m_kinecluster.end(); ++k) {

    if((*k).first!= K) return false;
    if(d->detectorElement()==(*k).second->detectorElement()) return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////
// Test detector element
///////////////////////////////////////////////////////////////////

bool ITk::TrackClusterAssValidation::isTheSameDetElement
(const ITk::TrackClusterAssValidation::EventData_t &event_data, int K,const Trk::SpacePoint* sp) 
{
  const Trk::PrepRawData*  p1 = sp->clusterList().first;
  const Trk::PrepRawData*  p2 = sp->clusterList().second;

  std::multimap<int,const Trk::SpacePoint*>::const_iterator  k = event_data.m_kinespacepoint.find(K);

  if(!p2) {

    for(; k!=event_data.m_kinespacepoint.end(); ++k) {
      if((*k).first!= K) return false;

      const Trk::PrepRawData*  n1 = (*k).second->clusterList().first ;
      const Trk::PrepRawData*  n2 = (*k).second->clusterList().second;

      if(p1->detectorElement() == n1->detectorElement()) return true;
      if(!n2) continue;
      if(p1->detectorElement() == n2->detectorElement()) return true;
    }
    return false;
  }

  for(; k!=event_data.m_kinespacepoint.end(); ++k) {
    if((*k).first!= K) return false;

    const Trk::PrepRawData*  n1 = (*k).second->clusterList().first ;
    const Trk::PrepRawData*  n2 = (*k).second->clusterList().second;

    if(p1->detectorElement() == n1->detectorElement()) return true;
    if(p2->detectorElement() == n1->detectorElement()) return true;
    if(!n2) continue;
    if(p1->detectorElement() == n2->detectorElement()) return true;
    if(p2->detectorElement() == n2->detectorElement()) return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////
// Dump information about no recontructed particles
//////////////recon/////////////////////////////////////////////////////

bool ITk::TrackClusterAssValidation::noReconstructedParticles(const ITk::TrackClusterAssValidation::EventData_t &event_data) const
{

  for(SG::ReadHandleKeyArray<TrackCollection>::size_type nc=0; nc!=m_tracklocation.size(); ++nc) {

    auto p = event_data.m_particles[nc].begin(), pe =event_data.m_particles[nc].end();
    if(p==pe) continue;

    std::list<int>::const_iterator dif = event_data.m_difference[nc].begin();

    std::multimap<int,const Trk::PrepRawData*>::const_iterator c,ce = event_data.m_kinecluster.end();

    int n  = 69-m_tracklocation[nc].key().size();
    std::string s1; for(int i=0; i<n; ++i) s1.append(" "); s1.append("|");
    std::stringstream out;
    out<<"|----------------------------------------------------------------------------------------|\n";
    out<<"|                   "<<m_tracklocation[nc]<<s1<<"\n";
    out<<"|----------------------------------------------------------------------------------------|\n";
    out<<"|    #   pdg     kine   Ncl Ntr Nsp Lose     pT(MeV)    rapidity    radius          z    |\n";
    out<<"|----------------------------------------------------------------------------------------|\n";
    
    n = 0;
    for(; p!=pe; ++p) {

      const int uniqueID = HepMC::uniqueID(*p);

      c = event_data.m_kinecluster.find(uniqueID); if(c==ce) continue;
      const Trk::PrepRawData* d = (*c).second;

      PRD_MultiTruthCollection::const_iterator mce;
      PRD_MultiTruthCollection::const_iterator mc = findTruth(event_data,d,mce);

      Identifier ID    = d->identify();
      bool Q = false;
      for(; mc!=mce; ++mc) {
	if((*mc).first != ID) break;
	if(HepMC::uniqueID((*mc).second)==uniqueID) {Q=true; break;}
      }

      if(!Q) continue;

      const HepMC::ConstGenParticlePtr pa = (*mc).second.cptr();

      double           px =  pa->momentum().px();
      double           py =  pa->momentum().py();
      double           pz =  pa->momentum().pz();
      double           vx = pa->production_vertex()->position().x();
      double           vy = pa->production_vertex()->position().y();
      double           vz = pa->production_vertex()->position().z();
      double           pt = std::sqrt(px*px+py*py);
      double           t  = std::atan2(pt,pz);
      double           ra =-std::log(std::tan(.5*t));
      double           r  = std::sqrt(vx*vx+vy*vy);
      ++n;
      out<<"| "
	     <<std::setw(4)<<n
	       <<std::setw(6)<<pa->pdg_id()
	       <<std::setw(10)<<HepMC::barcode(pa)
	       <<std::setw(4)<<event_data.m_kinecluster   .count(uniqueID)
	       <<std::setw(4)<<event_data.m_kinespacepoint.count(uniqueID)
	       <<std::setw(4)<<(*dif)
	       <<std::setw(12)<<std::setprecision(5)<<pt
	       <<std::setw(12)<<std::setprecision(5)<<ra
	       <<std::setw(12)<<std::setprecision(5)<<r
	       <<std::setw(12)<<std::setprecision(5)<<vz
	       <<"   |\n";
      ++dif;

    }
    out<<"|----------------------------------------------------------------------------------------|\n";
    ATH_MSG_INFO("\n"<<out.str());
  }
  return true;
}

///////////////////////////////////////////////////////////////////
// Cluster truth information
//////////////recon/////////////////////////////////////////////////////

PRD_MultiTruthCollection::const_iterator
ITk::TrackClusterAssValidation::findTruth
(const ITk::TrackClusterAssValidation::EventData_t &event_data,
 const Trk::PrepRawData* d,
 PRD_MultiTruthCollection::const_iterator& mce) 
{
  const InDet::SCT_Cluster    * si = dynamic_cast<const InDet::SCT_Cluster*>    (d);
  const InDet::PixelCluster   * px = dynamic_cast<const InDet::PixelCluster*>   (d);

  PRD_MultiTruthCollection::const_iterator mc;

  if     (px && event_data.m_truthPix) {mc=event_data.m_truthPix->find(d->identify()); mce=event_data.m_truthPix->end();}
  else if(si && event_data.m_truthStrip) {mc=event_data.m_truthStrip->find(d->identify()); mce=event_data.m_truthStrip->end();}
  else {
    const PRD_MultiTruthCollection *truth[] {event_data.m_truthPix,event_data.m_truthStrip};
    for (int i=0; i!=2; ++i) {
        if (truth[i]) {
          mce=truth[i]->end();
          return truth[i]->end();
        }
    }
    throw std::runtime_error("Neither Pixel nor Strip truth.");
  }
  return mc;
}

///////////////////////////////////////////////////////////////////
// Cluster truth information
//////////////recon/////////////////////////////////////////////////////

int ITk::TrackClusterAssValidation::charge(const ITk::TrackClusterAssValidation::EventData_t &event_data,std::pair<int,const Trk::PrepRawData*> pa,int& rap, double& eta) const
{
  int                     uniqueID = pa.first;
  const Trk::PrepRawData* d = pa.second;
  PRD_MultiTruthCollection::const_iterator mce;
  PRD_MultiTruthCollection::const_iterator mc = findTruth(event_data,d,mce);

  for(; mc!=mce; ++mc) {
    if(HepMC::uniqueID((*mc).second)==uniqueID) {

      const HepMC::ConstGenParticlePtr   pat  = (*mc).second.cptr();

      rap       = 0;
      double px =  pat->momentum().px();
      double py =  pat->momentum().py();
      double pz =  pat->momentum().pz();
      double pt = std::sqrt(px*px+py*py)   ;
      double t  = std::atan2(pt,pz)        ;
      eta = std::abs(std::log(std::tan(.5*t)));
      // Forward
      if (eta > 3.0)
	rap = 3;
      else
      // other regions
	eta > 1.6 ? rap = 2 : eta > .8 ?  rap = 1 : rap = 0;

      int                         pdg = pat->pdg_id();
      if (MC::isNucleus(pdg)) continue; // ignore nuclei from hadronic interactions
      double ch = MC::charge(pdg);
      if(ch >  .5) return  1;
      if(ch < -.5) return -1;
      return 0;
    }
  }
  return 0;
}
