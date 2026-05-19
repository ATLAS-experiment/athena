/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
   @class egammaLargeClusterCellRecoveryTool
   Recovers large 7x11 clusters for clusters in EMB & EMEC.

   @author Gabriel P. Matos <gpinheir@cern.ch>
   Following discussions with Christos Anastopoulos.
*/

#ifndef EGAMMA_LARGE_CLUSTER_CELL_RECOVERY_TOOL_H
#define EGAMMA_LARGE_CLUSTER_CELL_RECOVERY_TOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "egammaInterfaces/IegammaLargeClusterCellRecoveryTool.h"

#include "CaloUtils/CaloClusterProcessor.h"

class egammaLargeClusterCellRecoveryTool : public AthAlgTool, virtual public IegammaLargeClusterCellRecoveryTool {

    public:
    /** Constructor */
    egammaLargeClusterCellRecoveryTool(const std::string& type,
                                    const std::string& name,
                                    const IInterface* parent);

    /** Destructor */
    virtual ~egammaLargeClusterCellRecoveryTool() = default;

    /** Initialize method */
    virtual StatusCode initialize() override;

    /** Method to recover large clusters */
    virtual StatusCode execute(const xAOD::CaloCluster* cluster,
                               const CaloDetDescrManager* cmgr,
                               const CaloCellContainer* cell_container,
                               Info& info) const override final;


    private:
      // Copying from Reconstruction/egamma/egammaTools/egammaLargeClusterMaker.h
      
      // Size of window to look for hottest cell
      static constexpr int m_neta = 7;
      static constexpr int m_nphi = 7;

      // Et threshold for large cluster to exist
      static constexpr double m_centEtThr = 3000.; // in MeV

      /** @brief Tool to fill rectangular 7x11 cluster */
      ToolHandle<CaloClusterProcessor> m_caloFillRectangularTool{
          this,
          "CaloFillRectangularClusterTool",
          "",
          "Handle of the CaloFillRectangularClusterTool"
        };

};


#endif // EGAMMA_LARGE_CLUSTER_CELL_RECOVERY_TOOL_H
