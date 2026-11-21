/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BLMGEOMODEL_BLMBUILDER_H
#define BLMGEOMODEL_BLMBUILDER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GeoModelInterfaces/IGeoSubDetTool.h"
#include <vector>

class GeoVPhysVol;
class StoreGateSvc;

namespace InDetDD
{

  /** @class  BLMBuilder 
   *  @brief  Beam Loss Monitor Builder
   *  @author Bostjan Macek <bostjan.macek@cern.ch>
   */  

  class BLM_Builder : public extends<AthAlgTool, IGeoSubDetTool>
    {
    public:
      using base_class::base_class;

       /** default destructor */
      virtual ~BLM_Builder () = default;
      
       /** standard Athena-Algorithm method */
      virtual StatusCode initialize() override;
       /** standard Athena-Algorithm method */
      virtual StatusCode finalize() override;
       /** build the BCM geometry */
      virtual StatusCode build(GeoVPhysVol* parent) override;

    private:
      /** member variables for algorithm properties: */
      // module parameters: Z, R, PHI, ROT_X, ROT_Y, ROT_Z, DIAM_TRANS_X, DIAM_TRANS_Y, DIAM_TRANS_Z
      Gaudi::Property<std::vector<double>> m_module0{this, "Modul0", {3383.5, 62.825, 17, 180, 0, 287, 0, 0, 0}};
      Gaudi::Property<std::vector<double>> m_moduleI{this, "ModulI", {3383.5, 62.825, 45, 180, 0, 315, 0, 0, 0}};
      Gaudi::Property<std::vector<double>> m_moduleII{this, "ModulII", {3383.5, 62.825, 73, 180, 0, 343, 0, 0, 0}};
      Gaudi::Property<std::vector<double>> m_moduleIII{this, "ModulIII", {3383.5, 62.825, 197, 180, 0, 107, 0, 0, 0}};
      Gaudi::Property<std::vector<double>> m_moduleIV{this, "ModulIV", {3383.5, 62.825, 225, 180, 0, 135, 0, 0, 0}};
      Gaudi::Property<std::vector<double>> m_moduleV{this, "ModulV", {3383.5, 62.825, 253, 180, 0, 163, 0, 0, 0}};
      Gaudi::Property<std::vector<double>> m_moduleVI{this, "ModulVI", {-3383.5, 62.825, 17, 0, 0, 107, 0, 0, 0}};
      Gaudi::Property<std::vector<double>> m_moduleVII{this, "ModulVII", {-3383.5, 62.825, 45, 0, 0, 135, 0, 0, 0}};
      Gaudi::Property<std::vector<double>> m_moduleVIII{this, "ModulVIII", {-3383.5, 62.825, 73, 0, 0, 163, 0, 0, 0}};
      Gaudi::Property<std::vector<double>> m_moduleIX{this, "ModulIX", {-3383.5, 62.825, 197, 0, 0, 287, 0, 0, 0}};
      Gaudi::Property<std::vector<double>> m_moduleX{this, "ModulX", {-3383.5, 62.825, 225, 0, 0, 315, 0, 0, 0}};
      Gaudi::Property<std::vector<double>> m_moduleXI{this, "ModulXI", {-3383.5, 62.825, 253, 0, 0, 343, 0, 0, 0}};
      Gaudi::Property<unsigned int> m_moduleon{this, "ModulesOn", 0xFFFF};
      Gaudi::Property<bool> m_blmon{this, "BLMon", true};
      Gaudi::Property<bool> m_BDparameters{this, "DBparameters", true};
    }; 
} // end of namespace

#endif 
