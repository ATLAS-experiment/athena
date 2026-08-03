/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// LayerMaterialAnalyser.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRTESTTOOLS_LAYERMATERIALRECORDDIAGNOSER_H
#define TRKDETDESCRTESTTOOLS_LAYERMATERIALRECORDDIAGNOSER_H

// Trk
#include "TrkDetDescrInterfaces/ILayerMaterialAnalyser.h"
// Gaudi & Athena
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "CxxUtils/checker_macros.h"

#ifndef LAYERMAXBINS
#define LAYERMAXBINS 50000
#endif

class TTree;

namespace Trk {

    class LayerMaterialProperties;
    class LayerMaterialRecord;
    class Layer;

    /** @class LayerMaterialAnalyser

        LayerMaterialProperties creator for CompoundLayerMaterial

      @author Andreas.Salzburger@cern.ch
     */

    class ATLAS_NOT_THREAD_SAFE LayerMaterialAnalyser : //mutable
      public AthAlgTool, virtual public ILayerMaterialAnalyser {

      public:
        /** Constructor */
        LayerMaterialAnalyser(const std::string&,const std::string&,const IInterface*);

        /** AlgTool initialize method */
        StatusCode initialize();

        /** AlgTool finalize method */
        StatusCode finalize();

        /** process the layer  - after material creation and loading */
        StatusCode analyseLayerMaterial(const Layer& lay) const;

        /** process the layer material proerties  - after material creation and before loading */
        StatusCode analyseLayerMaterial(const Layer& lay, const LayerMaterialProperties& lmp) const;

        /** process the layer material record  - before material creation */
        StatusCode analyseLayerMaterial(const Layer& lay, const MaterialPropertiesMatrix& lmr) const;

        /** process the layer material record  - before material creation */
        StatusCode analyseLayerMaterial(const Layer& lay, const LayerMaterialRecord& lmr) const;


    private:
        StatusCode analyse(const Layer& lay,
                           const MaterialPropertiesMatrix& lmr,
                           const std::vector< std::vector< unsigned int > >* bCounter = 0) const;

        TTree* m_validationTree = nullptr;            //!< The validation tree
        Gaudi::Property<std::string> m_validationTreeName
          {this, "ValidationTreeName", "LayerMaterialAnalyser",
           "validation tree name - to be accessed by this from root"};
        Gaudi::Property<std::string> m_validationTreeDescription
          {this, "ValidationTreeDescription", "LayerMaterialAnalyser information",
           "validation tree description - second argument in TTree"};
        Gaudi::Property<std::string> m_validationTreeFolder
          {this, "ValidationTreeFolder", "/val/LayerMaterialAnalyser",
           "stream/folder to for the TTree to be written out"};

        mutable int                 m_layerIndex = 0;                //!< the layer index given by the TrackingGeometry
        mutable int                 m_layerType = 0;                 //!< the type of the layer 1 - cylinder, 2 - disk
        std::vector<float>* m_layerTranslation = nullptr;       //!< center of the transform
        std::vector<float>* m_layerRotation = nullptr;          //!< orientation of the layer
        mutable float               m_layerDimension0 = 0.;           //!< dimension 0 : cylinder r, disk r_min
        mutable float               m_layerDimension1 = 0.;           //!< dimension 1 : cylinder z, disk r_max
        mutable int                 m_layerBins = 0;                 //!< total number of bins - loc0 * loc 1
        mutable int                 m_layerBins0 = 0;                //!< total number of bins - loc 0
        mutable int                 m_layerBins1 = 0;                //!< total number of bins - loc 0
        std::vector<int>*   m_bin0 = nullptr;        //!< bin 0
        std::vector<int>*   m_bin1 = nullptr;        //!< bin 1
        std::vector<float>* m_thickness = nullptr;   //!< gathered thickness from material mapping/material properties
        std::vector<float>* m_X0 = nullptr;          //!< gathered X0 from material mapping/material properties
        std::vector<float>* m_L0 = nullptr;          //!< gathered L0 from material mapping/material properties
        std::vector<float>* m_A = nullptr;           //!< gathered A from material mapping/material properties
        std::vector<float>* m_Z = nullptr;           //!< gathered Z from material mapping/material properties
        std::vector<float>* m_Rho = nullptr;         //!< gathered rho from material mapping/material properties
        std::vector<int>*   m_elements = nullptr;    //!< gathered number of elements from material mapping/material properties
        std::vector<int>*   m_binCounter = nullptr;  //!< how often was this bin hit / used

    };

}

#endif




