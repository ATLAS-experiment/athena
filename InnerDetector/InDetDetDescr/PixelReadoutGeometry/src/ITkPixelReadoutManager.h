/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak


#ifndef ITKPIXELREADOUTMANAGER_H
#define ITKPIXELREADOUTMANAGER_H

#include <AthenaBaseComps/AthService.h>
#include <PixelReadoutGeometry/IPixelReadoutManager.h>
//#include <InDetReadoutGeometry/SiDetectorElement.h>

class PixelID;

namespace InDetDD
{
class PixelDetectorManager;

namespace ITk
{
class PixelReadoutManager final : public extends<AthService, IPixelReadoutManager>
{
public:
  PixelReadoutManager(const std::string &name,
                      ISvcLocator *svc);

  virtual StatusCode initialize() override final;

  virtual PixelModuleType getModuleType(Identifier id) const override final;
  // Avoid detector element search if already available
  PixelModuleType getModuleType(Identifier id, const SiDetectorElement* element) const;
  
  virtual PixelDiodeType getDiodeType(Identifier id) const override final;
  // Avoid detector element search if already available
  PixelDiodeType getDiodeType(Identifier id, const SiDetectorElement* element) const;
  
  virtual Identifier getPixelIdfromHash(IdentifierHash offlineIdHash,
                                        uint32_t FE,
                                        uint32_t row,
                                        uint32_t column) const override final;
  virtual Identifier getPixelId(Identifier offlineId,
                                uint32_t FE,
                                uint32_t row,
                                uint32_t column) const override final;

  virtual uint32_t getFE(Identifier diodeId,
                         Identifier offlineId) const override final;

  virtual uint32_t getFE(Identifier diodeId,
			 Identifier offlineId,
			 const SiDetectorElement* element) const override final;
  
  virtual uint32_t getColumn(Identifier diodeId,
                             Identifier offlineId) const override final;

  uint32_t getColumn(Identifier diodeId,
		     Identifier offlineId,
		     const SiDetectorElement* element) const;
  
  virtual uint32_t getRow(Identifier diodeId,
                          Identifier offlineId) const override final;

  uint32_t getRow(Identifier diodeId,
		  Identifier offlineId,
		  const SiDetectorElement* element) const;
		  
  
private:
  ServiceHandle<StoreGateSvc> m_detStore{this, "DetectorStore", "DetectorStore"};

  const PixelDetectorManager *m_detManager{};
  const PixelID *m_idHelper{};

  Gaudi::Property<std::string> m_detectorName
  {this, "DetectorName", "ITkPixel", "Pixel detector name"};
  Gaudi::Property<std::string> m_pixelIDName
  {this, "PixelIDName", "PixelID", "Pixel ID name"};
};

} // namespace ITk
} // namespace InDetDD

#endif
