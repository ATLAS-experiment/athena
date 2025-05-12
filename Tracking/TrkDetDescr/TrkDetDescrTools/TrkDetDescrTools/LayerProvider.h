/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKDETDESCRTOOLS_LAYERPROVIDER_H
#define TRKDETDESCRTOOLS_LAYERPROVIDER_H

// Trk
#include "TrkDetDescrTools/LayerProviderImpl.h"
#include "TrkDetDescrInterfaces/ILayerProvider.h"
#include "TrkDetDescrInterfaces/ILayerBuilder.h"
// Gaudi & Athena
#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "CxxUtils/checker_macros.h"

namespace Trk {

    class Layer;

    /** @class LayerProvider
      Wrapper around an ILayerBuilder to feed into the StagedGeometryBuilder
      @author Andreas.Salzburger@cern.ch
      aauthor Christos Anastopoulos (AthenaMT modifications),
     */
    class LayerProvider : public extends<LayerProviderImpl, ILayerProvider>
    {
      public:
        /** Constructor */
        LayerProvider(const std::string&,const std::string&,const IInterface*);

        /** Destructor */
        virtual ~LayerProvider() = default;

        /** initialize */
        virtual StatusCode initialize() override final;

        /** LayerBuilder interface method - returning the endcap layer */
        virtual std::pair<const std::vector<Layer*>, const std::vector<Layer*> > endcapLayer() const override final;

        /** LayerBuilder interface method - returning the central layers */
        virtual const std::vector<Layer*> centralLayers() const override final;

        /** Name identification */
        virtual const std::string& identification() const override final;

      private:
        PublicToolHandle<ILayerBuilder> m_layerBuilder{this, "LayerBuilder", ""};
    };


} // end of namespace

#endif // TRKDETDESCRTOOLS_LAYERPROVIDER_H
