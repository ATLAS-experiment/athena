///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// SGAudSvc.cxx 
// Implementation file for class SGAudSvc
// Author: Ilija Vukotic<ivukotic@cern.ch>
/////////////////////////////////////////////////////////////////// 


// STL includes
#include <sstream>
#include <fstream>
#include <vector>
#include <set>
#include <utility>

// FrameWork includes
#include "Gaudi/Property.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/IAlgContextSvc.h"

// StoreGate includes
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/DataHandle.h"

// SGAudSvc includes
#include "SGAudSvc.h"
/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

/*----------------------------------------------------------------------------*/
// Constructors
////////////////
SGAudSvc::SGAudSvc( const std::string& name, ISvcLocator* pSvcLocator ) : 
  base_class  ( name,     pSvcLocator ),
  p_algCtxSvc("AlgContextSvc", name),
  m_pCID("ClassIDSvc", name)
{
}


/*----------------------------------------------------------------------------*/

// Athena Algorithm's Hooks
////////////////////////////
StatusCode 
SGAudSvc::initialize() {
  
  ATH_MSG_INFO("Initializing " << name() << "..." );

  ATH_CHECK( p_algCtxSvc.retrieve() );
  ATH_CHECK( m_pCID.retrieve() );

  if (m_allFileName != "") {
    m_ofa.open(m_allFileName.value().c_str());
  }

  if (m_sumFileName != "") {
    m_ofs.open(m_sumFileName.value().c_str());
  }
  
  // Set to be listener for end-of-event
  ServiceHandle<IIncidentSvc> incSvc( "IncidentSvc", this->name() );
  ATH_CHECK( incSvc.retrieve() );

  incSvc->addListener( this, IncidentType::BeginRun );
  incSvc->addListener( this, IncidentType::BeginEvent );
  incSvc->addListener( this, IncidentType::EndEvent );

  return StatusCode::SUCCESS;
}

/*----------------------------------------------------------------------------*/

StatusCode 
SGAudSvc::finalize() {

  if (m_vAlg.size()==0) {
    ATH_MSG_WARNING("No data gathered. This might be because you did not run over at least 3 events.");
    return StatusCode::SUCCESS;
  }
  
  ATH_MSG_INFO("Writing output to: "<<m_outFileName);
  std::ofstream f( m_outFileName.value().c_str() );
  
  f << "Algs: " << m_vAlg.size() << std::endl;
  std::vector<std::string>::iterator i;
  for (i=m_vAlg.begin();i<m_vAlg.end();++i) {
          f << (*i) << std::endl;
  }
  
  f << "Obj: "<< m_vObj.size()<<std::endl;
  for (i=m_vObj.begin();i<m_vObj.end();++i) {
          f << (*i) << std::endl; 
  }

  
  for (unsigned int w=0;w<m_vAlg.size();w++){
    for(unsigned int q=0;q<m_vObj.size();q++){
      int oaHash=q*1000 + w;
      if (m_timesRead.find(oaHash)==m_timesRead.end())
	f << "0:";
      else
	f << m_timesRead.find(oaHash)->second << ":";
      if (m_timesWritten.find(oaHash)==m_timesWritten.end())
	f << "0\t";
      else
	f << m_timesWritten.find(oaHash)->second << "\t";	
    }
    f << std::endl;
  }
  
  f.close();

  if (m_ofa.is_open()) m_ofa.close();

  if (m_ofs.is_open()) {

    writeJSON();

    m_ofs.close();
  }


  return StatusCode::SUCCESS;
}

/*----------------------------------------------------------------------------*/

void 
SGAudSvc::handle( const Incident& inc )
{
  ATH_MSG_VERBOSE("Entering handle(). Incidence type: " << inc.type() <<
                  " from: " << inc.source());

  if (inc.type() == IncidentType::BeginEvent) {
    if (m_ofa.is_open())
      m_ofa << "---- BEGIN EVENT " << m_nEvents << " ----" << std::endl;
    m_inExec = true;

  }

  // Performing performance-monitoring for EndEvent incident
  if ( inc.type() == IncidentType::EndEvent ) {
    monitor();

    if (m_ofa.is_open()) 
      m_ofa << "---- END EVENT " << m_nEvents << " ----" << std::endl;
    m_inExec = false;

  }

  // Performing performance-monitoring for BeginRun incident
  // at this point everybody has been initialized, we can harvest performance
  // data for the initialize step.
  
  if ( inc.type() == IncidentType::BeginRun ) {
    monitor();
  }

  return;
}

		
/*----------------------------------------------------------------------------*/

void 
SGAudSvc::monitor(){
  m_nEvents++;
  return;
}

/*----------------------------------------------------------------------------*/

void 
SGAudSvc::SGAudRETRIEVE(std::string SGobject){
  // if (m_nEvents<3) return;
  if (SGobject=="") SGobject="noKey";
  bool isAnumb=true;
  for (unsigned int i = 0; i < SGobject.length(); i++)        
    if (!std::isdigit(SGobject[i])){
      isAnumb=false; 
      break;   
    }

  if (!isAnumb){
    if (m_currObj==SGobject) 
      addRead();
    else {
      getNobj(SGobject);
      addRead();
    }
  }
  return;	
}

/*----------------------------------------------------------------------------*/

void 
SGAudSvc::SGAudRECORD(std::string SGobject){
  // if (m_nEvents<3) return;
  if (SGobject=="") SGobject="noKey";  
  for (unsigned int i = 0; i < SGobject.length(); i++)        
    if (!std::isdigit(SGobject[i])){
      //isAnumb=false; 
      break;   
    }
  
  if (m_currObj==SGobject) 
    addWrite();
  else {
    getNobj(SGobject);
    addWrite();
  }

  return;	
}

/*----------------------------------------------------------------------------*/
void 
SGAudSvc::SGAudit(const std::string& key, const CLID& id, 
		  const int& typ, const int& store_id) {

  // we can sometimes get here really early, before initialization.
  if (m_pCID == 0) { return; }

  bool a = SGGetCurrentAlg();  

  if (m_nEvents >= m_startEvent && store_id == 0 && a) {
    if (typ == 0) {
      SGAudRETRIEVE(key);
    } else {
      SGAudRECORD(key);
    }
  }

  std::string idname;

  if (m_ofa.is_open() || ( m_ofs.is_open() && ! m_useCLID ) ) {
      if( ! m_pCID->getTypeNameOfID(id,idname).isSuccess()) {
	std::ostringstream ost;
	ost << id;
	idname = ost.str();
      }
  }

  if (m_ofa.is_open()) {
    m_ofa << ( (typ == 1) ? "RECORD" : "RETRIEVE" ) << " clid: " << id 
	  << " | ";
    
    m_ofa << idname;

    m_ofa << "  key: " << key << "  alg: " << m_currAlg 
	  << "  store: " << store_id
	  << std:: endl;
  }    

  // store stuff for the summary

  if (!m_ofs.is_open()) return;
  
  if (m_nEvents < m_startEvent || !m_inExec) return;
  
  if (m_currAlg == "----") return;
    
  std::string kk;
  if (m_useCLID) {
    std::ostringstream ost;
    ost << id << "/" << key;
    kk = ost.str();
  } else {
    kk = idname + "/" + key;
  }

  DataMap::iterator itr;
  if (typ == 0) {
    itr = m_read.find(m_currAlg);
    if (itr != m_read.end()) {
      itr->second.insert(std::move(kk));
    } else {
      m_read[m_currAlg] = std::set<std::string> ( {std::move(kk)} );
    }
  } else {
    itr = m_write.find(m_currAlg);
    if (itr != m_write.end()) {
      itr->second.insert(std::move(kk));
    } else {
      m_write[m_currAlg] = std::set<std::string> ( {std::move(kk)} );
    }
  }
}

/*----------------------------------------------------------------------------*/

bool 
SGAudSvc::SGGetCurrentAlg(){

  IAlgorithm *asdf = p_algCtxSvc->currentAlg();
  if (!asdf || m_nEvents==0) { // to skip before first event 
    m_currAlg="----";
    m_nCurrAlg=-1;
    return false;
  }
  std::string name = asdf->name();
  
  if ( !m_ignoreFakeAlgs && !m_fakeCurrAlg.empty() )
    name=m_fakeCurrAlg;
  
  if (name!=m_currAlg){
    std::vector<std::string>::iterator i;
    int index=0;
    for (i=m_vAlg.begin();i<m_vAlg.end();++i){
      if (*i==name) {
	m_nCurrAlg=index;
	m_currAlg=name;
	return true;
      }
      index++;
    }
    m_vAlg.push_back(name);
    m_nCurrAlg=index;
    m_currAlg=std::move(name);
    return true;
  }
  return true;
}

/*----------------------------------------------------------------------------*/

void 
SGAudSvc::getNobj(const std::string& name){
  std::vector<std::string>::iterator i;
  int index=0;
  for (i=m_vObj.begin();i<m_vObj.end();++i){
    if (*i==name) {
      m_nCurrObj=index;
      m_currObj=name;
      return;
    }
    index++;
  }  
  m_vObj.push_back(name);
  m_nCurrObj=index;
  m_currObj=name;
return;
}

/*----------------------------------------------------------------------------*/

void 
SGAudSvc::addRead(){
  int oaHash=m_nCurrObj*1000 + m_nCurrAlg;
  if (m_timesRead.end()!=m_timesRead.find(oaHash)){
    m_timesRead.find(oaHash)->second++;
  } 
  else{
    std::pair<int,int> p(oaHash,1);
    m_timesRead.insert(p);
  }
  return;
}

/*----------------------------------------------------------------------------*/

void 
SGAudSvc::addWrite(){
  int oaHash=m_nCurrObj*1000 + m_nCurrAlg;
  if (m_timesWritten.end()!=m_timesWritten.find(oaHash)){
    m_timesWritten.find(oaHash)->second++; 
  }
  else{
    std::pair<int,int> p(oaHash,1);
    m_timesWritten.insert(p);
  }
  return;
}

/*----------------------------------------------------------------------------*/

void 
SGAudSvc::setFakeCurrentAlg( const std::string& s ) { m_fakeCurrAlg=s; }

/*----------------------------------------------------------------------------*/

void 
SGAudSvc::clearFakeCurrentAlg() { m_fakeCurrAlg.clear(); }

/*----------------------------------------------------------------------------*/

void
SGAudSvc::writeJSON() {

  DataMap::const_iterator itr;
  std::set<std::string>::const_iterator it2;
  std::vector<std::string>::const_iterator ia;
  
  m_ofs << "{ \"algorithms\" : [" << std::endl;
  for (ia = m_vAlg.begin(); ia != m_vAlg.end(); ++ia) {
    m_ofs << " {" << std::endl;
    m_ofs << "  \"name\" : \"" << *ia << "\"," << std::endl;
    m_ofs << "  \"inputs\" : [";
    
    itr = m_read.find(*ia);
    if (itr != m_read.end()) {
      for (it2 = itr->second.begin(); it2 != itr->second.end(); ++it2) {
	if (it2 != itr->second.begin()) m_ofs << ",";
	m_ofs << "\"" << *it2 << "\"";
      }
    }
    m_ofs << "]," << std::endl;
    
    m_ofs << "  \"outputs\" : [";
    
    itr = m_write.find(*ia);
    if (itr != m_write.end()) {
      for (it2 = itr->second.begin(); it2 != itr->second.end(); ++it2) {
	if (it2 != itr->second.begin()) m_ofs << ",";
	m_ofs << "\"" << *it2 << "\"";
      }
    }
    m_ofs << "]," << std::endl;    
    m_ofs << " }," << std::endl;
  }
  m_ofs << " ]" << std::endl << "}" << std::endl;

}
