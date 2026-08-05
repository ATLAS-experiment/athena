/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////// 
// DecayParser.cxx 
// Implementation file for class DecayParser
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 

// STL includes
#include <algorithm>
#include <cctype>
#include <iostream>
#include <ranges>
#include <stdexcept>
#include <sstream>
#include <string_view>

// McParticleUtils includes
#include "McParticleUtils/DecayParser.h"

namespace {

std::vector<McUtils::Strings> process_block(std::string_view cmd)
{
  std::vector<McUtils::Strings> result;
  if (cmd.empty()) {
    return result;
  }

  std::vector<std::string> slots;
  for (auto const& token : cmd | std::views::split('+')) {
    slots.emplace_back(token.begin(), token.end());
  }

  result.reserve(slots.size());
  for (auto const& slot : slots) {
    McUtils::Strings candidates;
    for (auto const& token : slot | std::views::split('|')) {
      candidates.emplace_back(token.begin(), token.end());
    }
    std::ranges::sort(candidates);
    auto [first, last] = std::ranges::unique(candidates);
    candidates.erase(first, last);
    result.emplace_back(std::move(candidates));
  }

  return result;
}

} // anonymous namespace

/////////////////////////////////////////////////////////////////// 
/// Public methods: 
/////////////////////////////////////////////////////////////////// 

/// Constructors
////////////////

DecayParser::DecayParser( const std::string& cmd ) :
  m_parents  ( ),
  m_children ( )
{
  parse(cmd);
}


/// Destructor
///////////////
DecayParser::~DecayParser() 
{
}

/////////////////////////////////////////////////////////////////// 
/// Const methods: 
///////////////////////////////////////////////////////////////////
void DecayParser::dump() const
{
  std::cout << "--- Parents ---" << std::endl;
  printMcUtilsStrings( m_parents );

  std::cout << "--- Children ---" << std::endl;
  printMcUtilsStrings( m_children );
}

int DecayParser::pdgId( const std::string& pdgIdString ) const
{
  int pdgID = 0;
  int iPDG = 0;
  std::stringstream( pdgIdString ) >> iPDG;
  pdgID = iPDG;

  return pdgID;
}
/////////////////////////////////////////////////////////////////// 
/// Non-const methods: 
/////////////////////////////////////////////////////////////////// 
void DecayParser::parse( const std::string& inputCmd ) 
{
  if ( inputCmd.empty() ) {
    return;
  }

  std::string cmd;
  cmd.reserve(inputCmd.size());
  for (unsigned char c : inputCmd) {
    if (!std::isspace(c)) {
      cmd.push_back(c);
    }
  }
  if (cmd.empty()) {
    return;
  }

  // Reset the parents and children lists
  m_parents.clear();
  m_children.clear();

  if (cmd == "->") {
    return;
  }

  const std::size_t arrowPos = cmd.find("->");
  if (arrowPos == std::string::npos) {
    std::string error = "missing '->' in command [" + inputCmd + "]";
    throw std::runtime_error(error);
  }

  if (cmd.find("->", arrowPos + 2) != std::string::npos) {
    std::string error = "multiple '->' separators in command [" + inputCmd + "]";
    throw std::runtime_error(error);
  }

  const std::string_view cmdView{cmd};
  const auto parentsBlock  = cmdView.substr(0, arrowPos);
  const auto childrenBlock = cmdView.substr(arrowPos + 2);

  m_parents  = process_block(parentsBlock);
  m_children = process_block(childrenBlock);
}


void 
DecayParser::printMcUtilsStrings( const std::vector<McUtils::Strings>& list ) const
{
  unsigned int iSlot = 0;
  for( std::vector<McUtils::Strings>::const_iterator itr = list.begin();
       itr != list.end();
       ++itr,++iSlot ) {
    std::stringstream iSlotStr;
    iSlotStr << iSlot;
    const McUtils::Strings::const_iterator candEnd = itr->end();
    std::cout << "slot #" << iSlotStr.str() << ": candidates= [ ";
    for( McUtils::Strings::const_iterator candidate = itr->begin();
	 candidate != candEnd;
	 ++candidate ) {
      std::cout << *candidate;
      if ( candidate+1 != candEnd ) {
	std::cout << " | ";
      }
    }
    std::cout << " ]" << std::endl;
  }
  return;
}


/////////////////////////////////////////////////////////////////// 
// Operators: 
///////////////////////////////////////////////////////////////////
DecayParser & DecayParser::operator=(const DecayParser& rhs )
{
  if ( this != &rhs ) {
    m_parents      = rhs.m_parents;
    m_children     = rhs.m_children;
  }
  return *this;
}
