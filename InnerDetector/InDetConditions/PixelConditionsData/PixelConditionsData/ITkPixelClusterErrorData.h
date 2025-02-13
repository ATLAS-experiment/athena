/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITKPIXELCLUSTERERRORDATA_H
#define ITKPIXELCLUSTERERRORDATA_H

#include "AthenaKernel/CLASS_DEF.h"

#include "Identifier/Identifier.h"
#include "InDetIdentifier/PixelID.h"

#include <string>
#include <vector>
#include <cassert>
#include <span>

namespace ITk
{

class PixelClusterErrorData
{

  public:
    enum EParam {
       kPeriod_phi,
       kPeriod_sinheta,
       kDelta_x_slope,
       kDelta_x_offset,
       kError_x,
       kDelta_y_slope,
       kDelta_y_offset,
       kError_y,
       kNParam
    };

    PixelClusterErrorData() { Initialize(); }
    ~PixelClusterErrorData() {};

    /** Methods to access the calibration data */

    std::pair<double,double> getDelta(IdentifierHash idHash,
                                      int sizePhi, double angle,
                                      int sizeZ, double eta) const;
    std::pair<double,double> getDeltaError(IdentifierHash idHash) const {
       const std::array<float,kNParam> &values = m_constmap.at(idHash);
       return std::make_pair(values[kError_x],values[kError_y]);
    }
    void setDeltaError(IdentifierHash idHash,
                       float period_phi, float period_sinheta,
                       float delta_x_slope, float delta_x_offset, float error_x,
                       float delta_y_slope, float delta_y_offset, float error_y );
    void setDeltaError(IdentifierHash idHash, const std::array<float, kNParam> &param ) {
       m_constmap.at(idHash)=param;
    }
    void setDeltaError(IdentifierHash idHash, std::span<double> param ) {
       std::array<float,kNParam> &dest=m_constmap.at(idHash);
       if (param.size() != dest.size()) throw std::range_error("Parameter sizes do not match");
       std::copy(param.begin(),param.end(), dest.begin());
    }
    void print(const std::string& file) const;
    /** load cluster error data from ascii file
     * @param file name of the ascii file
     * @return number of entries read from the file.
     */
    unsigned int load(const std::string& file);

    IdentifierHash getIdentifierHash(Identifier identifier) const {
       return m_pixelID->wafer_hash(identifier);
    }
    Identifier getIdentifier(IdentifierHash idHash) const {
       return m_pixelID->wafer_id(idHash);
    }

    const std::vector< std::array<float, kNParam> > &getConstMap() const { return m_constmap; }

  private:
     void Initialize();
     // map to store all ITk Analogue Clustering constants and errors
     std::vector< std::array<float, kNParam> > m_constmap;

     const PixelID* m_pixelID{nullptr};

};

} // namespace ITk

#endif

