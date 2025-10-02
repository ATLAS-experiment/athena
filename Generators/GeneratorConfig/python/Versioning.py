# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from os import getenv

import re
versionRegex = re.compile(r'\(v\.(.*)\)$')

ignoredGenerators = ["Pythia8B", "Powheg"]


def legacyReleaseData():
    from pathlib import Path
    from PathResolver import PathResolver
    filePath = Path(PathResolver.FindCalibFile("GeneratorConfig/Legacy_AthGeneration_versions.txt"))
    data = {}
    with filePath.open() as f:
        header = f.readline().strip().split(",")
        generators = header[2:]
        for line in f:
            line = line.strip().split(",")
            item = {generator: version for generator, version in zip(generators, line[2:])}
            item['version'] = line[0]
            data[line[1]] = item
    return data

def legacyReleaseDataSampleTagOverrides():
    from pathlib import Path
    from PathResolver import PathResolver
    filePath = Path(PathResolver.FindCalibFile("GeneratorConfig/Legacy_SampleTagOverrides.txt"))
    data = {}
    with filePath.open() as f:
        f.readline() # skip header
        for line in f:
            line = line.strip()
            if line[0] == "#":
                continue
            line = line.split(",")
            data[int(line[0])] = line[1]
    return data

def legacyReleaseDataSampleGeneratorOverrides():
    from pathlib import Path
    from PathResolver import PathResolver
    filePath = Path(PathResolver.FindCalibFile("GeneratorConfig/Legacy_SampleGeneratorOverrides.txt"))
    data = {}
    with filePath.open() as f:
        f.readline() # skip header
        for line in f:
            line = line.strip()
            if line[0] == "#":
                continue
            line = line.split(",")
            versions = {}
            for i in range(1, len(line), 2):
                versions[line[i]] = line[i+1] if line[i+1] != "None" else None
            data[int(line[0])] = versions
    return data

def generatorsGetInitialVersionedDictionary(generators):
    output = {}
    for generator in generators:
        env_variable = f"{generator.upper()}VER"
        output[generator] = getenv(env_variable, None)
    return output

def generatorsGetFromMetadata(metadataString):
    output = {}
    metadataStringSplit = metadataString.split("+")
    for s in metadataStringSplit:
        match = versionRegex.search(s)
        if match:
            s = s.replace(match.group(0), "")
            output[s] = match.group(1)
        else:
            output[s] = None
    return output

def generatorsVersionedStringList(generatorsDictionary):
    list = []
    for generator, version in generatorsDictionary.items():
        if version is not None:
            list.append(f"{generator}(v.{version})")
        else:
            list.append(generator)
    return list

def generatorsVersionedString(generatorsVersionedList):
    return "+".join(generatorsVersionedList)


def GeneratorVersioningFixCfg(flags):
    from AthenaCommon.Logging import logging
    log = logging.getLogger("GeneratorsInfo")
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

    generatorsData = flags.Input.GeneratorsInfo
    log.info(f"Generators data: {generatorsData}")

    missingVersion = False
    for k, v in generatorsData.items():
        if v is None and k not in ignoredGenerators:
            missingVersion = True

    if not missingVersion:
        return ComponentAccumulator()

    log.info("At least one MC generator is missing version information. Attempting to fix...")

    from PyUtils.AMITagHelperConfig import inputAMITags
    tags = inputAMITags(flags, fixBroken=True, silent=True)
    tag = None
    if tags and tags[0].startswith("e"):
        tag = tags[0]

    # Fix specific samples directly
    releaseDataSampleGeneratorOverridesDict = legacyReleaseDataSampleGeneratorOverrides()
    if flags.Input.MCChannelNumber and flags.Input.MCChannelNumber in releaseDataSampleGeneratorOverridesDict:
        overrides = releaseDataSampleGeneratorOverridesDict[flags.Input.MCChannelNumber]
        log.warning(f"Overriding generators to {'+'.join(overrides.keys())}.")
        generatorsData = {}
        for generator, version in overrides.items():
            if version is not None:
                log.warning(f"Overriding version for {generator} to {version}.")
            generatorsData[generator] = version

        outputStringList = generatorsVersionedStringList(generatorsData)

        from EventInfoMgt.TagInfoMgrConfig import TagInfoMgrCfg
        return TagInfoMgrCfg(flags, tagValuePairs={"generators": generatorsVersionedString(outputStringList)})

    # Fix specific samples' e-tag
    releaseDataSampleTagOverridesDict = legacyReleaseDataSampleTagOverrides()
    if flags.Input.MCChannelNumber and flags.Input.MCChannelNumber in releaseDataSampleTagOverridesDict:
        log.warning(f"Overriding e-tag for sample {flags.Input.MCChannelNumber} to {releaseDataSampleTagOverridesDict[flags.Input.MCChannelNumber]}.")
        tag = releaseDataSampleTagOverridesDict[flags.Input.MCChannelNumber]

    # Retrieve generators release data
    releaseDataDict = legacyReleaseData()
    if tag not in releaseDataDict:
        log.warning(f"Could not find release data for tag {tag}.")
        return ComponentAccumulator()

    # Apply release data to missing generators
    releaseData = releaseDataDict[tag]
    for k, v in generatorsData.items():
        if v is None and k not in ignoredGenerators and k in releaseData:
            log.info(f"Setting version for {k} to {releaseData[k]}.")
            generatorsData[k] = releaseData[k]

    outputStringList = generatorsVersionedStringList(generatorsData)

    from EventInfoMgt.TagInfoMgrConfig import TagInfoMgrCfg
    return TagInfoMgrCfg(flags, tagValuePairs={"generators": generatorsVersionedString(outputStringList)})
