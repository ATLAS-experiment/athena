################################################################################
# Package: TileRecEx
################################################################################

# Declare the package name:
atlas_subdir( TileRecEx )

# Install files from the package:
atlas_install_runtime( share/TileRecEx_*.C share/DoTileOptimalFilteringAlgsRttHistograms.C )
atlas_install_scripts( share/TileRecEx_links.sh share/TileRecEx_links.csh )
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_install_scripts( python/RunTileCalibRec.py )

# Tests:
atlas_add_test( TileInputFiles_test
                SCRIPT python -m TileRecEx.TileInputFiles -r 485506 --inputDirectory /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TileRecEx/
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( RunTileTBDump_test
                SCRIPT python -m TileRecEx.RunTileTBDump -r 485865 --evtMax 3 --stat --inputDirectory /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TileRecEx/
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( RunTileCalibRec_physics_test
                SCRIPT RunTileCalibRec.py -r 485506 --evtMax 3 --physics --inputDirectory /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TileRecEx/
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( RunTileCalibRec_monocis_test
                SCRIPT RunTileCalibRec.py -r 485865 --evtMax 3 --mono-cis --inputDirectory /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TileRecEx/
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( RunTileCalibRec_cis_test
                SCRIPT RunTileCalibRec.py -r 485866 --evtMax 3 --cis --skipEvents 0 --inputDirectory /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TileRecEx/
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( RunTileCalibRec_pedestals_test
                SCRIPT RunTileCalibRec.py -r 485868 --evtMax 3 --pedestals --inputDirectory /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TileRecEx/
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( RunTileCalibRec_laser_test
                SCRIPT RunTileCalibRec.py -r 485872 --evtMax 3 --laser --inputDirectory /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TileRecEx/
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( TileD3PDConfig_test
                SCRIPT python -m TileRecEx.TileD3PDConfig
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT noerror.sh)
