# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""Configuration for MetaDataToCondAlg - reads metadata and populates ConditionStore"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def MetaDataToCondAlgCfg(flags, folderName, name=None):
    """
    Configure MetaDataToCondAlg to read from MetaDataStore and write to ConditionStore.

    This is used for direct in-file metadata mode where simulation/digitization
    parameters are stored in the file metadata without intermediate sqlite files.

    Args:
        flags: Configuration flags
        folderName: Folder path (e.g., '/Digitization/Parameters')
        name: Optional algorithm name (defaults to folder-based name)

    Returns:
        ComponentAccumulator with configured MetaDataToCondAlg
    """
    result = ComponentAccumulator()

    if name is None:
        # Create a name from the folder path (e.g., "/Digitization/Parameters" -> "DigiParamsMetaDataToCondAlg")
        cleanName = folderName.replace("/", "").replace("Parameters", "Params")
        name = f"{cleanName}MetaDataToCondAlg"

    alg = CompFactory.MetaDataToCondAlg(name, FolderName=folderName, OutputKey=folderName)

    result.addCondAlgo(alg)

    return result
