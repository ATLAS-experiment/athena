################################################################################
# Package: TileSimEx
################################################################################

# Declare the package name:
atlas_subdir( TileSimEx )

# Install files from the package:
atlas_install_runtime( test/TileSimEx_TestConfig.xml share/*.C )
atlas_install_scripts( share/TileSimEx_links.sh share/TileSimEx_links.csh )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Tests:
atlas_add_test( TileSimD3PD_test
                SCRIPT python -m TileSimEx.TileSimD3PDConfig
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( TileSimOutput_test
                SCRIPT python -m TileSimEx.TileSimOutputConfig
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT nopost.sh)

