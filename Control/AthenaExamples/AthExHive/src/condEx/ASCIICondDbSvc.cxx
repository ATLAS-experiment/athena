/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ASCIICondDbSvc.h"
#include "AthenaKernel/CondCont.h"

#include <regex>
#include <ranges>

#include <fstream>

const std::string r_t("\\[([0-9]+),([0-9]+)\\]");
const std::string r_r = "\\s*\\{" + r_t + "-" + r_t + "\\}\\s*";
const std::string r_e = "\\s*\\{" + r_t + "-" + r_t + "\\}=([0-9]+)\\s*";
const std::string r_ef = "\\s*\\{" + r_t + "-" + r_t + "\\}=(-*[0-9]*\\.*[0-9]*)\\s*";
const std::regex rr(r_r);
const std::regex re(r_e);
const std::regex ref(r_ef);


//---------------------------------------------------------------------------

ASCIICondDbSvc::ASCIICondDbSvc( const std::string& name, ISvcLocator* svcLoc ):
  base_class(name,svcLoc)
{}

//---------------------------------------------------------------------------

StatusCode
ASCIICondDbSvc::initialize() {

  // Initialise mother class in order to print DEBUG messages during initialize()
  StatusCode sc(AthService::initialize());
  msg().setLevel( m_outputLevel.value() );

  if (!sc.isSuccess()) {
    ATH_MSG_WARNING( "Base class could not be initialized" );
    return StatusCode::FAILURE;
  }

  if (m_file == "") {
    ATH_MSG_DEBUG("db file not set");
    return StatusCode::SUCCESS;
  }

  if (readDbFile(m_file).isFailure()) {
    return StatusCode::FAILURE;
  }

  std::ostringstream ost;
  std::print(ost, " Printing CondDB registry");
  for (const auto& e : m_registry) {
    std::print(ost, "\n  - id: {}  r:", e.first);
    for (const IOVEntryT<IASCIICondDbSvc::dbData_t>& r : e.second) {
      std::print (ost, "  {} :: {}", static_cast<std::string>(r.range()), *r.objPtr());
    }
  }

  ATH_MSG_DEBUG( ost.str() );  

  return StatusCode::SUCCESS;

}
//---------------------------------------------------------------------------

StatusCode
ASCIICondDbSvc::readDbFile(const std::string& fname) {

  StatusCode sc(StatusCode::SUCCESS);

  ATH_MSG_DEBUG("reading cond db from \"{}\"", fname);

  std::ifstream ifs (fname);
  std::string line;
  if(ifs.is_open()) {

    IOVEntryT<IASCIICondDbSvc::dbData_t> ie;

    while( getline (ifs, line) ) {
      
      // ignore anything after a "#" and blank lines
      size_t fh = line.find("#");
      if(fh != std::string::npos) 
        line.erase(fh,line.length()-fh);
      if (line.length() == 0) continue;
      
      std::vector<std::string> tokens;
      for (auto&& token : line | std::views::split(' ')) if (!token.empty()) tokens.emplace_back(token.begin(), token.end());
      auto it = tokens.begin();
      
      std::string dbKey = *it;
      
      ++it;
      
      while (it != tokens.end()) {
        if (parse(ie,*it)) {
          m_registry[dbKey].push_back( ie );
        } else {
          ATH_MSG_ERROR( "while reading {} problem parsing  in line {}",
                         fname, *it, line);
          sc = StatusCode::FAILURE;
        }
        ++it;
      }
    }
    ifs.close();
  } else {
    ATH_MSG_ERROR( "unable to open file {}", m_file.value() );
    sc = StatusCode::FAILURE;
  }

  return sc;

}
//---------------------------------------------------------------------------

void
ASCIICondDbSvc::dump() const {

  std::ostringstream ost;
  dump(ost);

  ATH_MSG_INFO( "{}", ost.str() );

}


//---------------------------------------------------------------------------

void
ASCIICondDbSvc::dump(std::ostringstream& ost) const {

  std::lock_guard<std::mutex> lock(m_lock);
  std::println (ost, "ASCIICondDbSvc::dump()");
}

//---------------------------------------------------------------------------

StatusCode
ASCIICondDbSvc::finalize() {

  ATH_MSG_DEBUG( "ASCIICondDbSvc::finalize()" );

  if (msgLvl(MSG::DEBUG)) {
    std::ostringstream ost;
    dump(ost);
    
    ATH_MSG_DEBUG( ost.str() );
  }

  for ( auto e : m_registry ) {
    for ( auto ie : e.second ) {
      delete ie.objPtr();
      ie.setPtr(0);
    }
  }


  return StatusCode::SUCCESS;

}

//---------------------------------------------------------------------------

bool 
ASCIICondDbSvc::parse(EventIDRange& t, const std::string& s) {

  std::smatch m;
  std::regex_match(s,m,rr);

  if (m.size() != 5) { return false; }

  // set run# and timestamp
  EventIDBase start(std::stoi(m[1]), EventIDBase::UNDEFEVT, std::stoi(m[2]));
  EventIDBase   end(std::stoi(m[3]), EventIDBase::UNDEFEVT, std::stoi(m[4]));

  start.set_lumi_block(m_lbn);
  end.set_lumi_block(m_lbn);
  
  t = EventIDRange(start, end);

  return true;

}

//---------------------------------------------------------------------------

bool
ASCIICondDbSvc::parse(IOVEntryT<IASCIICondDbSvc::dbData_t>& ie, const std::string& s) {

  std::smatch m;
  std::regex_match(s,m,ref);

  if (m.size() != 6) { return false; }

  // set run#, lumi and timestamp
  // EventIDBase start(std::stoi(m[1]), EventIDBase::UNDEFEVT, std::stoi(m[2]));
  // EventIDBase   end(std::stoi(m[3]), EventIDBase::UNDEFEVT, std::stoi(m[4]));
  // start.set_lumi_block(m_lbn);
  // end.set_lumi_block(m_lbn);

  // set lumi and Timestamp
  EventIDBase start(0, EventIDBase::UNDEFEVT,
                    std::stoi(m[2]));
  EventIDBase   end(EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT,
                    std::stoi(m[4]));
  start.set_lumi_block(std::stoi(m[1]));
  end.set_lumi_block(std::stoi(m[3]));

  // Set Run/Lumi
  // EventIDBase start(std::stoi(m[1]), EventIDBase::UNDEFEVT);
  // EventIDBase   end(std::stoi(m[3]), EventIDBase::UNDEFEVT);
  // start.set_lumi_block(std::stoi(m[2]));
  // end.set_lumi_block(std::stoi(m[4]));
  
  ie.setRange(EventIDRange(start,end));
  
  IASCIICondDbSvc::dbData_t *v = new IASCIICondDbSvc::dbData_t( std::stof(m[5]) );
  ie.setPtr(v);

  return true;

}

//---------------------------------------------------------------------------

StatusCode
ASCIICondDbSvc::getRange(const std::string& dbKey , const EventContext& ctx,
                  EventIDRange& rng, IASCIICondDbSvc::dbData_t& val) const {

  std::lock_guard<std::mutex> lock(m_lock);

  registry_t::const_iterator itr = m_registry.find(dbKey);

  if (itr == m_registry.end()) {
    ATH_MSG_ERROR( "getRange: no dbKey {} found in registry", dbKey );
    return StatusCode::FAILURE;
  }

  for (const IOVEntryT<IASCIICondDbSvc::dbData_t>& e : itr->second) {
    ATH_MSG_DEBUG( "compare " << e.range() << " with " << ctx.eventID() );
    if (e.range().isInRange(EventIDBase(ctx.eventID()))) {
      rng = e.range();
      val = *(e.objPtr());
      return StatusCode::SUCCESS;
    }
  }

  ATH_MSG_ERROR( "getRange: no range for Time " << ctx.eventID()
                 << " found for dbKey "  << dbKey );

  return StatusCode::FAILURE;
}
