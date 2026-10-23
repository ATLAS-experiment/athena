# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PROCTools )

# Install files from the package:
atlas_install_scripts( python/UploadAMITag.py
                       python/UploadTfAMITag.py
                       python/xAODDigest.py
                       python/xAODHist.py
                       python/xAODHistSize.py
                       python/update_ci_reference_files.py )

# Install digest reference files for easy access
atlas_install_data( data/*.ref
                    data/*.txt
                    data/*.yaml )
