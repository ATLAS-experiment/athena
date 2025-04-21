/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "SiSpacePointRetriever.h"
#include "StoreGate/DataHandle.h"
#include "JiveXML/DataType.h"

#include "AthContainers/DataVector.h"
#include "StoreGate/ReadHandle.h"

#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "TrkSpacePoint/SpacePoint.h"
#include "TrkSpacePoint/SpacePointCollection.h"
#include "TrkSpacePoint/SpacePointCLASS_DEF.h"
#include "TrkPrepRawData/PrepRawData.h"

#include "CLHEP/Units/SystemOfUnits.h"

namespace JiveXML 
{

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   **/
  SiSpacePointRetriever::SiSpacePointRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent)
  {

    //Declare the interface
    declareInterface<IDataRetriever>(this);
    
    //And the properties
    declareProperty("PixelSpacePoints"     , m_PixelSPContainerName = std::string("PixelSpacePoints"));
    declareProperty("SCTSpacePoints"       , m_SCTSPContainerName = std::string("SCT_SpacePoints"));
    declareProperty("PRD_TruthPixel"       , m_PixelPRDTruthName = std::string("PRD_MultiTruthPixel")); 
    declareProperty("PRD_TruthSCT"         , m_SCTPRDTruthName = std::string("PRD_MultiTruthSCT"));   
  }

  //Namespace for the helper functions
  namespace SiSpacePointRetrieverHelpers {
    
    /**
     * Get the barcodes of associated truth particles for a SpacePoint ( we require both clusters, in case there are)
     * @param idFirst the SpacePoint identifier of first cluster
     * @param idSecond the SpacePoint identifier of second cluster (only for SCT)
     * @param truthColl the truth collection
     * @param barcodes the date vector to which to append the barcodes
     * @return number of associated truth particles
     */
    unsigned int getTruthBarcodes( const Identifier idFirst, const Identifier idSecond, const PRD_MultiTruthCollection* truthColl, DataVect& barcodes) {
      
      //Make life easier
      using PRDTruthIter = PRD_MultiTruthCollection::const_iterator;


      /**
       * NOTE: This function could be simplified if we could directly insert
       * into the DataVect, instead of barcodesCommon. At the moment, this
       * doesnt work as we are lacking operator< and operator== for DataType
       */

      //Sets of barcodes associated with first, second and both cluster
      std::set<int> barcodesFirst;
      std::set<int> barcodesSecond;
      std::set<int> barcodesCommon;

      //Get the set of particle barcodes associated with the first cluster identifier
      std::pair<PRDTruthIter,PRDTruthIter> equalRangeFirst = truthColl->equal_range(idFirst);
      for(PRDTruthIter TruthCollItr=equalRangeFirst.first; TruthCollItr!= equalRangeFirst.second; ++TruthCollItr)
        barcodesFirst.insert(TruthCollItr->second.barcode());

      //Check if we have only have one valid cluster identifier
      if (! idSecond.is_valid()){

        //Make this our list of barcodes (swap is O(1))
        barcodesCommon.swap(barcodesFirst);

      //otherwise only store barcodes associated with both identifiers
      } else {

        //Get the set of particle barcodes associated with the second cluster identifier
        std::pair<PRDTruthIter,PRDTruthIter> equalRangeSecond = truthColl->equal_range(idSecond);
        for(PRDTruthIter TruthCollItr=equalRangeSecond.first; TruthCollItr!= equalRangeSecond.second; ++TruthCollItr)
          barcodesSecond.insert(TruthCollItr->second.barcode());

        //Copy the list of particle barcodes that are associated with both clusters
        std::set_intersection(barcodesFirst.begin(), barcodesFirst.end(), barcodesSecond.begin(), barcodesSecond.end(), 
                              std::insert_iterator< std::set<int> >(barcodesCommon,barcodesCommon.begin()) );
      }

      //Finally add the list of barcodes to our DataVect
      for (const auto barcodeCommon : barcodesCommon) barcodes.push_back(DataType(barcodeCommon));

      //return the number of added barcodes
      return barcodesCommon.size();
    }

  }//helpers namespace

  /**
   * Implementation of DataRetriever interface
   * @param FormatTool the tool that will create formated output from the DataMap
   **/
  StatusCode SiSpacePointRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {

    //be verbose
    ATH_MSG_DEBUG( "Retrieving " << dataTypeName() ); 
   
    /**
     * Try to retrieve all the relevant collections
     */

    SG::ReadHandle<SpacePointContainer> PixelSPContainer(m_PixelSPContainerName);
    if ( not PixelSPContainer.isValid() )
      ATH_MSG_WARNING( "Unable to retrieve SpacePoint container with name " << m_PixelSPContainerName.key() );

    SG::ReadHandle<PRD_MultiTruthCollection> PixelPRDTruthColl;
    if (m_usePixelTruthMap) {
      PixelPRDTruthColl = SG::makeHandle(m_PixelPRDTruthName);
      if ( not PixelPRDTruthColl.isValid() )
        ATH_MSG_WARNING( "Unable to retrieve PRD_MultiTruth collection with name " << m_PixelPRDTruthName.key() );
    }

    SG::ReadHandle<SpacePointContainer> SCTSPContainer(m_SCTSPContainerName);
    if ( not SCTSPContainer.isValid() )
      ATH_MSG_WARNING( "Unable to retrieve SpacePoint container with name " << m_SCTSPContainerName.key() );

    SG::ReadHandle<PRD_MultiTruthCollection> SCTPRDTruthColl;
    if (m_useSCTTruthMap) {
      SCTPRDTruthColl = SG::makeHandle(m_SCTPRDTruthName);
      if ( not SCTPRDTruthColl.isValid() )        
        ATH_MSG_WARNING( "Unable to retrieve PRD_MultiTruth collection with name " << m_SCTPRDTruthName.key() );
    }

    /**
     * Now make a list of SpacePoint - PRDTruth collection pairs to run over
     */
    using SpacePointTruthPair = std::pair<const SpacePointContainer *, const PRD_MultiTruthCollection *>;
    std::vector<SpacePointTruthPair> SpacePointTruthPairList;

    //Add Pixel if there is a collection
    if (PixelSPContainer.isValid())
      SpacePointTruthPairList.emplace_back(PixelSPContainer.cptr(), m_usePixelTruthMap ? PixelPRDTruthColl.cptr() : nullptr);
    
    //Add SCT if there is a collection
    if (SCTSPContainer.isValid())
      SpacePointTruthPairList.emplace_back(SCTSPContainer.cptr(), m_useSCTTruthMap ? SCTPRDTruthColl.cptr() : nullptr);
    
    /**
     * Found out how much space we will need
     */
    int NSpacePoints = 0;
    //Loop over all SpacePoint - PRDTruth pairs
    for (const auto &SPTruthPair : SpacePointTruthPairList) {

      //Add up the size of the SpacePoint collections in the container
      for (const auto SpacePoint : *(SPTruthPair.first))
        NSpacePoints += SpacePoint->size();
    }

    ATH_MSG_DEBUG(  "Counted  " << NSpacePoints << " in total" );

    /**
     * Declare all the data we want to retrieve
     */
    DataVect x; x.reserve(NSpacePoints);
    DataVect y; y.reserve(NSpacePoints);
    DataVect z; z.reserve(NSpacePoints);
    DataVect clusters; clusters.reserve(NSpacePoints*2);
    DataVect phiModule; phiModule.reserve(NSpacePoints);
    DataVect etaModule; etaModule.reserve(NSpacePoints);
    DataVect numBarcodes; numBarcodes.reserve(NSpacePoints);
    DataVect barcodes; barcodes.reserve(NSpacePoints); // Usually less then one per space point

    /**
     * Now fill in all the data
     */

    //Loop over all SpacePoint - PRDTruth pairs
    for (const auto &SPTruthPair : SpacePointTruthPairList) {

      // Loop over SpacePoint Collections in the SpacePoint container
      for (const auto SpacePointColl : *(SPTruthPair.first)){

        //Loop over SpacePoints themselves
        for (const auto SpacePoint : *SpacePointColl) {
          
          //Get the position of the space point
	        Amg::Vector3D point = SpacePoint->globalPosition();
          
          //Store position in units of centimeters
          x.push_back(DataType(point.x() * CLHEP::mm/CLHEP::cm));
          y.push_back(DataType(point.y() * CLHEP::mm/CLHEP::cm));
          z.push_back(DataType(point.z() * CLHEP::mm/CLHEP::cm));
          
          //Get the cluster list for the Space point (first and second)
          const std::pair<const Trk::PrepRawData*, const Trk::PrepRawData*> clusterList = SpacePoint->clusterList();

          //Get the identifiers of the first and second cluster 
          Identifier idFirst = clusterList.first->identify();
          Identifier idSecond = (clusterList.second != NULL) ? clusterList.second->identify() : Identifier();

          //Get phi and eta of the module in detector coordinates of the first cluster
          if (clusterList.first->type(Trk::PrepRawDataType::SCT_Cluster)) {
                phiModule.push_back(DataType(m_geo->SCTIDHelper()->phi_module(idFirst)));
                etaModule.push_back(DataType(m_geo->SCTIDHelper()->eta_module(idFirst)));
          }
          else {
             phiModule.push_back(DataType(m_geo->PixelIDHelper()->phi_module(idFirst)));
             etaModule.push_back(DataType(m_geo->PixelIDHelper()->eta_module(idFirst)));
          }

          // Store the cluster(s) identifier (pair)
          clusters.push_back(DataType(idFirst.get_compact()));   
          clusters.push_back((idSecond.is_valid()) ? DataType(idSecond.get_compact()) : DataType(-1));

          //Stop here if there is no truth
          const PRD_MultiTruthCollection* PRDTruthColl = SPTruthPair.second;
          if ( PRDTruthColl == nullptr ) continue ;

          // Finally get barcodes of associated truth particles
          numBarcodes.push_back(SiSpacePointRetrieverHelpers::getTruthBarcodes(idFirst, idSecond, PRDTruthColl, barcodes));

          ATH_MSG_VERBOSE( "Found " << numBarcodes.back() << " common barcodes, now " 
                                                      << barcodes.size() << " in total" );

        } // loop over SpacePoint collection
      } // loop over SpacePoint container
    } //loop over SpacePoint - TruthMap collection pairs

    //Now put together the DataMap
    DataMap dataMap;
    const auto sz = x.size();
    dataMap["x"] = std::move(x);
    dataMap["y"] = std::move(y);
    dataMap["z"] = std::move(z);
    dataMap["clusters multiple=\"2\""] = std::move(clusters);
    dataMap["phiModule"] = std::move(phiModule);
    dataMap["etaModule"] = std::move(etaModule);

    //Only store truth associations if we retrieved them
    if ( numBarcodes.size() > 0 ){
      //Add barcodes counter
      dataMap["numBarcodes"] = numBarcodes;
      // Compute the "multiple" and put the barcodes vector in the map.
      std::string bctag = "barcodes multiple=\""+DataType(barcodes.size()/double(numBarcodes.size())).toString()+"\"";
      dataMap[bctag] = std::move(barcodes);
    }

    ATH_MSG_DEBUG( dataTypeName() << ": "<< sz );

     //forward data to formating tool and return
    return FormatTool->AddToEvent(dataTypeName(), "", &dataMap);
  }

  StatusCode SiSpacePointRetriever::initialize() {
    // Read Handle Key
    ATH_CHECK(m_PixelSPContainerName.initialize());
    ATH_CHECK(m_SCTSPContainerName.initialize());
    m_usePixelTruthMap = !m_PixelPRDTruthName.key().empty();
    ATH_CHECK(m_PixelPRDTruthName.initialize(m_usePixelTruthMap));
    m_useSCTTruthMap = !m_SCTPRDTruthName.key().empty();
    ATH_CHECK(m_SCTPRDTruthName.initialize(m_useSCTTruthMap));

    return m_geo.retrieve();
  }
  
} //namespace

