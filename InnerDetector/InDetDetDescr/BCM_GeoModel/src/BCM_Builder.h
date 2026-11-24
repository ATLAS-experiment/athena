/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BCMGEOMODEL_BCMBCMBUILDER_H
#define BCMGEOMODEL_BCMBCMBUILDER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GeoModelInterfaces/IGeoSubDetTool.h"
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
      using base_class::base_class;

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
      // module parameters: TRANS_X, TRANS_Y, TRANS_Z, ROT_X, ROT_Y, ROT_Z, DIMENSION_Z, DIAM_TRANS_Y, DIAM_TRANS_Z, DIAM_DIAM_X, DIAM_DIAM_Y, DIAM_DIAM_Z, LAMEL_OFFSET_H, LAMEL_OFFSET_I, LAMEL_OFFSET_J
      Gaudi::Property<std::vector<double>> m_module0{this, "Modul0", {95.744, 0, 1882.4, 0, 45, 0, 0.4, 0, -59.785, 0.2, 0, -0.4, 34.860, 29.509, 20.205}}; //F410
      Gaudi::Property<std::vector<double>> m_moduleI{this, "ModulI", {0, 95.744, 1882.4, 0, 45, 90, 0.4, 0, -59.785, 0.2, 0, -0.4, 34.860, 29.509, 20.205}}; //F405
      Gaudi::Property<std::vector<double>> m_moduleII{this, "ModulII", {-95.744, 0, 1882.4, 0, 45, 180, 0.4, 0, -59.785, 0.2, 0, -0.4, 34.860, 29.509, 20.205}}; //F413
      Gaudi::Property<std::vector<double>> m_moduleIII{this, "ModulIII", {0, -95.744, 1882.4, 0, 45, 270, 0.4, 0, -59.785, 0.2, 0, -0.4, 34.860, 29.509, 20.205}}; //F404
      Gaudi::Property<std::vector<double>> m_moduleIV{this, "ModulIV", {95.744, 0, -1882.4, 180, -45, 0, 1.4, 0, -59.785, 0.2, 0, -0.4, 34.860, 29.509, 20.205}}; //F424
      Gaudi::Property<std::vector<double>> m_moduleV{this, "ModulV", {0, 95.744, -1882.4, 180, -45, 90, 1.4, 0, -59.785, 0.2, 0, -0.4, 34.860, 29.509, 20.205}}; //F420
      Gaudi::Property<std::vector<double>> m_moduleVI{this, "ModulVI", {-95.744, 0, -1882.4, 180, -45, 180, 1.4, 0, -59.785, 0.2, 0, -0.4, 34.860, 29.509, 20.205}}; //F422
      Gaudi::Property<std::vector<double>> m_moduleVII{this, "ModulVII", {0, -95.744, -1882.4, 180, -45, 270, 0.4, 0, -59.785, 0.2, 0, -0.4, 34.860, 29.509, 20.205}}; //F408
      Gaudi::Property<unsigned int> m_moduleon{this, "ModulesOn", 255};
      Gaudi::Property<bool> m_bcmon{this, "BCMon", true};
      Gaudi::Property<bool> m_BDparameters{this, "DBparameters", true};
    };
} // end of namespace

#endif
