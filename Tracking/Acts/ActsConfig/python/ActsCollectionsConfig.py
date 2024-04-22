# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

class TrackBackends:
    # Allowed types
    types = ('xAOD::TrackSummaryContainer',
             'xAOD::TrackStateContainer',
             'xAOD::TrackParametersContainer',
             'xAOD::TrackJacobianContainer',
             'xAOD::TrackMeasurementContainer',
             'xAOD::TrackSurfaceContainer')
    # Allowed post-fixes
    postFixes = ('TrackSummary',
                 'TrackStates',
                 'TrackParameters',
                 'TrackJacobians',
                 'TrackMeasurements',
                 'TrackStateSurfaces',
                 'TrackSurfaces')
    
    def __init__(self):
        self.collections = dict()
        for el in TrackBackends.postFixes:
            self.collections[el] = None

    def __str__(self) -> str:
        message = "[ "
        for el in TrackBackends.postFixes:
            message += f"{el}={self.collections[el]} " 
        message += "]"
        return message
            
    @staticmethod
    def extractPrefix(collection: str) -> str:
        assert isinstance(collection, str)        
        for el in TrackBackends.postFixes:
            if collection.endswith(el):
                return collection.replace(el, "")
        # If not in allowed post fixes we raise Exception
        raise Exception(f"Collection {collection} is a NOT KNOWN ACTS track backend, prefix could not be deduced")
        
    def isValid(self) -> bool:
        for (key, value) in self.collections.items():
            if value is None:
                return False
        return True
        
    def addCollection(self,
                      collection: str) -> None:
        assert isinstance(collection, str)
        for el in TrackBackends.postFixes:
            if collection.endswith(el):
                if self.collections[el] is not None:
                    raise Exception(f"Trying to add collection '{collection}', but this backend is already recorded")
                self.collections[el] = collection
                return
        # If not in allowed post fixes we raise Exception
        raise Exception(f"Collection {collection} is a NOT KNOWN ACTS track backend")
        
def ActsSpacePointReaderAlgCfg(flags,
                               name: str,
                               **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.ActsTrk.SpacePointReader(name=name, **kwargs))
    return acc

def ActsTrackReaderAlgCfg(flags,
                          prefix: str) -> ComponentAccumulator:
    assert isinstance(prefix, str)
    """
    Setup algorithm that reads xAOD track backends and produced TrackContainer
    name - the collections prefix, for consistency it also is the prefix of the output container name   
    """
    acc = ComponentAccumulator()
    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
    acc.addEventAlgo(CompFactory.ActsTrk.TrackContainerReader(f"{prefix}TrackContainerReaderAlg",
                                                               TrackContainer=prefix+"Tracks",
                                                               TrackingGeometryTool=acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags))
                                                              ))
    return acc

def ActsPoolReadCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    StoredTracks = dict()
    
    typedCollections = flags.Input.TypedCollections
    for typedCollection in typedCollections:
        [colType, colName] = typedCollection.split('#')

        # Space Point Collections
        if colType == "xAOD::SpacePointContainer":
            acc.merge(ActsSpacePointReaderAlgCfg(flags,
                                                 name=f"Acts{colName}ReaderAlg",
                                                 SpacePointKey=colName))
            continue

        # Track Backend Collections
        if colType in TrackBackends.types:
            prefix = TrackBackends.extractPrefix(colName)
            StoredTracks[prefix] = StoredTracks.get(prefix, TrackBackends())
            StoredTracks[prefix].addCollection(colName)
            continue

    for (key, backends) in StoredTracks.items():
        if not backends.isValid():
            raise Exception(f'Track backends with prefix {key} are not consistent. Be sure all the backends have been properly persistified in the file: {backends}')
        acc.merge(ActsTrackReaderAlgCfg(flags, key))
    
    return acc

if __name__ == "__main__":
    # test reading
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    flags = initConfigFlags()
    flags.fillFromArgs()
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))
    cfg.merge(ActsPoolReadCfg(flags))

    status = cfg.run()
    if status.isFailure():
        import sys
        sys.exit("Execution failed")
