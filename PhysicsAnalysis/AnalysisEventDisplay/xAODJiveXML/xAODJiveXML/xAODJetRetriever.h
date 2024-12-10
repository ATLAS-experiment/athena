/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JIVEXML_XAODJETRETRIEVER_H
#define JIVEXML_XAODJETRETRIEVER_H

#include <string>
#include <vector>
#include <map>

#include "JiveXML/IDataRetriever.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "xAODJet/JetContainer.h" 
#include "AsgTools/AnaToolHandle.h"
#include "FTagAnalysisInterfaces/IBTaggingSelectionTool.h"


namespace JiveXML{
  
  /**
   * @class xAODJetRetriever
   * @brief Retrieves all @c Jet @c objects (JetAODCollection etc.)
   *
   *  - @b Properties
   *    - StoreGateKey: First collection to be retrieved, displayed
   *      in Atlantis without switching. All other collections are 
   *      also retrieved.
   *
   *  - @b Retrieved @b Data
   *    - Usual four-vectors: phi, eta, et etc.
   *    - Associations for clusters and tracks via ElementLink: key/index scheme
   */
  class xAODJetRetriever : virtual public IDataRetriever,
                                   public AthAlgTool {
    
    public:
      
      /// Standard Constructor
      xAODJetRetriever(const std::string& type,const std::string& name,const IInterface* parent);
      
      /// Retrieve all the data
      virtual StatusCode retrieve(ToolHandle<IFormatTool> &FormatTool); 
      const DataMap getData(const xAOD::JetContainer*, const std::string& jetkey);
    
      /// Return the name of the data type
      virtual std::string dataTypeName() const { return "Jet"; };

      virtual StatusCode initialize();

    private:
      Gaudi::Property<std::string> m_sgKeyFavourite{
          this, "FavouriteJetCollection", "AntiKt4TopoEMJets", "Collection to be first in output, shown in Atlantis without switching"};
      Gaudi::Property<std::vector<std::string>> m_otherKeys{
          this, "OtherJetCollections", {}, "Other collections to be retrieved. If list left empty, all available retrieved"};
      Gaudi::Property<bool> m_doWriteHLT{
          this, "DoWriteHLT", false, "Write out HLTAutokey object information. False by default."};
      Gaudi::Property<bool> m_writeJetQuality{
          this, "WriteJetQuality", false, "Write out extended jet quality information. False by default."};
      Gaudi::Property<std::string> m_tracksName{
          this, "TracksName", "InDetTrackParticles_xAOD", "Name of the track container to be retrieved"};
      Gaudi::Property<std::vector<std::string>> m_bTaggerNames{
          this, "BTaggerNames", {}, "Names of the b-taggers to be retrieved"};
      Gaudi::Property<std::vector<std::string>> m_CDIPaths{
          this, "CDIPaths", {}, "Paths to the CDI files storing the b-tagger properties"};

      std::unordered_map<std::string, asg::AnaToolHandle<IBTaggingSelectionTool>> m_btagSelTools;
      unsigned int m_nTaggers=0;
      
      };
}
#endif
