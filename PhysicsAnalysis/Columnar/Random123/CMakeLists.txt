# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

# Declare the package name.
atlas_subdir( Random123 )

# Vendored, header-only third-party library (see Random123/README.atlas).
# Expose the Random123/ subtree so other packages can #include <Random123/...>.
atlas_add_library( Random123
                   INTERFACE
                   PUBLIC_HEADERS Random123 )
