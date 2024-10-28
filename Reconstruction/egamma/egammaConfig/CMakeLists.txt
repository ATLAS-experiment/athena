################################################################################
# Package: egammaConfig
################################################################################

# Declare the package name:
atlas_subdir( egammaConfig )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( test/*.sh )

atlas_add_test( egammaConfigFlagsTest
		SCRIPT python -m egammaConfig.egammaConfigFlags
		POST_EXEC_SCRIPT nopost.sh )

atlas_add_test( egammaReconstructionConfigTest
		SCRIPT python -m egammaConfig.egammaReconstructionConfig
		POST_EXEC_SCRIPT nopost.sh )

atlas_add_test( egammaxAODThinningConfigTest
		SCRIPT python -m egammaConfig.egammaxAODThinningConfig
		POST_EXEC_SCRIPT nopost.sh )

atlas_add_test( egammaSteeringConfigTest
		SCRIPT python -m egammaConfig.egammaSteeringConfig egamma.configOnly=True
		POST_EXEC_SCRIPT nopost.sh )

atlas_add_test( egammaLRTReconstructionConfigTest
		SCRIPT python -m egammaConfig.egammaLRTReconstructionConfig
		POST_EXEC_SCRIPT nopost.sh )

