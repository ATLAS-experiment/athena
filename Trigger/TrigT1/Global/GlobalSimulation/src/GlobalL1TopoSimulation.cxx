//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#include "GlobalL1TopoSimulation.h"
#include "Digraph.h"
#include "DataRepository.h"
#include "DataRepositoryVisitors.h"
#include "TrigConfData/L1Menu.h"
#include "GlobalTopoHistSvc.h"
#include "AlgDataFetcherFactory.h"

#include "L1TopoHardware/L1TopoHardware.h"


#include "L1TopoInterfaces/ConfigurableAlg.h"
#include "L1TopoInterfaces/CountingAlg.h"
#include "L1TopoInterfaces/AlgFactory.h"
#include "L1TopoInterfaces/ParameterSpace.h"

#include "GlobalAlgs.h"


#include "dot.h"

#include <string>
#include <map>
#include <algorithm>
#include <iterator>
#include <vector>
#include <sstream>
#include <fstream>

#include <boost/lexical_cast.hpp>

ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

namespace  GlobalSim {

  // declarations of non-member functions

 // obtain non-legacy L1Topo boards from the menu object.
  std::vector<const TrigConf::L1Board*>  get_boards_(const TrigConf::L1Menu*);

  // find the Decision or Counting configuration Algs according to flag
  std::vector<const TrigConf::L1TopoAlgorithm*>
  confAlgs_(const TrigConf::L1Menu*, bool decisionAlgs);

  //add Algortihm instances to TCS::AlgFactory. TCSAlgFacotry is a global
  // singleton.
  void
  addAlgorithmsToFactory(const std::vector<const TrigConf::L1TopoAlgorithm*>&);

  // use config algs to initialise TCS algs (algorithms that act on ATLAS data)
  // for Algs which are not Counting Algs
  void
  initialiseNonCountingAlgorithms
  (const std::vector<const TrigConf::L1TopoAlgorithm*>&,
   std::shared_ptr<IL1TopoHistSvc>);

  // use config algs to initialise TCS algs (algorithms that act on ATLAS data)
  // for Counting Algs
  void initialiseCountingAlgorithms ATLAS_NOT_THREAD_SAFE 
  (const std::vector<const TrigConf::L1TopoAlgorithm*>&,
   const TrigConf::L1Menu*,
   std::shared_ptr<IL1TopoHistSvc> histSvc);
  
  // Find the configuration connectors associated to a configuration board.
  std::vector<const TrigConf::L1Connector*>
  conf_connectors_(const TrigConf::L1Menu*, const TrigConf::L1Board*);


  // Ensure a vector of config Decision Algs has no duplicates
  template<typename Iter>
  std::vector<const TrigConf::L1TopoAlgorithm*>
  unique_conf_decision_algs_(Iter begin,
			     Iter end,
			     const TrigConf::L1Menu*);

  // Ensure a vector of config Sorting Algs has no duplicates
  std::vector<const TrigConf::L1TopoAlgorithm*>
  unique_conf_sorting_algs_(const  std::vector<const TrigConf::L1TopoAlgorithm*>&,
			    const TrigConf::L1Menu*);
  
  // Ensure a vector of config Counting Algs has no duplicates
  template<typename Iter>
  std::vector<const TrigConf::L1TopoAlgorithm*>
  unique_conf_counting_algs_(Iter begin,
			     Iter end,
			     const TrigConf::L1Menu*);
 
  uint32_t interpretGenericParam(const std::string&);
}

namespace GlobalSim {

  GlobalL1TopoSimulation::GlobalL1TopoSimulation(const std::string& name,
				   ISvcLocator* pSvcLocator):
    AthReentrantAlgorithm(name, pSvcLocator) {
  }

  using StringMap = std::map<std::string, std::vector<std::string>>;
  
  StatusCode GlobalL1TopoSimulation::initialize  ATLAS_NOT_THREAD_SAFE () {

    ATH_MSG_DEBUG("initialising");
    
    if (!(m_doEConns or m_doOConns)) {
      ATH_MSG_ERROR ("No Algorithms  requested");
      return StatusCode::FAILURE;
    }


    /*
     * Set up a graph based set of calls to L1Topo Algorithms.
     *
     * connectorGraph() uses the
     * hardware entity objects from the menu to  find the Algorithms
     * to use, and build a graph
     * from the connectors (each connector has a single Algorithm).
     *
     */

    /*
     * Retrieve the L1menu. This will be used to
     * - build the call graph bsed on TrigConf::Connectors.
     * - instantiate and initialise the TCS::Algorithms held in the global
     *   AlgFactory,
     */

    const TrigConf::L1Menu* l1menu;
    ATH_CHECK(detStore()->retrieve(l1menu));


    // The hist svc is used by the L1Topo Algotihms, so initialise
    // this before Alg instaniation
    m_topoHistSvc =
      std::shared_ptr<IL1TopoHistSvc>(new GlobalTopoHistSvc);

    /*
     * add TCS Algorithm instances to AlgFactory. Initialise
     * the TCS Algorithms from conf Algoritithms
     */

    const bool& dec_algs = true;
    
    auto decisionConfAlgs = confAlgs_(l1menu, dec_algs);
    auto sortingConfAlgs =  unique_conf_sorting_algs_(decisionConfAlgs,
						      l1menu);
    auto countingConfAlgs = confAlgs_(l1menu, !dec_algs);

    // Instantiate and initialise TCS Algorithms

    if (m_doEConns) {
      addAlgorithmsToFactory(decisionConfAlgs); // add TCS alg to AlgFactory
      addAlgorithmsToFactory(sortingConfAlgs); // add TCS alg to AlgFactory
      initialiseNonCountingAlgorithms(decisionConfAlgs,
				      m_topoHistSvc);
      initialiseNonCountingAlgorithms(sortingConfAlgs,
				      m_topoHistSvc);
    }

    if (m_doOConns) {
      addAlgorithmsToFactory(countingConfAlgs); // add TCS alg to AlgFactory
      initialiseCountingAlgorithms(countingConfAlgs,
				   l1menu,
				   m_topoHistSvc);
    }

    /*
     * Obtain information used to build the call digraph
     * Need the connectors In the L1TopoSimulation package
     * Decision, Sorting and Counting connectors each
     * contain a single Algorithm.  All Connectors, except for Input
     * Connectors, know (as strings) their input connectors:
     *
     * DecisionConnector <- Sorting Connector
     * SortingConnector <- InputConnector
     * CountingConnector <- InputConnectors.
     *
     * In GlobalSim, we are interested in the TCS Algorithms. Further,
     * we add the notion of InputAlgorithm (these are  Algorithms
     * which extract data from the TopoInputEvent, and write it
     * to the DataRepository.
     */
    
    ATH_MSG_DEBUG("obtaining AlgData");
	  
    
    auto optAlgDataFetcherPair = makeAlgDataFetcher(m_doEConns,
						    m_doOConns,
						    l1menu);
    if (!(optAlgDataFetcherPair.first).has_value()){
      ATH_MSG_ERROR("invalid algDataFetcher");
      for (const auto& msg : optAlgDataFetcherPair.second) {
	ATH_MSG_ERROR(msg);
      }
      return StatusCode::FAILURE;
    }
    auto algDataFetcher = *(optAlgDataFetcherPair.first);
    
    CHECK(addGSAlgorithms(algDataFetcher->execOrderedAlgData()));

  
    /*
     * Allow inputs from the Event Store
     */
    
    ATH_MSG_DEBUG("retrieving " << m_jetInputProvider);
    CHECK( m_jetInputProvider.retrieve() );
    
    ATH_MSG_DEBUG("retrieving " << m_emtauInputProvider);
    CHECK( m_emtauInputProvider.retrieve() );
    
    ATH_MSG_DEBUG("retrieving " << m_energyInputProvider);
    CHECK( m_energyInputProvider.retrieve() );
    
    
    /*   26/09/2023 - with asetup Athena, main -> detstore error
	 
	 ATH_MSG_DEBUG("retrieving " << m_muonInputProvider);
	 constexpr auto no_legacy = DisableTool(false);
	 CHECK( m_muonInputProvider.retrieve(DisableTool{no_legacy}));
	 
    */

    
    return StatusCode::SUCCESS;
    
  }

      
  StatusCode
  GlobalL1TopoSimulation::addGSAlgorithms (const std::vector<ADP>& adps) {

    auto addAlg = [] ATLAS_NOT_THREAD_SAFE (const auto& adp) {

      const auto& sn = adp->m_sn;
      const auto& descriptor = adp->m_desc;

      auto pGSAlg = std::shared_ptr<IGlobalAlg>(nullptr);
      
      if (descriptor == AlgDataDesc::root) {
	
	pGSAlg.reset(new Alg_gsRoot(sn));
	return pGSAlg;
      }
      
      const auto& algName = adp->m_algName;
      auto alg = TCS::AlgFactory::mutable_instance().algorithm(algName);
      
      if(!alg) {
	// input Alg. The alg name (string) will be used to
	// retrieve data from the input event object.
	pGSAlg.reset(new Alg_gsInput(sn, algName));
	return pGSAlg;
      }
      
      const auto& childSNs = adp->m_childSNs;
      
      if (descriptor == AlgDataDesc::count) {
	
	pGSAlg.reset(new Alg_gsCounting(childSNs[0], // input sn
					sn, // output sn
					alg));
	return pGSAlg;
      }
      
      if (descriptor == AlgDataDesc::sort) {
	
	pGSAlg.reset(new Alg_gsSorting(childSNs[0], // input sns
				       sn, // output sn
				       alg));
		     
	return pGSAlg;
      }
    
      
      if (descriptor == AlgDataDesc::decision) {
	
	pGSAlg.reset(new Alg_gsDecision(childSNs, // input sns
					sn, // output sn
					adp->m_triggerLines,
					alg));
	
	return pGSAlg;
      }
      
      return pGSAlg;
    };

    std::transform(adps.cbegin(),
		   adps.cend(),
		   std::back_inserter(m_gsAlgs),
		   addAlg);
  
    return StatusCode::SUCCESS;
  }

        
 
  StatusCode GlobalL1TopoSimulation::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("Executing ...");

    ATH_MSG_DEBUG (" StoreGate structure on entering execute() \n"
		  << evtStore()->dump());
    TCS::TopoInputEvent inputEvent;
    
    inputEvent.setEventInfo(ctx.eventID().run_number(),
			    ctx.eventID().event_number(),
			    ctx.eventID().lumi_block(),
			    ctx.eventID().bunch_crossing_id());
    
    
    
    // JET
    CHECK(m_jetInputProvider->fillTopoInputEvent(inputEvent));
    
    CHECK(m_emtauInputProvider->fillTopoInputEvent(inputEvent));
    CHECK(m_energyInputProvider->fillTopoInputEvent(inputEvent));
    
    inputEvent.print();
    /*
    // Muon
    if (m_muonInputProvider.isEnabled()) {
    CHECK(m_muonInputProvider->fillTopoInputEvent(inputEvent));
    }
    */

 
    // The reason for rebuilding the GlobalSystem instance every
    // event is the that the Alg classes derived from
    // TCS::ConfigurableAlgorithm have non-const process methods.
    
    std::ofstream dr_stream("DataRepository.txt", std::ios_base::app);
    auto pVisitor = std::make_shared<SimpleRepositoryDumper> (dr_stream);
    auto boards= std::vector<std::unique_ptr<GEPBoard>>();
    for (int i = 0; i != m_nGEPBoards; ++i) {
      auto board = std::make_unique<GEPBoard>(m_gsAlgs);
      boards.push_back(std::move(board));
    }
    auto globalSystem = GlobalSystem(std::move(boards));
    globalSystem.run(inputEvent, pVisitor);



    return StatusCode::SUCCESS;
  }
  
  StatusCode GlobalL1TopoSimulation::finalize() {
    ATH_MSG_INFO("finalising");

    // write out histograms
    m_topoHistSvc->save();

 
    return StatusCode::SUCCESS;
  }

  template<typename Iter>
  std::vector<const TrigConf::L1TopoAlgorithm*>
  unique_conf_decision_algs_(Iter begin,
			     Iter end,
			     const TrigConf::L1Menu* l1menu) {

    auto tlnames = std::vector<std::string>();
    for (auto iter = begin; iter != end; ++iter) {// config connectors
      auto l1conn = *iter;
      for( size_t fpga_ind : { 0, 1} ) {
	for( size_t clock_ind : { 0, 1} ) {
	  for( auto & tl : l1conn->triggerLines(fpga_ind, clock_ind)) {
	    tlnames.push_back(tl.name());
	  }
	}
      }
    }

    std::sort(tlnames.begin(), tlnames.end());
    
    auto iter = std::unique(tlnames.begin(), tlnames.end());

    tlnames.erase(iter, tlnames.end());

    auto algs = std::vector<const TrigConf::L1TopoAlgorithm*>();
    algs.reserve(tlnames.size());
    std::transform(tlnames.cbegin(),
		   tlnames.cend(),
		   std::back_inserter(algs),
		   [&l1menu](const auto& name) {
		     return &(l1menu->algorithmFromTriggerline(name));});

    // it seems that different names give rise to the same Algorithm...
    std::sort(algs.begin(), algs.end());
    auto iter1 = std::unique(algs.begin(), algs.end());

    algs.erase(iter1, algs.end());
    return algs;
  }

  std::vector<const TrigConf::L1TopoAlgorithm*>
  unique_conf_sorting_algs_(const std::vector<const TrigConf::L1TopoAlgorithm*>& decision_algs,
			 const TrigConf::L1Menu* l1menu) {
    
    auto sortNames = std::vector<std::pair<std::string,std::string>>();

    for (const auto& d_alg : decision_algs) {
      for (const auto& sc_name : d_alg->inputs()){
	sortNames.emplace_back(sc_name, d_alg->category());
      }	
    }

    std::sort(sortNames.begin(), sortNames.end());
    auto iter = std::unique(sortNames.begin(),
			    sortNames.end());
    
    sortNames.erase(iter, sortNames.end());

    auto sort_algs = std::vector<const TrigConf::L1TopoAlgorithm*>();
    sort_algs.reserve(sortNames.size());
    
    std::transform(sortNames.cbegin(),
		   sortNames.cend(),
		   std::back_inserter(sort_algs),
		   [&l1menu](const auto& s_name){
		     return &(l1menu->algorithm(s_name.first, s_name.second));});
    
    
    return sort_algs;
  }

  template<typename Iter>
  std::vector<const TrigConf::L1TopoAlgorithm*>
  unique_conf_counting_algs_(Iter begin,
			     Iter end,
			     const TrigConf::L1Menu* l1menu){
    /*
      Configure optical connectors - multiplicity algorithms
      In multiplicity boards all trigger lines are copied into the
      L1Menu::Connector 4 times (fpga 0,1 and clock 0,1)
      Take (fpga, clock) = (0, 0)
    */

    auto tlnames = std::vector<std::string>();
    
    for (auto iter = begin; iter != end; ++iter) {// config connectors
      auto l1conn = *iter;
      const auto& triggerLines = l1conn->triggerLines(0, 0);

      std::transform(triggerLines.cbegin(),
		     triggerLines.cend(),
		     std::back_inserter(tlnames),
		     [](const auto& tl) {return tl.name();});
    }

    std::sort(tlnames.begin(), tlnames.end());
    
    auto iter = std::unique(tlnames.begin(), tlnames.end());
    tlnames.erase(iter, tlnames.end());

    // it seems that different names give rise to the same Algorithm...
    auto algs = std::vector<const TrigConf::L1TopoAlgorithm*>();
    algs.reserve(tlnames.size());
    std::transform(tlnames.cbegin(),
		   tlnames.cend(),
		   std::back_inserter(algs),
		   [&l1menu](const auto& name) {
		     return &(l1menu->algorithmFromTriggerline(name));});
    
    // it seems that different names give rise to the same Algorithm...
    std::sort(algs.begin(), algs.end());
    auto iter1 = std::unique(algs.begin(), algs.end());
    algs.erase(iter1, algs.end());
 
    return algs;
  }
      
  std::vector<const TrigConf::L1Board*>
  get_boards_(const TrigConf::L1Menu* l1menu){
    std::vector<const TrigConf::L1Board*> boards;
    for (const std::string & boardName : l1menu->boardNames() ){
      
      auto & l1board = l1menu->board(boardName);
      
      if (l1board.type() != "TOPO") continue;
      if (l1board.legacy()) continue;
      boards.push_back(&l1board); // l1board is a reference
    }

    return boards;
 
  }

  std::vector<const TrigConf::L1Connector*>
    conf_connectors_(const TrigConf::L1Menu* menu,
		    const TrigConf::L1Board* board){

    std::vector<const TrigConf::L1Connector*> connectors;
    const auto& connectorNames = board->connectorNames();
    std::transform(connectorNames.cbegin(),
		   connectorNames.cend(),
		   std::back_inserter(connectors),
		   [&menu](const auto& name) {
		     return &(menu->connector(name));});		     
    return connectors;
  }


  void
  addAlgorithmsToFactory(const std::vector<const TrigConf::L1TopoAlgorithm*>& conf_algs){
    
    auto createAlg = [](const auto& c_alg){
      auto alg = TCS::AlgFactory::mutable_instance().algorithm(c_alg->name());
      if(!alg){
	auto classname = c_alg->klass();
	// in sim, use the same multiplicity algo
	// for fixed and variable thresholds
	if(classname=="eEmVarMultiplicity") {classname="eEmMultiplicity";}
	
	alg = TCS::AlgFactory::mutable_instance().create(classname,
							 c_alg->name());
	if(!alg){
	  std::stringstream msg;
	  msg  << "instantiateAlgorithms failed to create L1TopoAlg "
	       << " name " <<  c_alg->name()
	       << " klass " <<  c_alg->klass();
	  throw std::runtime_error(msg.str());
	}
      }
    };
    
    std::for_each(conf_algs.cbegin(),
		  conf_algs.cend(),
		  createAlg);
      
  }

  void
  initialiseNonCountingAlgorithms(const std::vector<const TrigConf::L1TopoAlgorithm*>& conf_algs,
				  std::shared_ptr<IL1TopoHistSvc> histSvc){

    auto setAlgParametersAndInit = [&histSvc](const auto& c_alg) {

      TCS::ConfigurableAlg * alg =
	TCS::AlgFactory::mutable_instance().algorithm(c_alg->name());
      if(!alg) {
	throw std::runtime_error("Unable to retrieve TCS::Alg " +
				 c_alg->name());
      }
    
      auto ps = TCS::ParameterSpace(alg->name());
      for (const auto& pe : c_alg->parameters()) {
	ps.addParameter(pe.name(), pe.value(), pe.selection());
      }
    
      for (const auto& pname : c_alg->generics().getKeys()) {
	const auto& pe = c_alg->generics().getObject(pname);
	uint32_t  val = interpretGenericParam(pe.getAttribute("value"));
      
	if (pname == "NumResultBits") {
	  auto sz =  c_alg->outputs().size();
	  if(val != sz) {
	    std::stringstream ss;
	    ss << "Algorithm <" << pname << "parameter OutputBits ("
	       << val << ") !=  output size ("
	       << sz << ")";
	    throw std::runtime_error(ss.str());
	  }
	  // parameter is not added as is defined through the output list
	  continue;
	}
	ps.addParameter(pname, val);
      }

      alg->setParameters(ps);
      alg->setLegacyMode(false);
      alg->setL1TopoHistSvc(histSvc);
      alg->initialize();
    };
    
    std::for_each(conf_algs.cbegin(),
		  conf_algs.cend(),
		  setAlgParametersAndInit);
  }
  
  void initialiseCountingAlgorithms ATLAS_NOT_THREAD_SAFE 
  (const std::vector<const TrigConf::L1TopoAlgorithm*>& conf_algs,
   const TrigConf::L1Menu* l1menu,
      std::shared_ptr<IL1TopoHistSvc> histSvc){

    auto setAlgParametersAndInit =
      [&l1menu, &histSvc](const auto& c_alg) {
      
      TCS::ConfigurableAlg* alg =
	TCS::AlgFactory::mutable_instance().algorithm(c_alg->name());
      if(!alg) {
	throw std::runtime_error("Unable to retrieve TCS::Alg " +
				 c_alg->name());
      }

      const auto& l1thr = l1menu->threshold(c_alg->outputs().at(0));
      auto* pca = dynamic_cast<TCS::CountingAlg*>(alg);
      if (!pca){
	throw std::runtime_error("Error down casting to CountingAlg");
      }

      pca->setThreshold(l1thr);
      alg->setL1TopoHistSvc(histSvc);
      pca->initialize();
    };

    std::for_each(conf_algs.cbegin(), conf_algs.cend(),
		  setAlgParametersAndInit);
  }
  
  uint32_t interpretGenericParam(const std::string& parvalue) {
    uint32_t val;
    try {
      val  = boost::lexical_cast<uint32_t, std::string>(parvalue);
    }
    catch(const boost::bad_lexical_cast & bc) {
      if( parvalue.size()>=3 && parvalue[0]==':'
	  and parvalue[parvalue.size()-1]==':' ) {
	
	auto x =
	  TCS::L1TopoHWParameters::get().find(parvalue.substr(1,parvalue.size()-2));

	std::string parname = parvalue.substr(1,parvalue.size()-2);

	if( x != TCS::L1TopoHWParameters::get().end()) {
	  val = x->second.value;
	} else {
	  std::stringstream ss;
	  ss <<"Generic parameter value "
	     << parvalue
	     << " has the hardware contrained parameter format, but '"
	     << parname
	     << "' is not listed in L1TopoHardware.cxx";
            
	  throw std::runtime_error(ss.str());
	}
      } else {
	std::stringstream ss;
	ss << "Generic parameter value "
	   << parvalue
	   << " is not a uint32_t and does not match the  hardware "
	   << " contrained parameter specification ':<parname>:' ";
	throw std::runtime_error(ss.str());
      }
    }
    return val;
  }


 std::vector<const TrigConf::L1TopoAlgorithm*>
 confAlgs_(const TrigConf::L1Menu* l1menu, bool decisionAlgs){

   /* find the Decision or Counting Configuration Algorithms:
    * boards->connectors->algs.
    * the boolean flag determines whether Decision or Counting algs
    * are returned.
    */

   auto boards = get_boards_(l1menu);
   
   std::vector<const TrigConf::L1Connector*> pConnectors;
   
   /*
    * obtain the conf connectors the L1 topo boards know about.
    */
   
   for (const auto& pboard : boards) {
     auto connectors = conf_connectors_(l1menu, pboard);
     pConnectors.insert(pConnectors.end(),
			connectors.begin(),
			connectors.end());
   }
   
   auto econn_end =
     std::partition(pConnectors.begin(),
		    pConnectors.end(),
		    [](const auto& pcon) {
		      return pcon->connectorType() ==
			TrigConf::L1Connector::ConnectorType::ELECTRICAL;});

   if (decisionAlgs) {
     return  unique_conf_decision_algs_(pConnectors.begin(),
					econn_end,
					l1menu);
   } else {
     return  unique_conf_counting_algs_(econn_end,
					pConnectors.end(),
					l1menu);
   }
 }

}

