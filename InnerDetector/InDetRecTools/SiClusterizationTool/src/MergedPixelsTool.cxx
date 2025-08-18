/*
  copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// MergedPixelsTool.cxx
// TheSiSimpleClusteringAlg adapted to implement cluster merging
// input from RDOs.
//
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "SiClusterizationTool/MergedPixelsTool.h"

#include "Identifier/IdentifierHash.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "InDetPrepRawData/SiWidth.h"
#include "InDetPrepRawData/PixelCluster.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "ReadoutGeometryBase/SiLocalPosition.h"
#include "InDetIdentifier/PixelID.h"
#include "SiClusterizationTool/ClusterMakerTool.h"

#include "GeoPrimitives/GeoPrimitives.h"
#include "EventPrimitives/EventPrimitives.h"
#include <unordered_set>

using PixelCalib::PixelOfflineCalibData;
using CLHEP::micrometer;

namespace InDet {

  // Constructor with parameters:
  MergedPixelsTool::MergedPixelsTool(const std::string &type,
                                     const std::string &name,
                                     const IInterface *parent) :
    base_class(type, name, parent) {}


  StatusCode  MergedPixelsTool::initialize()
  {
    ATH_CHECK(m_clusterMaker.retrieve());
    ATH_CHECK(m_pixelRDOTool.retrieve());
    ATH_CHECK(m_chargeDataKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_clusterErrorKey.initialize(SG::AllowEmpty));

    return StatusCode::SUCCESS;
  }


  StatusCode MergedPixelsTool::finalize()
  {

    ATH_MSG_DEBUG("------------------- Clusterization Statistics ------------------------");
    ATH_MSG_DEBUG("-- # Processed Pixel Clusters     : " << m_processedClusters);
    ATH_MSG_DEBUG("----------------------------------------------------------------------");
    return StatusCode::SUCCESS;
  }

  //-----------------------------------------------------------------------
  // Once the lists of RDOs which makes up the clusters have been found by the
  // clusterize() method, this method is called for each of these lists.
  // The method computes the local position of the cluster, and create
  // a "cluster object" with all the required information (including charge
  // interpolation variables Omegax and Omegay, and whether the cluster
  // contains ganged pixels)
  // This method calls the ClusterMakerTool to compute global position and
  // position errors.
  // Input parameters are the list of RDOs identifier of the would-be
  // cluster, the list of TOT values, the module the cluster belongs to,
  // the pixel helper tool and the number of RDOS of the cluster.

  PixelCluster MergedPixelsTool::makeCluster (const std::vector<Identifier>& group,
                                              const std::vector<int>& totgroup,
                                              const std::vector<int>& lvl1group,
                                              const InDetDD::SiDetectorElement* element,
                                              const PixelID& pixelID,
                                              int& clusterNumber,
                                              bool split,
                                              double splitProb1,
                                              double splitProb2,
                                              const PixelChargeCalibCondData* calibData,
                                              const PixelOfflineCalibData* offlineCalibData,
                                              const EventContext& ctx) const
  {
      ATH_MSG_VERBOSE("makeCluster called, number " << clusterNumber);

      const Identifier elementID = element->identify();
      const InDetDD::PixelModuleDesign* design
          (dynamic_cast<const InDetDD::PixelModuleDesign*>(&element->design()));
      if (not design){
        throw std::runtime_error( "Wrong design at MergedPixelsTool");
      }
      int rowMin = int(2*(design->width()/design->phiPitch()))+1;
      int rowMax = 0;
      int colMin = int(2*(design->length()/design->etaPitch()))+1;
      int colMax = 0;
      int qRowMin = 0;  int qRowMax = 0;
      int qColMin = 0;  int qColMax = 0;
      int lvl1min = 15; // lvl1 of a cluster is the minimum of the lvl1 of the hits

      InDetDD::SiLocalPosition sumOfPositions(0,0,0);
      std::vector<Identifier> DVid;
      DVid.reserve( group.size() );
      std::vector<Identifier>::const_iterator rdosBegin = group.begin();
      std::vector<Identifier>::const_iterator rdosEnd = group.end();
      std::vector<int>::const_iterator tot = totgroup.begin();
      std::vector<int>::const_iterator lvl1= lvl1group.begin();

      // Flag to tag clusters with any ganged pixel
      bool hasGanged = false;
      for (; rdosBegin!= rdosEnd; ++rdosBegin) {
        // compute cluster lvl1

        if ( (*lvl1) < lvl1min ) lvl1min=(*lvl1);
        ++lvl1;
        // process identifier
        const Identifier rId =  *rdosBegin;
        const int row = pixelID.phi_index(rId);
        const int col = pixelID.eta_index(rId);
        // flag if cluster contains at least a ganged pixel

        hasGanged = hasGanged ||
	            InDet::PixelRDOTool::isGanged(rId, element).has_value();
        DVid.push_back(rId);
        InDetDD::SiLocalPosition siLocalPosition
        (design->positionFromColumnRow(col,row));
        sumOfPositions += siLocalPosition;

        // check overflow for IBL
        int realtot = *tot;

        if (row == rowMin) qRowMin += realtot;
        if (row < rowMin) {
          rowMin = row;
          qRowMin = realtot;
        }

        if (row == rowMax) qRowMax += realtot;
        if (row > rowMax) {
          rowMax = row;
          qRowMax = realtot;
        }

        if (col == colMin) qColMin += realtot;
        if (col < colMin) {
          colMin = col;
          qColMin = realtot;
        }

        if (col == colMax) qColMax += realtot;
        if (col > colMax) {
          colMax = col;
          qColMax = realtot;
        }
        ++tot;
      }

      const int numberOfPixels = group.size();
      InDetDD::SiLocalPosition centroid(sumOfPositions/numberOfPixels);
      const Identifier id = element->identifierOfPosition(centroid);

      const int colWidth = colMax-colMin+1;
      const int rowWidth = rowMax-rowMin+1;

      // Compute eta for charge interpolation correction (if required)
      // Two pixels may have tot=0 (very rarely, hopefully)
      float etaRow = -1;
      float etaCol = -1;
      if(qRowMin+qRowMax > 0) etaRow = qRowMax/float(qRowMin+qRowMax);
      if(qColMin+qColMax > 0) etaCol = qColMax/float(qColMin+qColMax);

      double etaWidth = design->widthFromColumnRange(colMin, colMax);
      double phiWidth = design->widthFromRowRange(rowMin, rowMax);
      SiWidth siWidth(Amg::Vector2D(rowWidth,colWidth), Amg::Vector2D(phiWidth,etaWidth) );

      // Charge interpolation. Very rough guess (one can do better with
      // candidate track information later) TL
      if(m_posStrategy == 1 && !hasGanged && etaRow>0 && etaCol > 0){
        // width of the region of charge sharing
        // For disks assume normal incidence: delta is small, due to diffusion
        // of drifting charges in silicon
        // For barrel, assume 10 deg. incidence in Rphi, in z compute from
        // pseudorapidity
        // this may be improved with better parameterization, but it is
        // probably better to use candidate track information later in
        // reconstruction. TL
        // Values are made dependent on the sensor thickness to accomodate
        // different sensors layout. AA
        float deltax = 0;
        float deltay = 0;
        const float sensorThickness = element->thickness();
        Amg::Vector3D globalPos = element->globalPosition(centroid);
        InDetDD::SiLocalPosition totCorrection(0,0,0);
        if(pixelID.is_barrel(elementID)) {
          deltax = 30*micrometer*(sensorThickness/(250*micrometer));
          deltay = sensorThickness*std::abs(globalPos.z())/globalPos.perp();
          if(deltay > (design->etaPitch()) ) deltay = design->etaPitch();
        } else {
          deltax = 10*micrometer*std::sqrt(sensorThickness/(250*micrometer));
          deltay = 10*micrometer*std::sqrt(sensorThickness/(250*micrometer));
        }
        InDetDD::SiLocalPosition pos1 = design->positionFromColumnRow(colMin,rowMin);
        InDetDD::SiLocalPosition pos2 = design->positionFromColumnRow(colMax,rowMin);
        InDetDD::SiLocalPosition pos3 = design->positionFromColumnRow(colMin,rowMax);
        InDetDD::SiLocalPosition pos4 = design->positionFromColumnRow(colMax,rowMax);
        centroid = 0.25*(pos1+pos2+pos3+pos4)+
        InDetDD::SiLocalPosition(deltay*(etaCol-0.5),deltax*(etaRow-0.5),0.);
        ATH_MSG_VERBOSE("Barrel cluster with global position r= " << globalPos.perp() << " and z = " << globalPos.z());
        ATH_MSG_VERBOSE("deltax = " << deltax << " deltay = " << deltay);

      }
      if(m_posStrategy == 10 && !hasGanged  && etaRow>0 && etaCol > 0){
        // recFlag == 10 (CTB simulation)
        // use parametrization studied on CTB data by I. Reisinger (Dortmund)
        if (m_printw) {
          ATH_MSG_ERROR("Detected position strategy = 10, this is an obsolete setting for CTB analysis and is not supported anymore since Athena 15.4.0");
          ATH_MSG_ERROR("...reverting to default setting: position strategy = 0");
          m_printw=false;
        }
      }
      // Endcap SR1 Cosmics
      if(m_posStrategy == 20 && !hasGanged  && etaRow>0 && etaCol > 0){
        ATH_MSG_VERBOSE("Endcap cosmics simulation");
        const double deltax = 35*micrometer;
        const double deltay = 35*micrometer;
        InDetDD::SiLocalPosition pos1 =
            design->positionFromColumnRow(colMin,rowMin);
        InDetDD::SiLocalPosition pos2 =
            design->positionFromColumnRow(colMax,rowMin);
        InDetDD::SiLocalPosition pos3 =
            design->positionFromColumnRow(colMin,rowMax);
        InDetDD::SiLocalPosition pos4 =
            design->positionFromColumnRow(colMax,rowMax);
        centroid = 0.25*(pos1+pos2+pos3+pos4)+
            InDetDD::SiLocalPosition(deltay*(etaCol-0.5),
            deltax*(etaRow-0.5),0.);
      }


      Amg::Vector2D position(centroid.xPhi(),centroid.xEta());

      ATH_MSG_VERBOSE("Cluster ID =" << id);
      ATH_MSG_VERBOSE("Cluster width (eta x phi) = " << colWidth
          << " x " << rowWidth);
      ATH_MSG_VERBOSE("Cluster width (eta x phi) = " << etaWidth
          << " x " << phiWidth);
      ATH_MSG_VERBOSE("Cluster local position (eta,phi) = "
          << (position)[0] << " "
          << (position)[1]);

      if (!m_clusterMaker) {
        return PixelCluster(id, position, std::move(DVid), lvl1min,
                            std::vector<int>(totgroup), siWidth, element,
                            Amg::MatrixX{});
      } else {
        return m_clusterMaker->pixelCluster(
            id, position, std::move(DVid), lvl1min, std::vector<int>(totgroup),
            siWidth, element, hasGanged, m_errorStrategy, pixelID, split,
            splitProb1, splitProb2, calibData, offlineCalibData,ctx);
      }
  }

  //-----------------------------------------------------------------------
  // Runs for every pixel module (with non-empty RDO collection...).
  // It clusters together the RDOs with a pixell cell side in common
  // using connected component analysis based on four-cell
  // or eight-cell (if m_addCorners == true) connectivity
  PixelClusterCollection* MergedPixelsTool::clusterize(
      const InDetRawDataCollection<PixelRDORawData>& collection,
      const PixelID& pixelID,
      DataPool<PixelCluster>* dataItemsPool,
      const EventContext& ctx) const {

    const InDetDD::SiDetectorElement* element = m_pixelRDOTool->checkCollection(collection, ctx);
    if (element == nullptr){
      return nullptr;
    }

    std::vector<UnpackedPixelRDO> collectionID =
      m_pixelRDOTool->getUnpackedPixelRDOs(collection, pixelID, element, ctx);

    // Sort pixels in ascending columns order
    //
    if(collectionID.empty()) return nullptr;
    if(collectionID.size() > 1) std::sort(collectionID.begin(),collectionID.end(),pixel_less);

    // initialize the networks
    //
    std::vector<network> connections(collectionID.size());

    // Network production
    //
    int collectionSize = collectionID.size();
    // the maximum number of elements to save can be either 2 or 3
    // m_addCorners == true requires saving the bottom (or top), the side and the corner connection for each pixel,
    // otherwise you only save bottom (or top) and the side
    int maxElements = m_addCorners ? 3 : 2;

    // start looping on the pixels
    // for each pixel you build the network connection accordingly to the 4- or 8-cells connections
    for(int currentPixel = 0; currentPixel!=collectionSize-1; ++currentPixel) {
      int NB  = 0;
      int row = collectionID.at(currentPixel).ROW;
      int col = collectionID.at(currentPixel).COL;
      //
      auto & currentConnection = connections.at(currentPixel);
      for(int otherPixel = currentPixel+1; otherPixel!=collectionSize; ++otherPixel) {
        auto & otherConnection = connections.at(otherPixel);
        int deltaCol = std::abs(collectionID.at(otherPixel).COL - col);
        int deltaRow = std::abs(collectionID.at(otherPixel).ROW - row);
        // break if you are too far way in columns, as these ones will be taken in the next iterations
        if( deltaCol > 1) {
          break;
        }
        // if you need the corners, you jump the next rows, as these ones will be taken in the next iterations
        if ( m_addCorners and deltaRow > 1 ) {
          continue;
        }

        // Two default cases are considered:
        // 1) top/bottom connection (deltaCol=1 and deltaRow=0)
        // 2) side connection (deltaCol=0 and deltaRow=1)
        // In both cases the satisfied condition is:
        // deltaRow+deltaCol = 1
        //
        // As an optional case (true by defaul) we save also add a corner connection:
        // 3) corner connection (deltaCol=1 and deltaRow=1)

        // this builds the single pixel connection and breaks if the max number of elements is reached:
        if( (deltaCol+deltaRow) == 1 or (m_addCorners and deltaCol == 1 and deltaRow == 1) ) {
          int NC1 = currentConnection.NC;
          int NC2 = otherConnection.NC;
          int maxPossible = currentConnection.CON.size() - 1; //both are the same
          if ((NC1>maxPossible) or (NC2>maxPossible)){
            std::string m="attempt to access connection array of dimension 8 at idx "+std::to_string(currentConnection.NC);
            ATH_MSG_WARNING(m);
            break;
          } else {
            currentConnection.CON.at(currentConnection.NC++) = otherPixel;
            otherConnection.CON.at(otherConnection.NC++) = currentPixel ;
          }
          if(++NB==maxElements) {
            break;
          }
        }
      }
    }

    // Pixels clusterization
    //
    // Once the connections are built, the pixel clusterisation can start grouping together pixels
    int Ncluster = 0;
    for(int currentPixel=0; currentPixel!=collectionSize; ++currentPixel) {
      if(collectionID.at(currentPixel).NCL < 0) {
        collectionID.at(currentPixel).NCL = Ncluster;
        addClusterNumber(currentPixel,Ncluster,connections,collectionID);
        ++Ncluster;
      }
    }

    // Clusters sort in Ncluster order
    //
    if(--collectionSize > 1) {
      for(int i(1); i<collectionSize; ++i ) {
        UnpackedPixelRDO U  = collectionID.at(i+1);

        int j(i);
        while(collectionID.at(j).NCL > U.NCL) {
          collectionID.at(j+1)=collectionID.at(j);
          --j;
        }
        collectionID.at(j+1)=U;
      }
    }

    // Make a new pixel cluster collection
    //
    const Identifier elementID = collection.identify();
    const IdentifierHash idHash = collection.identifyHash();
    PixelClusterCollection  *clusterCollection = new PixelClusterCollection(idHash);
    if(dataItemsPool){
      clusterCollection->clear(SG::VIEW_ELEMENTS);
    }
    clusterCollection->setIdentifier(elementID);
    clusterCollection->reserve(Ncluster);

    std::vector<Identifier> DVid = {collectionID.at(0).ID };
    std::vector<int>        Totg = {collectionID.at(0).TOT};
    std::vector<int>        Lvl1 = {collectionID.at(0).LVL1};

    int clusterNumber = 0;
    int NCL0          = 0;

    DVid.reserve(collectionID.back().NCL);
    Totg.reserve(collectionID.back().NCL);
    Lvl1.reserve(collectionID.back().NCL);

    ++collectionSize;

    //retrieve conddata
    const PixelChargeCalibCondData *calibData = nullptr;
    if (!m_chargeDataKey.empty()) {
      SG::ReadCondHandle<PixelChargeCalibCondData> calibDataHandle(m_chargeDataKey, ctx);
      calibData = *calibDataHandle;
    }

    const PixelCalib::PixelOfflineCalibData *offlineCalibData = nullptr;
    if (!m_clusterErrorKey.empty()) {
      SG::ReadCondHandle<PixelCalib::PixelOfflineCalibData> offlineCalibDataHandle(m_clusterErrorKey, ctx);
      offlineCalibData = *offlineCalibDataHandle;
    }

    for(int i=1; i<=collectionSize; ++i) {
      if(i!=collectionSize and collectionID.at(i).NCL==NCL0) {
        DVid.push_back(collectionID.at(i).ID );
        Totg.push_back(collectionID.at(i).TOT);
        Lvl1.push_back(collectionID.at(i).LVL1);

      } else {
        // Cluster production
        // no merging has been done;
        ++m_processedClusters;
        PixelCluster* cluster = nullptr;
        if (dataItemsPool) {
          // data Item pool owns the element. The collection
          // is view. Just move assign to it.
          cluster = dataItemsPool->nextElementPtr();
          (*cluster) =
              makeCluster(DVid, Totg, Lvl1, element, pixelID, ++clusterNumber,
                          false, 0.0, 0.0, calibData, offlineCalibData,ctx);
        } else {
          // collection will own the element release
          cluster = new PixelCluster();
          (*cluster) =
              makeCluster(DVid, Totg, Lvl1, element, pixelID, ++clusterNumber,
                          false, 0.0, 0.0, calibData, offlineCalibData,ctx);
        }
        // statistics output
        cluster->setHashAndIndex(clusterCollection->identifyHash(),
                                 clusterCollection->size());
        clusterCollection->push_back(cluster);

        // Preparation for next cluster
        if (i!=collectionSize) {
          NCL0   = collectionID.at(i).NCL ;
          DVid.clear(); DVid = {collectionID.at(i).ID };
          Totg.clear(); Totg = {collectionID.at(i).TOT};
          Lvl1.clear(); Lvl1 = {collectionID.at(i).LVL1};
        }
      }
    }

    return clusterCollection;
  }

  void MergedPixelsTool::addClusterNumber(const int& r,
                                          const int& Ncluster,
                                          const std::vector<network>& connections,
                                          std::vector<UnpackedPixelRDO>& collectionID) const {
    for(int i=0; i!=connections.at(r).NC; ++i) {
      const int k = connections.at(r).CON.at(i);
      if(collectionID.at(k).NCL < 0) {
        collectionID.at(k).NCL = Ncluster;
        addClusterNumber(k, Ncluster, connections, collectionID);
      }
    }
  }
}
