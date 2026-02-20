/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <iomanip>
#include <bitset>

#include "L1TopoCoreSim/GlobalOutput.h"
#include "L1TopoCoreSim/DecisionConnector.h"
#include "L1TopoCoreSim/CountingConnector.h"
#include "L1TopoCommon/Exception.h"
#include "L1TopoInterfaces/DecisionAlg.h"
#include "L1TopoInterfaces/CountingAlg.h"

using namespace std;
using namespace TCS;

void
GlobalOutput::setDecisionTriggerLines(const vector<TrigConf::TriggerLine> & triggers) {

  m_triggersDec = triggers;
  vector<string> l1connNames = {};
  for (const TrigConf::TriggerLine & trigger : triggers){
    auto it = find(l1connNames.begin(), l1connNames.end(), trigger.connName());
    if (it == l1connNames.end()){
      l1connNames.push_back(trigger.connName());
      m_decision[trigger.connName()] = 0;
    }
  }
}

void
GlobalOutput::setMultiplicityTriggerLines(const vector<TrigConf::TriggerLine> & triggers) {

  m_triggersCount = triggers;
  vector<string> l1connNames = {};
  for (const TrigConf::TriggerLine & trigger : triggers){
    auto it = find(l1connNames.begin(), l1connNames.end(), trigger.connName());
    if (it == l1connNames.end()){
      l1connNames.push_back(trigger.connName());
      m_count[trigger.connName()] = 0;
    }
  }
}
  
uint64_t 
GlobalOutput::decision_field(const std::string & l1connName) const {
  if (m_decision.find(l1connName) != m_decision.end()) {
    return m_decision.find(l1connName)->second;
  }else{
    TRG_MSG_WARNING("Connector name " << l1connName << " unknown");
    return 0;
  }
}

uint32_t
GlobalOutput::decision_field(const std::string& l1connName, unsigned int clock) const {
  if (m_decision.find(l1connName) != m_decision.end()) {
    if(clock==0) {
      // lower 32 bit
      return static_cast<uint32_t>(m_decision.at(l1connName) & 0xffffffff);
    } else {
      // upper 32 bit
      uint64_t clock1 = m_decision.at(l1connName) & 0xffffffff00000000;
      return static_cast<uint32_t>(clock1 >> 32);
    }
  }else{
    TRG_MSG_WARNING("Connector name " << l1connName << " unknown");
    return 0;
  }
}

std::bitset<128>
GlobalOutput::count_field(const std::string& l1connName) const {
  if (m_count.find(l1connName) != m_count.end()) {
    return m_count.find(l1connName)->second;
  }else{
    TRG_MSG_WARNING("Connector name " << l1connName << " unknown");
    return 0;
  }
}

uint64_t 
GlobalOutput::overflow_field(const std::string& l1connName) const {
  if (m_overflow.find(l1connName) != m_overflow.end()) {
    return m_overflow.find(l1connName)->second;
  }else{
    TRG_MSG_WARNING("Connector name " << l1connName << " unknown");
    return 0;
  }
}

uint32_t
GlobalOutput::overflow_field(const std::string& l1connName, unsigned int clock) const {
   if (m_overflow.find(l1connName) != m_overflow.end()) {
      if(clock==0) {
         // lower 32 bit
         return static_cast<uint32_t>(m_overflow.find(l1connName)->second & 0xffffffff);
      } else {
         // upper 32 bit
         uint64_t clock1 = m_overflow.find(l1connName)->second & 0xffffffff00000000;
         return static_cast<uint32_t>(clock1 >> 32);
      }
   }else{
    TRG_MSG_WARNING("Connector name " << l1connName << " unknown");
    return 0;
  }
}

uint64_t 
GlobalOutput::ambiguity_field(const std::string& l1connName) const {
  if (m_ambiguity.find(l1connName) != m_ambiguity.end()) {
    return m_ambiguity.find(l1connName)->second;
  }else{
    TRG_MSG_WARNING("Connector name " << l1connName << " unknown");
    return 0;
  }
}

uint32_t
GlobalOutput::ambiguity_field(const std::string& l1connName, unsigned int clock) const {
   if (m_ambiguity.find(l1connName) != m_ambiguity.end()) {
      if(clock==0) {
         // lower 32 bit
         return static_cast<uint32_t>(m_ambiguity.find(l1connName)->second & 0xffffffff);
      } else {
         // upper 32 bit
         uint64_t clock1 = m_ambiguity.find(l1connName)->second & 0xffffffff00000000;
         return static_cast<uint32_t>(clock1 >> 32);
      }
   }else{
    TRG_MSG_WARNING("Connector name " << l1connName << " unknown");
    return 0;
  }
}

GlobalOutput::GlobalOutput(const std::string &name) :
   TrigConfMessaging(name)
{}

/****************************************************************
 *
 * Fill the decision bits with the pass/fail of all decision algs.
 * This is called by the TopoSteering after the connectors have
 * been executed.
 *
 ****************************************************************/

TCS::StatusCode
GlobalOutput::collectOutput(const set<DecisionConnector*> & outConn, const set<CountingConnector*> & countConn) {
   resetOutput();

   for( const DecisionConnector * conn : outConn ) {

      const Decision& dec = conn->decision();

      unsigned int pos = 0; // for multi-output algorithms pos is the output index
      for(const TrigConf::TriggerLine & trigger : conn->triggers() ) {
         unsigned int position = trigger.flatindex();

         uint64_t & l1connectorDec = m_decision[trigger.connName()];
         uint64_t & l1connectorOvf = m_overflow[trigger.connName()];
         uint64_t & l1connectorAmb = m_ambiguity[trigger.connName()];
         uint64_t mask(0x1);

         if( dec.bit(pos++) )  // bit set?
            l1connectorDec |= (mask << position);
         if( dec.overflow())
            l1connectorOvf |= (mask << position);
         if( dec.ambiguity())
            l1connectorAmb |= (mask << position);
      }

   }

   for   (const CountingConnector * conn : countConn) {

      const Count& count = conn->count();

      const TrigConf::TriggerLine & trigger = conn->triggers().at(0); // counting connectors only have one trigger line

      std::bitset<128> & l1connectorCount =  m_count[trigger.connName()];

      l1connectorCount |= count.getCountBits();

   }

   m_valid = true;
   return TCS::StatusCode::SUCCESS;
}


TCS::StatusCode
GlobalOutput::resetOutput() {
  m_decision.clear();
  m_overflow.clear();
  m_ambiguity.clear();
  m_count.clear();
  
  m_valid = false;
  return TCS::StatusCode::SUCCESS;
}


namespace TCS {


std::ostream&
operator<<(std::ostream& o, const TCS::GlobalOutput& dec){
  // Save stream state
  const auto flags = o.flags();
  const auto fill  = o.fill();
  const auto width = o.width();

  if (!dec.isValid())
    o << "Note that the overall decision has not been calculated\n";

  for (auto const& itdec : dec.m_decision) {
    o << "Overall decision for connector " << itdec.first << ": 0x"
      << std::right << std::hex << std::setfill('0') << std::setw(16)
      << dec.decision_field(itdec.first)
      << std::dec << std::setfill(' ') << '\n';
  }

  if (dec.isValid()) {
    for (const TrigConf::TriggerLine& trigger : dec.m_triggersDec) {
      unsigned int position = trigger.flatindex();
      o << "  " << std::setw(30) << std::left << trigger.name()
        << "  " << (dec.passed(trigger.connName(), position) ? "pass" : "fail")
        << '\n';
    }
  } else {
    for (const TrigConf::TriggerLine& trigger : dec.m_triggersDec) {
      o << "  " << std::setw(30) << std::left << trigger.name()
        << "  unset\n";
    }
  }

  // Restore stream state
  o.flags(flags);
  o.fill(fill);
  o.width(width);

  return o;
}

//----------------------------------------------------------
void
GlobalOutput::print() const {

   if(!isValid()) 
      TRG_MSG_INFO("Note that the overall decision has not been calculated");

   for(auto const& dec : m_decision)
      TRG_MSG_INFO("Overall decision from connector " << dec.first << ": 0x" << right << hex << setfill('0') << setw(16) << decision_field(dec.first) << std::dec << setfill(' '));

   if(isValid()) {
      for(const TrigConf::TriggerLine & trigger : m_triggersDec){
         unsigned int position = trigger.flatindex();
 	 TRG_MSG_INFO("      " << setw(30) << left << trigger.name() << "  " << (passed(trigger.connName(), position) ? "pass" : "fail") );}
   } else {
      for(const TrigConf::TriggerLine & trigger : m_triggersDec)
         TRG_MSG_INFO("      " << setw(30) << left << trigger.name() << "  unset" );
   }
}

}
