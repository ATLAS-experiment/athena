/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BCMGEOMODEL_BCMBCMBUILDER_H
#define BCMGEOMODEL_BCMBCMBUILDER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GeoModelInterfaces/IGeoSubDetTool.h"
#include "AthenaKernel/IOVSvcDefs.h"
#include <vector>

class GeoVPhysVol;
class StoreGateSvc;

namespace InDetDD
{

  /** @class   BCMBuilder
   *  @brief   Beam Condition Monitor Builder
   *  @author  Bostjan Macek <bostjan.macek@cern.ch>
   */

  class BCM_Builder : public extends<AthAlgTool, IGeoSubDetTool>
    {
    public:
      BCM_Builder(const std::string&,const std::string&,const IInterface*);

       /** default destructor */
      virtual ~BCM_Builder () = default;

       /** standard Athena-Algorithm method */
      virtual StatusCode initialize() override;
       /** standard Athena-Algorithm method */
      virtual StatusCode finalize() override;
       /** build the BCM geometry */
      virtual StatusCode build(GeoVPhysVol* parent) override;

    private:

      /** member variables for algorithm properties: */
      std::vector<double> m_module0;
      std::vector<double> m_moduleI;
      std::vector<double> m_moduleII;
      std::vector<double> m_moduleIII;
      std::vector<double> m_moduleIV;
      std::vector<double> m_moduleV;
      std::vector<double> m_moduleVI;
      std::vector<double> m_moduleVII;
      unsigned int m_moduleon;
      bool m_bcmon;
      bool m_BDparameters;
    };
} // end of namespace

#endif
