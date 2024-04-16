/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// PixelRDOTool.h
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef SICLUSTERIZATIONTOOL_PIXELRDOTOOL_H
#define SICLUSTERIZATIONTOOL_PIXELRDOTOOL_H

//#include "GaudiKernel/AlgTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "SiClusterizationTool/IPixelClusteringTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "InDetConditionsSummaryService/IInDetConditionsTool.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "InDetReadoutGeometry/SiDetectorElementStatus.h"
#include "InDetIdentifier/PixelID.h"
#include "PixelReadoutGeometry/IPixelReadoutManager.h"

class IInDetConditionsTool;
template <class T> class ServiceHandle;

namespace InDet {

class ClusterMakerTool;

class UnpackedPixelRDO {
  public:
    
    UnpackedPixelRDO(int ncl, int row, int col, int tot, int lvl1, Identifier id):
      NCL(ncl), ROW(row), COL(col), TOT(tot), LVL1(lvl1), ID(id) {};
    
    int        NCL;
    int        ROW;
    int        COL;
    int        TOT;
    int       LVL1;
    Identifier ID ;
};


class PixelRDOTool : public AthAlgTool 
{

public:
     PixelRDOTool
           (const std::string& type,
	    const std::string& name,
	    const IInterface* parent);
     
     virtual ~PixelRDOTool() = default;
     virtual StatusCode initialize();

     // Produces a vector of validated unpacked pixel RDOs
     std::vector<UnpackedPixelRDO>
     getUnpackedPixelRDOs(const InDetRawDataCollection<PixelRDORawData> &collection,
			  const PixelID& pixelID,
			  const InDetDD::SiDetectorElement* element,
			  const EventContext& ctx,
			  int defaultLabel = -1) const;

     // Determines if a pixel cell (whose identifier is the first argument) is 
     // a ganged pixel. If this is the case, the last argument assumes the 
     // value of the identifier of the cell it is ganged with. 
     // The second argument is the pixel module the hit belongs to.
     static std::optional<Identifier>
     isGanged(const Identifier& rdoID,
	      const InDetDD::SiDetectorElement* element);

     // Determines if a pixel cell is in a good state, either using
     // the detector element status or the module-map-based summary tool
     bool isGoodRDO(const InDet::SiDetectorElementStatus *pixelDetElStatus,
		    const IdentifierHash& moduleHash,
		    const Identifier& rdoID,
        const EventContext& ctx) const;

    // Method to check if an RDO is duplicated.
    // If it is, update lvl1 value.
    bool checkDuplication(const PixelID& pixelID,
			  const Identifier& rdoID, 
			  const int lvl1, 
			  std::vector<UnpackedPixelRDO>& collectionID) const;
                       
    const InDet::SiDetectorElementStatus* getPixelDetElStatus(const EventContext& ctx) const;

    // Check if we have a valid RDO collection. If so, return pointer
    // to corresponding detector element.
    const InDetDD::SiDetectorElement*
    checkCollection(const InDetRawDataCollection<PixelRDORawData> &collection,
		    const EventContext& ctx) const;


private:
    
    ToolHandle<IInDetConditionsTool> m_summaryTool {
	this,
	"PixelConditionsSummaryTool",
	"PixelConditionsSummaryTool",
	"Tool to retrieve Pixel Conditions summary"
    };

    /** @brief Optional read handle to get status data to test whether
     * a pixel detector element is good.  If set to
     * e.g. PixelDetectorElementStatus the event data will be used
     * instead of the pixel conditions summary tool.
     */
    SG::ReadHandleKey<InDet::SiDetectorElementStatus> m_pixelDetElStatus {
        this,
	"PixelDetElStatus",
	"" ,
	"Key of SiDetectorElementStatus for Pixel"
    };

    ServiceHandle<InDetDD::IPixelReadoutManager> m_pixelReadout {
        this,
	"PixelReadoutManager",
	"PixelReadoutManager",
	"Pixel readout manager"
    };

    const PixelID* m_pixelId = nullptr;

    BooleanProperty m_checkDuplicatedRDO {
	this,
	"CheckDuplicatedRDO",
	false,
	"Check duplicated RDOs using isDuplicated method"
    };

    BooleanProperty m_printDuplicate {
        this,
	"PrintDuplicate",
	true,
	"Print duplicate RDO warning"
    };

    BooleanProperty m_checkGanged {
	this,
	"CheckGanged",
	true,
	"Check for ganged pixels when unpacking the RDOs"
    };


    BooleanProperty m_useModuleMap {
	this,
	"UsePixelModuleMap",
	true,
	"Use bad modules map"
    };
    
    SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_pixelDetEleCollKey {
	this,
	"PixelDetEleCollKey",
	"PixelDetectorElementCollection",
	"Key of SiDetectorElementCollection for Pixel"
    };


};

}
#endif // SICLUSTERIZATIONTOOL_PIXELRDOTOOL_H
