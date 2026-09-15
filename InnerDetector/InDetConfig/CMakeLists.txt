################################################################################
# Package: InDetConfig
################################################################################

# Declare the package name:
atlas_subdir( InDetConfig )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
#atlas_install_scripts( test/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
#atlas_install_joboptions( share/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

if (NOT SIMULATIONBASE)
    atlas_add_test( BackTrackingConfig_test
        SCRIPT python -m InDetConfig.BackTrackingConfig --norun
        POST_EXEC_SCRIPT noerror.sh)

    atlas_add_test( TRTSegmentFindingConfig_test
        SCRIPT python -m InDetConfig.TRTSegmentFindingConfig
        PROPERTIES TIMEOUT 600
        POST_EXEC_SCRIPT noerror.sh)

    atlas_add_test( TrackRecoConfig_test
        SCRIPT python -m InDetConfig.TrackRecoConfig
        PROPERTIES TIMEOUT 1800
        POST_EXEC_SCRIPT noerror.sh)

    atlas_add_test( ITkTrackRecoConfig_test
        SCRIPT python -m InDetConfig.ITkTrackRecoConfig --norun
        POST_EXEC_SCRIPT noerror.sh)

    atlas_add_test( VertexFindingConfigActsGaussAgaptive_test
        SCRIPT python -m InDetConfig.InDetPriVxFinderConfig ActsGaussAdaptiveMultiFinding
        POST_EXEC_SCRIPT noerror.sh)

    atlas_add_test( VertexFindingConfigIterative_test
        SCRIPT python -m InDetConfig.InDetPriVxFinderConfig IterativeFinding
        POST_EXEC_SCRIPT noerror.sh)

    atlas_add_test( VertexFindingConfigFastIterative_test
        SCRIPT python -m InDetConfig.InDetPriVxFinderConfig FastIterativeFinding
	POST_EXEC_SCRIPT noerror.sh)

    atlas_add_test( InDetGNNHardScatterSelection_test
        SCRIPT python -m InDetConfig.InDetGNNHardScatterSelectionConfig
        POST_EXEC_SCRIPT noerror.sh)


endif()
