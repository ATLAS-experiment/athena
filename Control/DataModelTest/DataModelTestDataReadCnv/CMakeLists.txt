# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( DataModelTestDataReadCnv )

# Component(s) in the package:
atlas_add_poolcnv_library( DataModelTestDataReadCnvPoolCnv
                           src/*.cxx
FILES
   DataModelTestDataRead/BVec.h DataModelTestDataRead/BDer.h
   DataModelTestDataRead/DVec.h DataModelTestDataRead/DDer.h
   DataModelTestDataRead/ELVec.h
   DataModelTestDataRead/G.h DataModelTestDataRead/GVec.h DataModelTestDataRead/GAuxContainer.h
   DataModelTestDataRead/H.h DataModelTestDataRead/HVec.h DataModelTestDataRead/HAuxContainer.h DataModelTestDataRead/HView.h
   DataModelTestDataCommon/C.h DataModelTestDataCommon/CVec.h DataModelTestDataCommon/CAuxContainer.h DataModelTestDataCommon/CView.h DataModelTestDataCommon/CVecWithData.h DataModelTestDataCommon/CInfoAuxContainer.h DataModelTestDataCommon/CTrigAuxContainer.h
   DataModelTestDataCommon/P.h DataModelTestDataCommon/PVec.h DataModelTestDataCommon/PAuxContainer.h
   DataModelTestDataCommon/S1.h DataModelTestDataCommon/S2.h
   DataModelTestDataRead/AllocTestContainer.h DataModelTestDataRead/AllocTestAuxContainer.h
   DataModelTestDataCommon/JVec.h DataModelTestDataCommon/JVecContainer.h DataModelTestDataCommon/JVecAuxContainer.h DataModelTestDataCommon/JVecAuxInfo.h
   DataModelTestDataCommon/PLinks.h DataModelTestDataCommon/PLinksContainer.h DataModelTestDataCommon/PLinksAuxContainer.h DataModelTestDataCommon/PLinksAuxInfo.h
TYPES_WITH_NAMESPACE
   DMTest::BVec DMTest::BDer
   DMTest::DVec DMTest::DDer
   DMTest::ELVec
   DMTest::C DMTest::CVec DMTest::CAuxContainer DMTest::CVecWithData DMTest::CInfoAuxContainer DMTest::CTrigAuxContainer DMTest::CView
   DMTest::P DMTest::PVec DMTest::PAuxContainer
   DMTest::G DMTest::GVec DMTest::GAuxContainer
   DMTest::H DMTest::HVec DMTest::HAuxContainer DMTest::HView
   DMTest::S1 DMTest::S2
   DMTest::CLinks DMTest::CLinksContainer DMTest::CLinksAuxInfo DMTest::CLinksAuxContainer DMTest::CLinksAOD
   DMTest::AllocTestContainer DMTest::AllocTestAuxContainer
   DMTest::JVec DMTest::JVecContainer DMTest::JVecAuxContainer DMTest::JVecAuxInfo
   DMTest::PLinks DMTest::PLinksContainer DMTest::PLinksAuxContainer DMTest::PLinksAuxInfo
LINK_LIBRARIES
   AthenaPoolUtilities AthenaKernel DataModelTestDataCommonLib DataModelTestDataReadLib AthenaPoolCnvSvcLib )

