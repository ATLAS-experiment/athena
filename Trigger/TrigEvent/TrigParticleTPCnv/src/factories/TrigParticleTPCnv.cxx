/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// generate the T/P converter entries
#include "AthenaKernel/TPCnvFactory.h"

/* TrigElectron T/P classes: persistent version 1*/
#include "TrigParticleTPCnv/TrigElectronContainer_tlp1.h"
#include "TrigParticleTPCnv/TrigElectronContainerCnv_tlp1.h"
#include "TrigParticleTPCnv/TrigElectronContainer_p1.h"
#include "TrigParticleTPCnv/TrigElectron_p1.h"
/* TrigElectron T/P classes: persistent version 2*/
#include "TrigParticleTPCnv/TrigElectronContainer_tlp2.h"
#include "TrigParticleTPCnv/TrigElectronContainerCnv_tlp2.h"
#include "TrigParticleTPCnv/TrigElectronContainer_p2.h"
#include "TrigParticleTPCnv/TrigElectron_p2.h"
/* TrigElectron T/P classes: persistent version 3*/
#include "TrigParticleTPCnv/TrigElectronContainerCnv_p3.h"
#include "TrigParticleTPCnv/TrigElectronContainer_p3.h"

//TrigPhoton p1
#include "TrigParticleTPCnv/TrigPhotonContainer_tlp1.h"
#include "TrigParticleTPCnv/TrigPhotonContainerCnv_tlp1.h"
#include "TrigParticleTPCnv/TrigPhotonContainer_p1.h"
#include "TrigParticleTPCnv/TrigPhoton_p1.h"

//TrigPhoton p2
#include "TrigParticleTPCnv/TrigPhotonContainer_tlp2.h"
#include "TrigParticleTPCnv/TrigPhotonContainerCnv_tlp2.h"
#include "TrigParticleTPCnv/TrigPhotonContainer_p2.h"
#include "TrigParticleTPCnv/TrigPhoton_p2.h"

#include "TrigParticleTPCnv/TrigPhotonContainerCnv_p3.h"
#include "TrigParticleTPCnv/TrigPhotonContainer_p3.h"

DECLARE_TPCNV_FACTORY(TrigElectronContainerCnv_tlp1,
                      TrigElectronContainer,
                      TrigElectronContainer_tlp1,
                      Athena::TPCnvVers::Old)
                      
DECLARE_TPCNV_FACTORY(TrigElectronContainerCnv_tlp2,
                      TrigElectronContainer,
                      TrigElectronContainer_tlp2,
                      Athena::TPCnvVers::Old)

DECLARE_TPCNV_FACTORY(TrigElectronContainerCnv_p3,
                      TrigElectronContainer,
                      TrigElectronContainer_p3,
                      Athena::TPCnvVers::Current)
                      
DECLARE_TPCNV_FACTORY(TrigPhotonContainerCnv_tlp1,
                      TrigPhotonContainer,
                      TrigPhotonContainer_tlp1,
                      Athena::TPCnvVers::Old)
                      
DECLARE_TPCNV_FACTORY(TrigPhotonContainerCnv_tlp2,
                      TrigPhotonContainer,
                      TrigPhotonContainer_tlp2,
                      Athena::TPCnvVers::Old)

DECLARE_TPCNV_FACTORY(TrigPhotonContainerCnv_p3,
                      TrigPhotonContainer,
                      TrigPhotonContainer_p3,
                      Athena::TPCnvVers::Current)

