/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITKPIXEL_DECODINGSTREAMINFO_H
#define ITKPIXEL_DECODINGSTREAMINFO_H

#include <cstdint>

struct ITkPixelDecodingStreamInfo
{
	std::size_t m_dataOffset{0};
	uint16_t m_dataLength{0};
	uint32_t m_moduleId{0};
	uint8_t m_transformType{0};
	uint8_t m_chipId{0}; 
};

#endif // ITKPIXEL_DECODINGSTREAMINFO_H