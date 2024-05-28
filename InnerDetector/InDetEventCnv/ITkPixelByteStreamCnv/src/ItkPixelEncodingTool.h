/*
* Author: Ondra Kovanda, ondrej.kovanda at cern.ch
* Date: 05/2024
* Description: Athena tool wrapper around the ITkPix encoder
*/

#ifndef ITKPIXELBYTESTREAMCNV_ITKPIXELENCODINGTOOL_H
#define ITKPIXELBYTESTREAMCNV_ITKPIXELENCODINGTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ItkpixLayout.h"
#include "Itkpixv2Encoder.h"

class ItkPixelEncodingTool: public AthAlgTool {
    public:

        typedef ItkpixLayout<uint16_t> HitMap;
        
        ItkPixelEncodingTool(const std::string& type,const std::string& name,const IInterface* parent);

        StatusCode initialize();

        std::vector<uint32_t> encodeFE(HitMap hitMap);
        //std::vector<uint32_t> encodeFE(HitMap hitMap, uint8_t FE_id);

    private:

        std::unique_ptr<Itkpixv2Encoder> m_encoder;
};


#endif