/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONGEOMODELR4_TOROIDDETECTORMANGER_H
#define MUONGEOMODELR4_TOROIDDETECTORMANGER_H
#include "GeoModelKernel/GeoVDetectorManager.h"
#include "AthenaKernel/CLASS_DEF.h"
namespace MuonGMR4{
    /** @brief Simple detector manager that just holds the tree tops
     *         to the components representing the passive muon structure */
    class ToroidDetectorManager : public GeoVDetectorManager {
        public:
            /** @brief Constructor taking the detector manger name */
            ToroidDetectorManager(const std::string& name);
            /** @brief Default desctructor */
            virtual ~ToroidDetectorManager() = default;
            /** @brief Number of tree tops */
            unsigned int getNumTreeTops () const override final;
            /** @brief Access to the i-th tree top */
            PVConstLink getTreeTop (unsigned int i) const override final;
            /** @brief Add a GeoVolume to the toroid */
            void addTreeTop(PVConstLink pvLink);
        private:
            std::vector<PVConstLink> m_treeTops{};
    };
}

CLASS_DEF( MuonGMR4::ToroidDetectorManager , 30463770 , 1 );
#endif