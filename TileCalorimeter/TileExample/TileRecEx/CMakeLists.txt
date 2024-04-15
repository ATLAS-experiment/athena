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
                SCRIPT python -m TileRecEx.TileInputFiles -r 239908 --inputDirectory /eos/atlas/atlascerngroupdisk/det-tile/test/
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( RunTileTBDump_test
                SCRIPT python -m TileRecEx.RunTileTBDump -r 239908 --evtMax 3 --stat --inputDirectory /eos/atlas/atlascerngroupdisk/det-tile/test/
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( RunTileCalibRec_test
                SCRIPT RunTileCalibRec.py -r 239908 --evtMax 3 --physics --inputDirectory /eos/atlas/atlascerngroupdisk/det-tile/test/
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( TileD3PDConfig_test
                SCRIPT python -m TileRecEx.TileD3PDConfig
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT nopost.sh)
