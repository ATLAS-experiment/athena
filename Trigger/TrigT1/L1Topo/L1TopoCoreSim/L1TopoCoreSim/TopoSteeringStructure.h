/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "CxxUtils/checker_macros.h"

#include "L1TopoCommon/StatusCode.h"

#include "TrigConfData/L1Menu.h"
#include "TrigConfData/L1TopoAlgorithm.h"

#include <vector>
#include <map>
#include <string>

namespace TXC {
   class L1TopoMenu;
}

namespace TCS {

   class Connector;
   class DecisionConnector;
   class SortingConnector;
   class CountingConnector;
   class InputConnector;
   class ParameterSpace;
//    class DecisionAlg;
//    class SortingAlg;

   class TopoSteeringStructure {
   public:

      TopoSteeringStructure();

      ~TopoSteeringStructure();

      StatusCode setupFromMenu ATLAS_NOT_THREAD_SAFE (const TrigConf::L1Menu& l1menu, bool legacy = false, bool debug = false);

      // accessors
      bool isConfigured() const { return m_isConfigured; }

      const std::vector<TCS::Connector*> & connectors() const { return m_connectors; }

      const std::map<std::string, TCS::DecisionConnector*> & outputConnectors() const { return m_outputLookup; }

      const std::map<std::string, TCS::CountingConnector*> & countConnectors() const { return m_countLookup; }

      Connector* connector(const std::string & connectorName) const;

      DecisionConnector* outputConnector(const std::string & output);

      CountingConnector* countingConnector(const std::string & output);

      // resets the connectors (status, intermediate TOBs, and decision of algs)
      StatusCode reset();

      // l1menu isolation info
      const std::map<std::string, int> & isolationFW_CTAU() const { return m_isolationFW_CTAU; } 
      const std::map<std::string, int> & isolationFW_CTAU_jTAUCoreScale() const { return m_isolationFW_CTAU_jTAUCoreScale; } 
      const std::map<std::string, int> & isolationFW_JTAU() const { return m_isolationFW_JTAU; } 

      void setIsolationFW_CTAU(const TrigConf::L1Menu& l1menu);
      void setIsolationFW_JTAU(const TrigConf::L1Menu& l1menu);

      // Functions used by HLT seeding
      #ifndef TRIGCONF_STANDALONE

      static void setIsolationFW_CTAU( std::map<std::string, int>& isoFW_CTAU, std::map<std::string, int>& isoFW_CTAU_jTAUCoreScale, const TrigConf::L1ThrExtraInfoBase& menuExtraInfo );

      #endif

      // print
      void print(std::ostream & o) const;

      void printParameters(std::ostream & o) const;

   private:

      SortingConnector* sortingConnector(const std::string & output) const;

      StatusCode addDecisionConnector(DecisionConnector * conn);

      StatusCode addSortingConnector(SortingConnector * conn);

      StatusCode addCountingConnector(CountingConnector * conn);

      StatusCode linkConnectors();

      StatusCode instantiateAlgorithms ATLAS_NOT_THREAD_SAFE (bool debug);

   private:

      bool m_isConfigured  { false };     // set to true after configuration has run

      std::vector<TCS::Connector*> m_connectors; // list of connectors
      
      std::map<std::string, TCS::DecisionConnector*> m_outputLookup; // output connectors (subset of m_connectors) by connector name

      std::map<std::string, TCS::SortingConnector*> m_sortedLookup; // sorting connectors (subset of m_connectors) by connector name

      std::map<std::string, TCS::CountingConnector*> m_countLookup; // counting connectors (subset of m_connectors) by connector name

      std::map<std::string, TCS::InputConnector*> m_inputLookup; // input connectors (subset of m_connectors) by connector name

      std::map<std::string, int> m_isolationFW_CTAU; // FW isolation WPs for cTau
      std::map<std::string, int> m_isolationFW_CTAU_jTAUCoreScale; // FW isolation WPs for cTau (jTAUCore factor)
      std::map<std::string, int> m_isolationFW_JTAU; // FW isolation WPs for jTau

      std::vector<TCS::ParameterSpace*> m_parameters;
      
   };

}
