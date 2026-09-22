///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// AthHistogramming.cxx
// Implementation file for class AthHistogramming
// Author: Karsten Koeneke
///////////////////////////////////////////////////////////////////

// This class' header includes
#include "AthenaBaseComps/AthHistogramming.h"

// Framework includes
#include "AthenaKernel/getMessageSvc.h"
#include <algorithm>

namespace{
  //split a histogram name which contains a path into the separate path/name components
  std::pair<std::string_view, std::string_view>
  splitName(std::string_view name){
    const auto pos  = name.rfind('/');
    if (pos == std::string_view::npos){
      return {{}, name};
    }
    return {name.substr(0,pos), name.substr(pos+1)};
  }
}



///////////////////////////////////////////////////////////////////
// Public methods:
///////////////////////////////////////////////////////////////////

// Constructors
////////////////
AthHistogramming::AthHistogramming( const std::string& name ) :
  m_histSvc ( "THistSvc/THistSvc", name ),
  m_streamName(""),
  m_name ( name ),
  m_msg( Athena::getMessageSvc(), name )
{
}




// Destructor
///////////////
AthHistogramming::~AthHistogramming()
{
  //ATH_MSG_DEBUG ("Calling destructor of AthHistogramming");
  if ( m_msg.level() <= MSG::VERBOSE ) m_msg << MSG::DEBUG << "Calling destructor of AthHistogramming" << endmsg;
}


///////////////////////////////////////////////////////////////////
// Non-const methods:
///////////////////////////////////////////////////////////////////

// To be called by the derived classes to fill the internal configuration
StatusCode AthHistogramming::configAthHistogramming( const ServiceHandle<ITHistSvc>& histSvc,
                                                     const std::string& prefix,          const std::string& rootDir,
                                                     const std::string& histNamePrefix,  const std::string& histNamePostfix,
                                                     const std::string& histTitlePrefix, const std::string& histTitlePostfix )
{
  m_histSvc          = histSvc;
  m_streamName       = prefix;
  m_rootDir          = rootDir;
  m_histNamePrefix   = histNamePrefix;
  m_histNamePostfix  = histNamePostfix;
  m_histTitlePrefix  = histTitlePrefix;
  m_histTitlePostfix = histTitlePostfix;

  return StatusCode::SUCCESS;
}




///////////////////////////////////////////////////////////////////
// Protected methods:
///////////////////////////////////////////////////////////////////


// -----------------------
// For histogramming
// -----------------------


// =============================================================================
// Simplify the booking and registering (into THistSvc) of histograms
// =============================================================================
TH1* 
AthHistogramming::bookGetPointer( TH1& histRef, const std::string & tDir, const std::string & stream ){
  
  const std::string originalName{histRef.GetName()};
  const auto histName = splitName(originalName).second;
  
  const std::string histTitle(histRef.GetTitle());
  std::string bookingString = buildBookingString(originalName, tDir, stream);

  std::string finalHistName{m_histNamePrefix};
  finalHistName += histName;
  finalHistName += m_histNamePostfix;

  histRef.SetTitle((m_histTitlePrefix + histTitle + m_histTitlePostfix).c_str());
  histRef.SetName(finalHistName.c_str());

  const hash_t histHash = this->hash(histName);
  HistMap_t::const_iterator it = m_histMap.find(histHash);
  if (it != m_histMap.end()) {
    m_msg << MSG::WARNING
          << "Detected a hash collision. The hash for the histogram with name=" << histName
          << " already exists and points to a histogram with name=" << it->second->GetName()
          << " NOT going to book the new histogram and returning a NULL pointer!" << endmsg;
    return nullptr;
  }

  if (!histSvc()->regHist(bookingString, &histRef).isSuccess()) {
    m_msg << MSG::WARNING
          << "Problem registering histogram with name " << histName
          << ", name prefix " << m_histNamePrefix
          << ", title " << histTitle
          << ", title prefix " << m_histTitlePrefix
          << ", and title postfix " << m_histTitlePostfix
          << " in " << m_name << "!" << endmsg;
    return nullptr;
  }

  m_histMap.insert(m_histMap.end(), std::pair<const hash_t, TH1*>(histHash, &histRef));

  return &histRef;
}

TEfficiency*
AthHistogramming::bookGetPointer(TEfficiency& effRef, const std::string & tDir, const std::string & stream)
{
  std::string originalName{effRef.GetName()};
  const auto effName = splitName(originalName).second;
  const std::string effTitle(effRef.GetTitle());
  std::string bookingString = buildBookingString(originalName, tDir, stream);
  std::string finalEffName{m_histNamePrefix};
  finalEffName += effName;
  finalEffName += m_histNamePostfix;
  effRef.SetTitle((m_histTitlePrefix + effTitle + m_histTitlePostfix).c_str());
  effRef.SetName(finalEffName.c_str());

  const hash_t effHash = this->hash(effName);
  EffMap_t::const_iterator it = m_effMap.find(effHash);
  if (it != m_effMap.end()) {
    m_msg << MSG::WARNING
          << "Detected a hash collision. The hash for the TEfficiency with name=" << effName
          << " already exists and points to a TEfficiency with name=" << it->second->GetName()
          << " NOT going to book the new TEfficiency and returning a NULL pointer!" << endmsg;
    return nullptr;
  }

  if (!histSvc()->regEfficiency(bookingString, &effRef).isSuccess()) {
    m_msg << MSG::WARNING
          << "Problem registering TEfficiency with name " << effName
          << ", name prefix " << m_histNamePrefix
          << ", title " << effTitle
          << ", title prefix " << m_histTitlePrefix
          << ", and title postfix " << m_histTitlePostfix
          << " in " << m_name << "!" << endmsg;
    return nullptr;
  }

  m_effMap.insert(m_effMap.end(), std::pair<const hash_t, TEfficiency*>(effHash, &effRef));

  return &effRef;
}


// =============================================================================
// Simplify the retrieval of registered histograms of any type
// =============================================================================
TH1*
AthHistogramming::hist(std::string_view histName, const std::string& tDir,
 const std::string& stream){

  const auto histBaseName = splitName(histName).second;
  std::string bookingString = buildBookingString(histName, tDir, stream, false);

  const hash_t histHash = this->hash(histBaseName);

  HistMap_t::const_iterator it = m_histMap.find(histHash);
  if (it == m_histMap.end()) {
    TH1* histPointer(nullptr);

    if (!histSvc()->getHist(bookingString, histPointer).isSuccess()) {
      std::string prefixedBookingString = buildBookingString(histName, tDir, stream, true);
      if (!histSvc()->getHist(prefixedBookingString, histPointer).isSuccess()) {
        m_msg << MSG::WARNING
              << "Problem retrieving the histogram with name (including pre- and post-fixes) "
              << prefixedBookingString
              << " or with name " << histName
              << " in " << m_name << "... it doesn't exist, neither in the cached map nor in the THistSvc!"
              << " Will return an NULL pointer... you have to handle it correctly!" << endmsg;
        return nullptr;
      }
    }

    m_histMap.insert(m_histMap.end(), std::pair<const hash_t, TH1*>(histHash, histPointer));
    return histPointer;
  }

  return it->second;
}

TEfficiency* AthHistogramming::efficiency( const std::string& effName, const std::string& tDir, const std::string& stream )
{
  // Build a 32 bit hash out of the name
  const hash_t effHash = this->hash(effName);

  // See if this entry exists in the map
  EffMap_t::const_iterator it = m_effMap.find( effHash );
  if ( it == m_effMap.end() ) // It doesn't exist!
    { // Let's see into the THistSvc if somebody else has registered the TEfficiency...
      // Massage the final string to book things
      std::string bookingString = buildBookingString(effName, tDir, stream ,false);

      TEfficiency* effPointer{};
      if ( !((histSvc()->getEfficiency(bookingString, effPointer)).isSuccess()) )
        {
          // Book things
          std::string bookingString = buildBookingString( effName, tDir, stream, true );

          if ( !((histSvc()->getEfficiency(bookingString, effPointer)).isSuccess()) )
            {
              m_msg << MSG::WARNING
                    << "Problem retrieving the TEfficiency with name (including pre- and post-fixes) "
                    << bookingString
                    << " or with name " << effName
                    << " in " << m_name << "... it doesn't exist, neither in the cached map nor in the THistSvc!"
                    << " Will return an NULL pointer... you have to handle it correctly!" << endmsg;
              return NULL;
            }
          // If we get to here, we actually found the TEfficiency in the THistSvc.
          // So let's add it to the local cache map and return its pointer
          m_effMap.insert( m_effMap.end(), std::pair< const hash_t, TEfficiency* >( effHash, effPointer ) );
          return effPointer;
        }
      // If we get to here, we actually found the TEfficiency in the THistSvc.
      // So let's add it to the local cache map and return its pointer
      m_effMap.insert( m_effMap.end(), std::pair< const hash_t, TEfficiency* >( effHash, effPointer ) );
      return effPointer;
    }

  // Return the pointer to the TEfficiency that we got from the local cache map
  return it->second;
}





// -----------------------
// For TTrees
// -----------------------

// =============================================================================
// Simplify the booking and registering (into THistSvc) of TTrees
// =============================================================================
TTree* AthHistogramming::bookGetPointer( const TTree& treeRef, const std::string & tDir, const std::string & stream )
{
  // Get a pointer
  const TTree* treePointer = &treeRef;

  // Check that we got a valid pointer
  if ( !treePointer )
    {
      m_msg << MSG::WARNING
            << "We got an invalid TTree pointer in the BookGetPointer(TTree*) method of the class" << m_name
            << "!" << endmsg;
      return NULL;
    }

  // Modify the name and title according to the prefixes of this classes instance
  std::string treeName  = treePointer->GetName();
  const std::string treeTitle = treePointer->GetTitle();

  // Check if the hash for this treeName already exists, i.e., if we have a hash collision
  const hash_t treeHash = this->hash(treeName);
  TreeMap_t::const_iterator it = m_treeMap.find( treeHash );
  if ( it != m_treeMap.end() ) // It does exist!
    {
      m_msg << MSG::WARNING
            << "Detected a hash collision. The hash for the TTree with name=" << treeName
            << " already exists and points to a TTree with name=" << it->second->GetName()
            << " NOT going to book the new histogram and returning a NULL pointer!" << endmsg;
      return NULL;
    }

  // Create a clone that has the new name
  TTree* treeClone = dynamic_cast< TTree* >( treePointer->Clone(treeName.c_str()) );
  if( !treeClone )
    {
      m_msg << MSG::WARNING
            << "We couldn't clone the TTree in the BookGetPointer(TTree&) method of the class" << m_name
            << "!" << endmsg;
      return NULL;
    }
  treeClone->SetTitle (treeTitle.c_str());

  // Massage the final string to book things
  std::string bookingString = buildBookingString( treeName, tDir, stream );

  // Register the TTree into the THistSvc
  if ( !((histSvc()->regTree(bookingString, treeClone)).isSuccess()) )
    {
      m_msg << MSG::WARNING
            << "Problem registering TTree with name " << treeName
            << ", title " << treeTitle
            << " in " << m_name << "!" << endmsg;
      return NULL;
    }

  // Also register it in the local map of string to pointer
  m_treeMap.insert( m_treeMap.end(), std::pair< const hash_t, TTree* >( treeHash, treeClone ) );

  return treeClone;
}



// =============================================================================
// Simplify the retrieval of registered TTrees
// =============================================================================
TTree* AthHistogramming::tree( const std::string& treeName, const std::string& tDir, const std::string& stream )
{
  // Build a 32 bit hash out of the name
  const hash_t treeHash = this->hash(treeName);
  // See if this entry exists in the map
  TreeMap_t::const_iterator it = m_treeMap.find( treeHash );
  if ( it == m_treeMap.end() ) // It doesn't exist!
    { // Let's see into the THistSvc if somebody else has registered the TTree...
      // Massage the final string to book things
      std::string bookingString = buildBookingString( treeName, tDir, stream);

      TTree* treePointer(NULL);
      if ( !((histSvc()->getTree(bookingString, treePointer)).isSuccess()) )
        {
          m_msg << MSG::WARNING
                << "Problem retrieving the TTree with name " << treeName
                << " in " << m_name << "... it doesn't exist, neither in the cached map nor in the THistSvc!"
                << " Will return an NULL pointer... you have to handle it correctly!" << endmsg;
          return NULL;
        }
      // If we get to here, we actually found the TTree in the THistSvc.
      // So let's add it to the local cache map and return its pointer
      m_treeMap.insert( m_treeMap.end(), std::pair< const hash_t, TTree* >( treeHash, treePointer ) );
      return treePointer;
    }

  // Return the pointer to the TTree that we got from the local cache map
  return it->second;
}





// -----------------------
// For TGraphs
// -----------------------

// =============================================================================
// Simplify the booking and registering (into THistSvc) of TGraphs
// =============================================================================
TGraph* AthHistogramming::bookGetPointer( const TGraph& graphRef, const std::string & tDir, const std::string & stream )
{
  // Get a pointer
  const TGraph* graphPointer = &graphRef;

  // Check that we got a valid pointer
  if ( !graphPointer )
    {
      m_msg << MSG::WARNING
            << "We got an invalid TGraph pointer in the BookGetPointer(TGraph*) method of the class" << m_name
            << "!" << endmsg;
      return NULL;
    }

  // Modify the name and title according to the prefixes of this classes instance
  std::string graphName  = graphPointer->GetName();
  const std::string graphTitle = graphPointer->GetTitle();

  // Check if the hash for this graphName already exists, i.e., if we have a hash collision
  const hash_t graphHash = this->hash(graphName);
  GraphMap_t::const_iterator it = m_graphMap.find( graphHash );
  if ( it != m_graphMap.end() ) // It does exist!
    {
      m_msg << MSG::WARNING
            << "Detected a hash collision. The hash for the TGraph with name=" << graphName
            << " already exists and points to a TGraph with name=" << it->second->GetName()
            << " NOT going to book the new histogram and returning a NULL pointer!" << endmsg;
      return NULL;
    }

  // Create a clone that has the new name
  TGraph* graphClone = dynamic_cast< TGraph* >( graphPointer->Clone((m_histNamePrefix+graphName+m_histNamePostfix).c_str()) );
  if( !graphClone )
    {
      m_msg << MSG::WARNING
            << "We couldn't clone the TGraph in the BookGetPointer(TGraph&) method of the class" << m_name
            << "!" << endmsg;
      return NULL;
    }
  graphClone->SetTitle ((m_histTitlePrefix+graphTitle+m_histTitlePostfix).c_str());

  // Massage the final string to book things
  std::string bookingString = buildBookingString( graphName, tDir, stream );

  // Register the TGraph into the THistSvc
  if ( !((histSvc()->regGraph(bookingString, graphClone)).isSuccess()) )
    {
      m_msg << MSG::WARNING
            << "Problem registering TGraph with name " << graphName
            << ", title " << graphTitle
            << " in " << m_name << "!" << endmsg;
      return NULL;
    }

  // Also register it in the local map of string to pointer
  m_graphMap.insert( m_graphMap.end(), std::pair< const hash_t, TGraph* >( graphHash, graphClone ) );

  return graphClone;
}


// =============================================================================
// Simplify the retrieval of registered TGraphs
// =============================================================================
TGraph* AthHistogramming::graph( const std::string& graphName, const std::string& tDir, const std::string& stream )
{
  // Build a 32 bit hash out of the name
  const hash_t graphHash = this->hash(graphName);

  // See if this entry exists in the map
  GraphMap_t::const_iterator it = m_graphMap.find( graphHash );
  if ( it == m_graphMap.end() ) // It doesn't exist!
    { // Let's see into the THistSvc if somebody else has registered the TGraph...

      // Massage the final string to book things
      std::string bookingString = buildBookingString( graphName, tDir, stream, false);

      TGraph* graphPointer(nullptr);
      if ( !((histSvc()->getGraph(bookingString, graphPointer)).isSuccess()) )
        {
          // Massage the final string to book things
          std::string bookingString = buildBookingString( graphName, tDir, stream, true );

          if ( !((histSvc()->getGraph(bookingString, graphPointer)).isSuccess()) )
            {
              m_msg << MSG::WARNING
                    << "Problem retrieving the TGraph with name (including pre- and post-fixes) "
                    << bookingString
                    << " or with name " << graphName
                    << " in " << m_name << "... it doesn't exist, neither in the cached map nor in the THistSvc!"
                    << " Will return an NULL pointer... you have to handle it correctly!" << endmsg;
              return nullptr;
            }
          // If we get to here, we actually found the TGraph in the THistSvc.
          // So let's add it to the local cache map and return its pointer
          m_graphMap.insert( m_graphMap.end(), std::pair< const hash_t, TGraph* >( graphHash, graphPointer ) );
          return graphPointer;
        }
      // If we get to here, we actually found the TGraph in the THistSvc.
      // So let's add it to the local cache map and return its pointer
      m_graphMap.insert( m_graphMap.end(), std::pair< const hash_t, TGraph* >( graphHash, graphPointer ) );
      return graphPointer;
    }


  // Return the pointer to the TGraph that we got from the local cache map
  return it->second;
}










///////////////////////////////////////////////////////////////////
// Private methods:
///////////////////////////////////////////////////////////////////

// =============================================================================
// Helper method to build the final string to be passed to the THistSvc
// =============================================================================
std::string
AthHistogramming::buildBookingString(std::string_view histName, std::string_view tDir,
 std::string_view stream, bool usePrefixPostfix) const {
  if (tDir.empty()) {
    tDir = m_rootDir;
  }
  if (stream.empty()) {
    stream = m_streamName;
  }
  const auto [histDir, baseName] = splitName(histName);
  std::string bookingString{"/"};
  bookingString += stream;
  bookingString += '/';
  bookingString += tDir;
  if (!histDir.empty()) {
    bookingString += '/';
    bookingString += histDir;
  }
  bookingString += '/';
  if (usePrefixPostfix) {
    bookingString += m_histNamePrefix;
  }
  bookingString += baseName;
  if (usePrefixPostfix) {
    bookingString += m_histNamePostfix;
  }
  const auto tail = std::ranges::unique(
    bookingString, [](char lhs, char rhs) { return lhs == '/' && rhs == '/'; });
  bookingString.erase(tail.begin(), tail.end());
  return bookingString;
}
