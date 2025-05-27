/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TRT_ReadoutGeometry/TRT_DetElementContainer.h"
#include "TRT_ReadoutGeometry/TRT_BarrelElement.h"
#include "TRT_ReadoutGeometry/TRT_EndcapElement.h"

namespace InDetDD{

  TRT_DetElementContainer::TRT_DetElementContainer()
    : AthMessaging("TRT_DetElementContainer")
    , m_trtcoll()
    , m_trtnum(nullptr)
  {
    clear();
  }

  TRT_DetElementContainer::~TRT_DetElementContainer()
  {
    clear();
  }

  void TRT_DetElementContainer::setNumerology(const TRT_Numerology* mynum)
  {
    m_trtnum=mynum;
  }

  const TRT_DetElementCollection* TRT_DetElementContainer::getElements() const
  {
    return &m_trtcoll;
  }

  TRT_DetElementCollection* TRT_DetElementContainer::getElements()
  {
    return &m_trtcoll;
  }

  const TRT_Numerology* TRT_DetElementContainer::getTRTNumerology() const
  {
    return m_trtnum;
  }

  const TRT_BarrelElement* TRT_DetElementContainer::getBarrelDetElement(unsigned int positive
									, unsigned int moduleIndex
									, unsigned int phiIndex
									, unsigned int strawLayerIndex) const
  {
    if ( positive >= 2 || moduleIndex >= NMODMAX
	 || phiIndex>=NPHIMAX || strawLayerIndex >= NSTRAWLAYMAXBR) return nullptr;

    return m_baArray[positive][moduleIndex][phiIndex][strawLayerIndex];
  }

  TRT_BarrelElement* TRT_DetElementContainer::getBarrelDetElement(unsigned int positive
								  , unsigned int moduleIndex
								  , unsigned int phiIndex
								  , unsigned int strawLayerIndex)
  {
    if ( positive >= 2 || moduleIndex >= NMODMAX
	 || phiIndex>=NPHIMAX || strawLayerIndex >= NSTRAWLAYMAXBR) return nullptr;

    return m_baArray[positive][moduleIndex][phiIndex][strawLayerIndex];
  }


  const TRT_EndcapElement* TRT_DetElementContainer::getEndcapDetElement(unsigned int positive
									, unsigned int wheelIndex
									, unsigned int strawLayerIndex
									, unsigned int phiIndex) const
  {
    if ( positive >= 2 || wheelIndex >= NWHEELMAX
	 || phiIndex>=NPHIMAX || strawLayerIndex >= NSTRAWLAYMAXEC) return nullptr;

    return m_ecArray[positive][wheelIndex][strawLayerIndex][phiIndex];
  }

  TRT_EndcapElement* TRT_DetElementContainer::getEndcapDetElement(unsigned int positive
								  , unsigned int wheelIndex
								  , unsigned int strawLayerIndex
								  , unsigned int phiIndex)
  {
    if ( positive >= 2 || wheelIndex >= NWHEELMAX
	 || phiIndex>=NPHIMAX || strawLayerIndex >= NSTRAWLAYMAXEC) return nullptr;

    return m_ecArray[positive][wheelIndex][strawLayerIndex][phiIndex];
  }

  void TRT_DetElementContainer::addBarrelElement(TRT_BarrelElement *barrel)
  {
    // check if the element has already been added
    if (std::find(m_trtcoll.begin(), m_trtcoll.end(), barrel) != m_trtcoll.end()) return;
    // check if something was stored at the given indices
    TRT_BarrelElement* arrayElement = m_baArray
      [barrel->getCode().isPosZ()]
      [barrel->getCode().getModuleIndex()]
      [barrel->getCode().getPhiIndex()]
      [barrel->getCode().getStrawLayerIndex()];
    if (arrayElement != nullptr) {
      m_trtcoll.erase(std::remove(m_trtcoll.begin(), m_trtcoll.end(), arrayElement));
      delete arrayElement;
    }
    m_baArray[barrel->getCode().isPosZ()]
      [barrel->getCode().getModuleIndex()]
      [barrel->getCode().getPhiIndex()]
      [barrel->getCode().getStrawLayerIndex()] = barrel;
    m_trtcoll.push_back(barrel);
  }

  void TRT_DetElementContainer::addEndcapElement(TRT_EndcapElement *endcap)
  {
    // check if the element has already been added
    if (std::find(m_trtcoll.begin(), m_trtcoll.end(), endcap) != m_trtcoll.end()) return;
    // check if something was stored at the given indices
    TRT_EndcapElement* arrayElement = m_ecArray
      [endcap->getCode().isPosZ()]
      [endcap->getCode().getWheelIndex()]
      [endcap->getCode().getStrawLayerIndex()]
      [endcap->getCode().getPhiIndex()];
    if (arrayElement != nullptr) {
      m_trtcoll.erase(std::remove(m_trtcoll.begin(), m_trtcoll.end(), arrayElement));
      delete arrayElement;
    }
    m_ecArray[endcap->getCode().isPosZ()]
      [endcap->getCode().getWheelIndex()]
      [endcap->getCode().getStrawLayerIndex()]
      [endcap->getCode().getPhiIndex()] = endcap;
    m_trtcoll.push_back(endcap);
  }

  void TRT_DetElementContainer::manageBarrelElement(TRT_BarrelElement *barrel, const TRT_ID* idHelper)
  {
    if (m_baArray
       [barrel->getCode().isPosZ()]
       [barrel->getCode().getModuleIndex()]
       [barrel->getCode().getPhiIndex()]
       [barrel->getCode().getStrawLayerIndex()] ) {

      //Element already added - complain!
      ATH_MSG_DEBUG("manageBarrelElement: Overriding existing element");
    }

    m_baArray
      [barrel->getCode().isPosZ()]
      [barrel->getCode().getModuleIndex()]
      [barrel->getCode().getPhiIndex()]
      [barrel->getCode().getStrawLayerIndex()]
      =barrel;

    // Add the barrel element to the hash vector:
    if (idHelper) {
      Identifier id = idHelper->layer_id(barrel->getCode().isPosZ() ? +1 : -1,
					 barrel->getCode().getPhiIndex(),
					 barrel->getCode().getModuleIndex(),
					 barrel->getCode().getStrawLayerIndex());
      IdentifierHash hashId = idHelper->straw_layer_hash(id);
      if (hashId.is_valid()) {
	if (m_trtcoll.size() <= hashId) {
	  m_trtcoll.resize(static_cast<unsigned int>(hashId) + 1);
	}
	if (m_trtcoll[hashId]) {
	  //Element already added - complain!
	  ATH_MSG_DEBUG("manageBarrelElement: Overriding existing element for hashID");
	}
	m_trtcoll[hashId]=barrel;
      }
      else {
	ATH_MSG_WARNING("manageBarrelElement: Invalid identifier");
      }
    }
  }

  void TRT_DetElementContainer::manageEndcapElement(TRT_EndcapElement *endcap, const TRT_ID* idHelper)
  {
    if (m_ecArray
	[endcap->getCode().isPosZ()]
	[endcap->getCode().getWheelIndex()]
	[endcap->getCode().getStrawLayerIndex()]
        [endcap->getCode().getPhiIndex()] ) {
      //Element already added - complain!
      ATH_MSG_WARNING("manageEndcapElement: Overriding existing element");
    }

    m_ecArray
      [endcap->getCode().isPosZ()]
      [endcap->getCode().getWheelIndex()]
      [endcap->getCode().getStrawLayerIndex()]
      [endcap->getCode().getPhiIndex()]
      =endcap;

    if (idHelper) {
      Identifier id = idHelper->layer_id(endcap->getCode().isPosZ() ? +2 : -2,
					 endcap->getCode().getPhiIndex(),
					 endcap->getCode().getWheelIndex(),
					 endcap->getCode().getStrawLayerIndex());

      IdentifierHash hashId = idHelper->straw_layer_hash(id);
      if (hashId.is_valid()) {
	if (m_trtcoll.size() <= hashId) {
	  m_trtcoll.resize(static_cast<unsigned int>(hashId) + 1);
	}
	if (m_trtcoll[hashId]) {
	  //Element already added - complain!
	  ATH_MSG_DEBUG("manageEndcapElement: Overriding existing element for hashID");
	}
	m_trtcoll[hashId]=endcap;
      } else {
	ATH_MSG_WARNING("manageEndcapElement: Invalid identifier");
      }
    }
  }

  void TRT_DetElementContainer::clear()
  {
    m_trtcoll.clear();
    for (auto & ec : m_baArray) {
      for (auto & mod : ec) {
        for (auto & phi : mod) {
          for (auto & sLay : phi) {
            sLay = nullptr;
          }
        }
      }
    }
    for (auto & ec : m_ecArray) {
      for (auto & whe : ec) {
        for (auto & sLay : whe) {
          for(auto & phi : sLay) {
            phi = nullptr;
          }
        }
      }
    }
  }

}
