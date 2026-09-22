# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

atlas_subdir ( L1MuonMDT )

atlas_add_component (L1MuonMDT
        src/*.cxx src/components/*.cxx

        LINK_LIBRARIES AthenaKernel xAODMuonRDO MuonRDO MuonDigitContainer xAODTrigger AthenaMonitoringKernelLib IRegionSelector 
  MuonReadoutGeometryR4 L1MuonMDTToolsLib TrigSteeringEvent xAODMuonViews xAODMuonPrepData xAODL0MuonCand MdtCalibSvcLib MuonCalibEventBase MuonCalibITools xAODMuonSimHit)




atlas_add_test( RunTest
        SCRIPT python -m L1MuonMDT.L0MuonMDTConfig --nEvents 2
                PRIVATE_WORKING_DIRECTORY
              	POST_EXEC_SCRIPT noerror.sh)


atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} --extend-extensions=ATL901 )

