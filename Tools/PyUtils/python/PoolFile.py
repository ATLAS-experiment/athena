# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# @author: Sebastien Binet <binet@cern.ch>
# @date:   March 2007
#
#
from __future__ import with_statement, print_function

__author__  = "Sebastien Binet <binet@cern.ch>"

### --- data ------------------------------------------------------------------
__all__ = [
    'PoolFileCatalog',
    'extract_stream_names',
    'extract_streams_from_tag',
    'PoolOpts',
    'extract_items',
    'PoolRecord',
    'PoolFile',
    'DiffFiles',
    ]

### --- imports ---------------------------------------------------------------
import sys
import os
import shelve
from builtins import range

from dbm import whichdb

from .Helpers import ShutUp
from .Decorators import forking

### --- data ------------------------------------------------------------------
class Units (object):
    kb = 1024.
    Mb = 1024.*1024.

### --- implementations -------------------------------------------------------
class PoolFileCatalog(object):
    """ reverse-engineering of the POOL FileCatalog.
        allows to retrieve the physical filename from a logical one, provided
        that the file-id is known to the (real) PoolFileCatalog
    """
    DefaultCatalog = "xmlcatalog_file:PoolFileCatalog.xml"
    AllowedProtocols = (
        # see: PoolSvc::createCatalog
        # http://alxr.usatlas.bnl.gov/lxr/source/atlas/Database/AthenaPOOL/PoolSvc/src/PoolSvc.cxx?v=head#736
        "xmlcatalog_file:",  # POOL default
        "apcfile:",          # ATLAS_POOLCOND_PATH
        "prfile:",           # file via PathResolver
        "file:",             # simple file on local FS
        )
    
    def __init__ (self, catalog=None):
        super (PoolFileCatalog, self).__init__()
        self.catalog = None

        if catalog is None:
            # chase poolfilecatalog location
            catalog = os.environ.get("POOL_CATALOG", self.DefaultCatalog)

        if isinstance(catalog, str):
            catalog = [catalog]
            
        if not isinstance (catalog, (str, list)):
            raise TypeError(
                "catalog contact string should be a string or a list thereof! (got %r)"%
                type(catalog))

        osp = os.path
        def osp_exp(x):
            return osp.expanduser(osp.expandvars(x))

        def _handle_apcfile_old(x):
            """ return $ATLAS_POOLCOND_PATH/poolcond/x
            """
            if 'ATLAS_POOLCOND_PATH' not in os.environ:
                return osp_exp(x)
            pcp = os.environ["ATLAS_POOLCOND_PATH"]
            if x.startswith("apcfile:"):
                x = x[len("apcfile:"):]
            return osp_exp(osp.join(pcp, 'poolcond', x))

        def _handle_apcfile(x):
            """ return $ATLAS_POOLCOND_PATH/x
            """
            if 'ATLAS_POOLCOND_PATH' not in os.environ:
                return osp_exp(x)
            pcp = os.environ["ATLAS_POOLCOND_PATH"]
            if x.startswith("apcfile:"):
                x = x[len("apcfile:"):]
            return osp_exp(osp.join(pcp, x))

        def _handle_xmlcatalog_file(x):
            return osp_exp(x[len("xmlcatalog_file:"):])

        def _handle_prfile(x):
            x = x[len("prfile:"):]
            x = osp_exp(x)
            try:
                import AthenaCommon.Utils.unixtools as u
                return u.FindFile(x,
                                  os.environ['DATAPATH'].split(os.pathsep),
                                  os.R_OK)
            except ImportError:
                return x

        def _handle_file(x):
            x = x[len("file:"):]
            x = osp_exp(x)
            return x
        
        cat_dispatch = {
            "xmlcatalog_file:": _handle_xmlcatalog_file,
            "apcfile:": _handle_apcfile,
            "prfile:":  _handle_prfile,
            "file:":    _handle_file,
            }
        assert sorted(cat_dispatch.keys()) == sorted(self.AllowedProtocols), \
               "catalog dispatch keys does not match AllowedProtocols:" \
               "\n%s\n%s" % (sorted(cat_dispatch.keys()),
                             sorted(self.AllowedProtocols))

        from . import xmldict
        def _build_catalog(catalog):
            if not catalog.startswith(self.AllowedProtocols):
                raise ValueError(
                    "sorry PoolFile:PoolFileCatalog only supports %s"
                    " as a protocol for the POOL file catalog (got: '%s')"
                    % (self.AllowedProtocols, catalog)
                    )
            for protocol, handler in cat_dispatch.iteritems():
                if catalog.startswith(protocol):
                    catalog = handler(catalog)
                    break
            # make sure the catalog exists...
            import os

            if not os.path.exists (catalog):
                return {}
                # raise RuntimeError(
                #     'could not find any PoolFileCatalog in [%s]' % catalog
                #     )
        
       
            root = xmldict.ElementTree.parse (catalog).getroot()
            return dict(xmldict.xml2dict(root))

        errors = []
        cat = {'POOLFILECATALOG':{'File':[]}}
        for c in catalog:
            try:
                bc = _build_catalog(c)
                pc = bc.get('POOLFILECATALOG',{})
                files = []
                if pc:
                    files = pc.get('File',[])
                if isinstance(files, dict):
                    files = [files]
                cat['POOLFILECATALOG']['File'].extend(files)
            except Exception as err:
                errors.append(err)

        if errors:
            raise errors[0] # FIXME : should we customize this a bit ?

        self.catalog = cat
        pass

    def pfn (self, url_or_fid):
        """find the physical file name given a url or a file-id"""
        import os.path as osp
        url_or_fid = osp.expanduser(osp.expandvars(url_or_fid))
        import types
        if isinstance (url_or_fid, types.ListType):
            return [self._pfn(f) for f in url_or_fid]
        else:
            return self._pfn(url_or_fid)
        
    def _pfn (self, url_or_fid):
        """find the physical file name given a url or a file-id"""
        if not ('POOLFILECATALOG' in self.catalog):
            return None
        if not ('File' in self.catalog['POOLFILECATALOG']):
            return None

        PFN_IDX = 0 # take this pfn when alternates exist
        
        files = self.catalog['POOLFILECATALOG']['File']
        if isinstance(files, dict):
            # in case there where only one entry in the catalog
            files = [files]
        import re
        if url_or_fid.lower().startswith('fid:'):
            url_or_fid = url_or_fid[len('fid:'):]
        if re.compile (r'\w{8}-\w{4}-\w{4}-\w{4}-\w{12}$').match (url_or_fid):
            fid = url_or_fid.lower()
            # better to check consistency of catalog over all entries
            # than declare success on first match...
            match = {}
            for f in files:
                if f.ID.lower() == fid:
                    match[fid] = []
                    pfn = f.physical.pfn
                    if isinstance(pfn, (list,tuple)):
                        match[fid].append([i.name for i in pfn])
                    else:
                        match[fid].append([pfn.name])
            if len(match[fid])==1:
                return match[fid][0][PFN_IDX]
            if len(match[fid])>1:
                raise LookupError (
                    "more than one match for FID='%s'!\n%r"%(fid,match)
                    )
            raise KeyError ("no entry with FID='%s' in catalog" % fid)
        else:
            url = url_or_fid
            if url.lower().startswith("lfn:"):
                url = url[len("lfn:"):]
                # better to check consistency of catalog over all entries
                # than declare success on first match...
                match = {}
                for f in files:
                    if (f.logical != '' # no LFN for this entry
                        and f.logical.lfn.name == url):
                        match[url] = []
                        pfn = f.physical.pfn
                        if isinstance(pfn, (list,tuple)):
                            match[url].append([i.name for i in pfn])
                        else:
                            match[url].append([pfn.name])
                if len(match[url])==1:
                    return match[url][0][PFN_IDX]
                if len(match[url])>1:
                    raise LookupError (
                        "more than one match for LFN='%s'!\n%r"%(url,match)
                    )
                raise KeyError ("no entry with LFN='%s' in catalog" % url)
            # assume that if not LFN: then PFN:, no matter what...
            if url.lower().startswith("pfn:"):
                url = url[len("pfn:"):]
            return url

    def __call__ (self, url_or_fid):
        return self.pfn (url_or_fid)
    
    pass

def extract_stream_names(fname):
    """find the stream names ('StreamESD', 'StreamAOD',...) contained in a
       given POOL file
    @params:
     `fname`     the filename of the POOL file to inspect (can be LFN or PFN)

    example:
     >>> import PyUtils.PoolFile as pf
     >>> streams = pf.extract_stream_names ('ESD.pool.root')
     >>> print streams
     ['StreamESD']
    """
    import PyUtils.AthFile as af
    f = af.fopen(fname)
    return f.fileinfos['stream_names']

@forking
def extract_streams_from_tag (fname,
                              tree_name=None,
                              nentries=-1,
                              stream_refs=None):
    """find the GUID(s) of a list of streams which are contained inside a TAG
       file.
       @params:
       `fname`       the filename of the TAG file to inspect
                     (can be a LFN or FID)
       `tree_name`   the name of the TTree containing the stream-refs
                     (ROOT.APRDefaults.TTreeNames.EventTag is the default)
       `nentries`    the number of entries to inspect, among the ttree entries
                     (-1: all the entries)
       `stream_refs` a list of stream names
                     (ex: ['StreamAOD_ref', 'Stream1_ref'] or,
                          None - to inspect all the stream refs in the TAG)

    example:
     >>> streams = extract_streams_from_tag ('tag.pool')
     >>> from pprint import pprint
     >>> pprint (streams)
     {'Stream1_ref': ['BEE16671-B9F6-DA11-8219-00304871B611'],
      'StreamAOD_ref': ['96F3018E-A0AC-DD11-8115-000423D59D52'],
      'StreamESD_ref': ['384D0CFC-9FAC-DD11-A905-000423D59D52'],
      'StreamRDO_ref': ['22C5BD99-3059-DB11-8D44-0030488365E6']}
     >>> from PoolFile import PoolFileCatalog as pfc
     >>> pprint (pfc().pfn(streams['StreamAOD_ref'][0]))
     ['aod.pool']
    """
    
    import PyUtils.RootUtils as ru
    ROOT = ru.import_root()

    print("::: opening file [%s]..." % fname)
    # get the "final" file name (handles all kind of protocols)
    import PyUtils.AthFile as af
    try:
        protocol, fname = af.server.fname(fname)
    except Exception:
        print("::: warning: problem extracting file name from PoolFileCatalog")
        print("::: warning: will use [%s]" % fname)
    
    f = ROOT.TFile.Open (fname, "read")
    assert not f.IsZombie() and f.IsOpen(), \
           "problem opening POOL file [%s]"%fname

    if tree_name is None: tree_name = ROOT.APRDefaults.TTreeNames.EventTag
    t = f.Get( tree_name )
    assert isinstance(t, ROOT.TTree), \
           "could not retrieve tree [%s]" % tree_name
    
    # interesting branch names
    branches = [str(b.GetName()) for b in t.GetListOfBranches()
                if b.GetName().endswith ("_ref")]
    if stream_refs is None:
        stream_refs = branches
    else:
        _streams = stream_refs[:]
        stream_refs = []
        for ref in _streams:
            if ref not in branches:
                print("::: discarding [%s] from file chasing..."%ref)
            else:
                stream_refs.append (ref)
    if nentries <= 0: nentries = t.GetEntries()
    else:             nentries = min (nentries, t.GetEntries())
    print("::: chasing streams: %s in tree: [%s]" % (stream_refs, tree_name))
    print("::: ...over entries: %r" % nentries)
    
    # disable everything...
    t.SetBranchStatus ("*", 0)

    streams = dict()
    for ref in stream_refs:
        streams[ref] = list() # list of FileIDs, according to POOL
                              # ex: 'B2B485E1-BB37-DD11-984C-0030487A17BA'
        # but (re-)enable the branches we are interested in
        t.SetBranchStatus (ref, 1)
    
    import re
    # Pool token are of the form:
    # '[DB=7CCD8D32-BC37-DD11-967E-0030487CD916]\
    #  [CNT=POOLContainer_DataHeader]\
    #  [CLID=72FBBC6F-C8BE-4122-8790-DC627696C176]\
    #  [TECH=00000202]\
    #  [OID=0000008C-000002BA]'
    token = re.compile (r'\[DB=(?P<FID>.*?)\]'
                        r'\[CNT=(?P<CNT>.*?)\]'
                        r'\[CLID=(?P<CLID>.*?)\]'
                        r'\[TECH=(?P<TECH>.*?)\]'
                        r'\[OID=(?P<OID>.*?)\]')
    for i in range(nentries):
        t.GetEntry (i)
        for ref in stream_refs:
            try:
                token_str = getattr(t, ref)
            except (AttributeError, TypeError):
                # MN: TypeError is a bug in ROOT 5.34.25, fixed in 5.34.30
                # filthy work-around...
                try:
                    token_branch = t.GetBranch (ref)
                    token_branch.GetEntry(i)
                    token_str = token_branch.GetLeaf("Token").GetValueString()
                except Exception:
                    print("::: could not access stream-ref [%s] (entry #%i)",
                          (ref, i))
                    continue
            tok = token.match (token_str)
            if not tok:
                print("::: invalid POOL token: [%s]" % token_str)
                continue
            streams[ref].append (tok.group('FID'))

    for ref in stream_refs:
        streams[ref] = list(set(streams[ref]))
    return streams

class PoolOpts(object):
    # default names of APR file storage elements
    # copied here from APRDefaults.h for performance (as the first dictionary access takes 7 sec)
    class TTreeNames:
        EventData   = "CollectionTree"
        EventTag    = "POOLCollectionTree"
        DataHeader  = "POOLContainer"
        MetaData    = "MetaData"
    class RNTupleNames:
        EventData   = "EventData"
        EventTag    = "EventTag"
        DataHeader  = "DataHeader"
        MetaData    = "MetaData"

    FAST_MODE   = False
    SUPER_DETAILED_BRANCH_SZ = False
    READ_MODE   = "READ"
    POOL_HEADER = TTreeNames.DataHeader
    EVENT_DATA  = TTreeNames.EventData
    META_DATA   = TTreeNames.MetaData
    HDR_FORMAT  = "  %11s     %11s     %11s      %11s  %5s  %s"
    ROW_FORMAT  = "%12.3f kb %12.3f kb %12.3f kb %12.3f %8i  %s"

    @classmethod
    def isData(cls, name):
        return not name.startswith("##") and name != PoolOpts.POOL_HEADER

    @classmethod
    def isDataHeader(cls, name):
        return ( name == PoolOpts.POOL_HEADER ) or \
               ( name == PoolOpts.POOL_HEADER+"_DataHeader" )

    @classmethod
    def isEventData(cls, name):
        return name.startswith(PoolOpts.EVENT_DATA)

    @classmethod
    def isAugmentation(cls, name):
        return "_DAOD_" in name

    @classmethod
    def augmentationName(cls, name):
        s = (name+"__").split('_')[2]
        if s.endswith("Form"):
            s = s[:-4]
        return s

    @classmethod
    def isAugmentedHeader(cls, name):
        return name.startswith(PoolOpts.POOL_HEADER) and cls.isAugmentation(name)

    pass # class PoolOpts


def _get_total_size (branch):
   if PoolOpts.FAST_MODE:
       return -1.
   if not PoolOpts.SUPER_DETAILED_BRANCH_SZ:
       return branch.GetTotalSize()
   brSize = 0
   branch.LoadBaskets()
   for bnum in range(0, branch.GetWriteBasket()):
       basket = branch.GetBasket(bnum)
       brSize += basket.GetObjlen() - 8
   return brSize

def retrieveBranchInfos( branch, poolRecord, ident = "" ):
    fmt = "%s %3i %8.3f %8.3f %8.3f %s"
    if 0:
        out = fmt % ( ident,
                      branch.GetListOfBranches().GetSize(),
                      _get_total_size (branch), 
                      branch.GetTotBytes(),
                      branch.GetZipBytes(),
                      branch.GetName() )
        print(out)
        
    branches  = branch.GetListOfBranches()
    for b in branches:
        poolRecord.memSize  += _get_total_size (b) / Units.kb
        if (b.GetZipBytes() < 0.001):
            poolRecord.memSizeNoZip  += _get_total_size (b) / Units.kb
        poolRecord.diskSize += b.GetZipBytes() / Units.kb
        poolRecord = retrieveBranchInfos ( b, poolRecord, ident+"  " )
        
    return poolRecord

def make_pool_record (branch, dirType):
    memSize = _get_total_size (branch) / Units.kb
    zipBytes = branch.GetZipBytes()
    memSizeNoZip = memSize if zipBytes < 0.001 else 0.
    diskSize     = branch.GetZipBytes() / Units.kb
    return PoolRecord(branch.GetName(), memSize, diskSize, memSizeNoZip,
                      branch.GetEntries(),
                      dirType=dirType)

def extract_items(pool_file, verbose=True, items_type='eventdata'):
    """Helper function to read a POOL file and extract the item-list from the
    DataHeader content.
    @params
      `pool_file`  the name of the pool file to inspect
      `verbose`    self-explanatory
      `items_type` what kind of items one is interested in
                   allowed values: 'eventdata' 'metadata'
    Note: this function is actually executed in a forked sub-process
          if `fork` is True
    """
    _allowed_values = ('eventdata',
                       'metadata',)
    if items_type not in _allowed_values:
        err = "".join([
            "invalid argument for 'items_type'. ",
            "got: [%s] " % items_type,
            "(allowed values: %r)" % _allowed_values
            ])
        raise ValueError(err)
    import PyUtils.AthFile as af
    f = af.fopen(pool_file)
    key = '%s_items' % items_type
    items = f.fileinfos[key]
    if items is None:
        items = []
    return items

class PoolRecord(object):
    """
    """
    class Sorter:
        DiskSize      = "diskSize"
        MemSize       = "memSize"
        ContainerName = "name"

        @staticmethod
        def allowedValues():
            return [ PoolRecord.Sorter.DiskSize,
                     PoolRecord.Sorter.MemSize,
                     PoolRecord.Sorter.ContainerName ]
        pass
    def __init__(self, name, memSize, diskSize, memSizeNoZip, nEntries, dirType,
                 detailedInfos = ""):
        object.__init__(self)
        self.name          = name
        self.memSize       = memSize
        self.diskSize      = diskSize
        self.memSizeNoZip  = memSizeNoZip
        self.nEntries      = nEntries
        self.dirType       = dirType
        self.details       = detailedInfos
        self.augName       = ''
        return

class PoolFile(object):
    """
    A simple class to retrieve informations about the content of a POOL file.
    It should be abstracted from the underlying technology used to create this
    POOL file (Db, ROOT,...).
    Right now, we are using the easy and loosy solution: going straight to the
    ROOT 'API'.
    """
    
    def __init__(self, fileName, verbose=True):
        object.__init__(self)

        self._fileInfos = None
        self.keys       = None
        self.dataHeader = PoolRecord("DataHeader", 0, 0, 0,
                                     nEntries = 0,
                                     dirType = "T")
        self.augNames   = set()
        self.dataHeaderA = {}
        self.data       = []
        self.verbose    = verbose

        # get the "final" file name (handles all kind of protocols)
        try:
            import PyUtils.AthFile as af
            protocol, fileName = af.server.fname(fileName)
        except Exception as err:
            print("## warning: problem opening PoolFileCatalog:\n%s"%err)
            import traceback
            traceback.print_exc(err)
            pass
        
        self.poolFile = None
        dbFileName = whichdb( fileName )
        if dbFileName not in ( None, '' ):
            if self.verbose is True:
                print("## opening file [%s]..." % str(fileName))
            db = shelve.open( fileName, 'r' )
            if self.verbose is True:
                print("## opening file [OK]")
            report = db['report']
            self._fileInfos = report['fileInfos']
            self.dataHeader = report['dataHeader']
            self.data       = report['data']
        else:
            if self.verbose is True:
                print("## opening file [%s]..." % str(fileName))
            self.__openPoolFile( fileName )
            if self.verbose is True:
                print("## opening file [OK]")
            self.__processFile()
            
        return

    def __openPoolFile(self, fileName):
        # hack to prevent ROOT from loading graphic libraries and hence bother
        # our fellow Mac users
        if self.verbose is True:
            print("## importing ROOT...")
        import PyUtils.RootUtils as ru
        ROOT = ru.import_root()
        if self.verbose is True:
            print("## importing ROOT... [DONE]")
        # prevent ROOT from being too verbose
        rootMsg = ShutUp()
        rootMsg.mute()
        ROOT.gErrorIgnoreLevel = ROOT.kFatal

        poolFile = None
        try:
            poolFile = ROOT.TFile.Open( fileName, PoolOpts.READ_MODE )
        except Exception as e:
            rootMsg.unMute()
            print("## Failed to open file [%s] !!" % fileName)
            print("## Reason:")
            print(e)
            print("## Bailing out...")
            raise IOError("Could not open file [%s]" % fileName)

        rootMsg.unMute()

        if poolFile is None:
            print("## Failed to open file [%s] !!" % fileName)
            msg = "Could not open file [%s]" % fileName
            raise IOError(msg)

        self.poolFile = poolFile
        assert self.poolFile.IsOpen() and not self.poolFile.IsZombie(), \
               "Invalid POOL file or a Zombie one" 
        self._fileInfos = {
            'name' : self.poolFile.GetName(),
            'size' : self.poolFile.GetSize(),
            }
        return

    def __processFile(self):
        ## first we try to fetch the DataHeader
        name  = PoolOpts.POOL_HEADER
        dhKey = self.poolFile.FindKey( name )
        if dhKey:
            nEntries = dhKey.ReadObj().GetEntries()
        else:
            name  = PoolOpts.POOL_HEADER + "_DataHeader"
            dhKey = self.poolFile.FindKey( name )
            if dhKey:
                nEntries = dhKey.ReadObj().GetEntries()
            else:
                nEntries = 0

        keys = []
        containers = []
        for k in self.poolFile.GetListOfKeys():
            treename = k.GetName()
            containerName = k.ReadObj().GetName()
            if containerName not in containers:
                keys.append(k)
                containers.append(containerName)
                pass
            if treename.startswith(PoolOpts.POOL_HEADER) and not treename.endswith('Form'):
                self.dataHeaderA[PoolOpts.augmentationName(k.GetName())] = \
                    PoolRecord("DataHeader", 0, 0, 0,
                               nEntries = k.ReadObj().GetEntries(),
                               dirType = "T")

        keys.sort (key = lambda x: x.GetName())
        self.keys = keys
        del containers
        
        for k in keys:
            tree = k.ReadObj()
            name = tree.GetName()

            if not PoolOpts.isDataHeader(name) and not PoolOpts.isData(name) :
                continue

            if PoolOpts.isDataHeader(name):
                if name == PoolOpts.POOL_HEADER:
                    contName     = "DataHeader"
                else:
                    contName     = name.replace(PoolOpts.POOL_HEADER+"_", "" )
                memSize      = tree.GetTotBytes() / Units.kb
                diskSize     = tree.GetZipBytes() / Units.kb
                memSizeNoZip = 0.0
                if diskSize < 0.001:
                    memSizeNoZip = memSize
                nEntries     = tree.GetEntries()
                ## try to also handle non-T/P separated DataHeaders
                ## (from old files)...
                dhBranchNames = [
                    br.GetName() for br in tree.GetListOfBranches()
                    if br.GetName().count("DataHeader_p") > 0
                ]
                if len(dhBranchNames) == 1:
                    dhBranch = tree.GetBranch(dhBranchNames[0])
                    poolRecord = retrieveBranchInfos(
                        dhBranch,
                        PoolRecord( contName, 0., 0., 0.,
                                    nEntries,
                                    dirType = "T" ),
                        ident = "  "
                        )
                else:
                    poolRecord = PoolRecord(contName, memSize, diskSize, memSizeNoZip,
                                            nEntries,
                                            dirType = "T")
                    
                if contName == "DataHeader":
                    self.dataHeader = poolRecord
                else:
                    self.data += [ poolRecord ]
            elif PoolOpts.isData(name):
                if not hasattr(tree, 'GetListOfBranches'):
                    continue
                branches = tree.GetListOfBranches()
                ## print ("=-=->",name,type(tree).__name__)
                dirType = "T"
                if name in (PoolOpts.EVENT_DATA, PoolOpts.META_DATA):
                    dirType = "B"
                for i,branch in enumerate(branches):
                    poolRecord = retrieveBranchInfos(
                        branch,
                        make_pool_record(branch, dirType),
                        ident = "  "
                        )
                    ## if dirType == "T":
                    ##     poolRecord.name = name.replace( PoolOpts.EVENT_DATA,
                    ##                                     "" )
                    poolRecord.augName = PoolOpts.augmentationName(name)
                    self.augNames.add(poolRecord.augName)
                    self.data += [ poolRecord ]
            else:
                print("WARNING: Don't know how to deal with branch [%s]" % \
                      name)

            pass # loop over keys
        
        return
    
    def fileInfos(self):
        return os.linesep.join( [
            "File:" + self._fileInfos['name'],
            "Size: %12.3f kb" % (self._fileInfos['size'] / Units.kb),
            "Nbr Events: %i" % self.dataHeader.nEntries
            ] )

    
    def checkFile(self, sorting = PoolRecord.Sorter.DiskSize):
        if self.verbose is True:
            print(self.fileInfos())
            if len(self.augNames) > 1:
                for aug in self.augNames:
                    if len(aug) > 0:
                        print( "Nbr %s Events: %i" % (aug, self.dataHeaderA[aug].nEntries) )

        ## sorting data
        data = self.data
        if sorting in PoolRecord.Sorter.allowedValues():
            import operator
            data.sort(key = operator.attrgetter(sorting) )

        def _get_val(x, dflt=-999.):
            if PoolOpts.FAST_MODE:
                return dflt
            return x

        totMemSize  = _get_val(self.dataHeader.memSize, dflt=0.)
        totDiskSize = self.dataHeader.diskSize
        
        def _safe_div(num,den):
            if float(den) == 0.:
                return 0.
            return num/den   
                 
        if self.verbose is True:
            print("")
            print("="*80)
            print(PoolOpts.HDR_FORMAT % ( "Mem Size", "Disk Size","Size/Evt",
                                          "MissZip/Mem","items",
                                          "(X) Container Name (X=Tree|Branch)" ))
            print("="*80)
            
            print(PoolOpts.ROW_FORMAT % (
                _get_val (self.dataHeader.memSize),
                self.dataHeader.diskSize,
                _safe_div(self.dataHeader.diskSize,float(self.dataHeader.nEntries)),
                _get_val (_safe_div(self.dataHeader.memSizeNoZip,
                                    self.dataHeader.memSize)),
                self.dataHeader.nEntries,
                "("+self.dataHeader.dirType+") "+self.dataHeader.name
                ))
            print("-"*80)

        totMemSizeA = {}
        totDiskSizeA = {}
        for d in data:
            totMemSize  += 0. if PoolOpts.FAST_MODE else d.memSize
            totDiskSize += d.diskSize
            memSizeNoZip = d.memSizeNoZip/d.memSize if d.memSize != 0. else 0.
            aug = d.augName
            totMemSizeA[aug]  = totMemSizeA.get(aug,0.) + d.memSize
            totDiskSizeA[aug] = totDiskSizeA.get(aug,0.) + d.diskSize
            if self.verbose is True:
                print(PoolOpts.ROW_FORMAT % (
                    _get_val (d.memSize),
                    d.diskSize,
                    _safe_div(d.diskSize, float(self.dataHeader.nEntries)),
                    _get_val (memSizeNoZip),
                    d.nEntries,
                    "("+d.dirType+") "+d.name
                    ))

        if self.verbose is True:
            print("="*80)
            if len(self.augNames) > 1:
                augs = sorted(self.augNames)
                for a in augs:
                    print(PoolOpts.ROW_FORMAT % (
                        totMemSizeA[a], totDiskSizeA[a],
                        _safe_div(totDiskSizeA[a], float(self.dataHeaderA[a].nEntries)),
                        0.0,
                        self.dataHeaderA[a].nEntries,
                        "Aug Stream: " + ('MAIN' if a=='' else a)
                        ))
                print("-"*80)
            print(PoolOpts.ROW_FORMAT % (
                totMemSize, totDiskSize,
                _safe_div(totDiskSize, float(self.dataHeader.nEntries)),
                0.0, self.dataHeader.nEntries,
                "TOTAL (POOL containers)"
                ))
            print("="*80)
            if PoolOpts.FAST_MODE:
                print("::: warning: FAST_MODE was enabled: some columns' content ",)
                print("is meaningless...")
        return

    def detailedDump(self, bufferName = sys.stdout.name ):
        if self.poolFile is None or \
           self.keys is None:
            print("Can't perform a detailedDump with a shelve file as input !")
            return
                  
        if bufferName == sys.stdout.name:
            bufferName = "/dev/stdout"
        out = open( bufferName, "w" )
        sys.stdout.flush()
        save_stdout_fileno = os.dup (sys.stdout.fileno())
        os.dup2( out.fileno(),        sys.stdout.fileno() )

        out.write( "#" * 80 + os.linesep )
        out.write( "## detailed dump" + os.linesep )
        out.flush()
        
        for key in self.keys:
            tree = key.ReadObj()
            name = tree.GetName()

            if PoolOpts.isDataHeader(name) or \
               PoolOpts.isData(name):
                try:
                    print ("=== [%s] ===" % name, file=sys.stderr)
                    tree.Print()
                except Exception as err:
                    print ("Caught:",err, file=sys.stderr)
                    print (sys.exc_info()[0], file=sys.stderr)
                    print (sys.exc_info()[1], file=sys.stderr)
                    pass
                pass
            pass
        out.write( "#" * 80 + os.linesep )
        out.flush()
        out.write( "#" * 80 + os.linesep )
##         out.write( "#### Map ####" + os.linesep )
##         out.flush()
##         self.poolFile.Map()
##         out.write( "#" * 80 + os.linesep )
        out.flush()
        if bufferName != "<stdout>":
            out.close()
            sys.stdout.close()
            sys.stdout = open (save_stdout_fileno, 'a')
        return

    def poolRecord(self, name):
        """
        Return a PoolRecord according to its (branch) name
        Raise KeyError if no match is found
        """
        for data in self.data:
            if data.name == name:
                return data
        raise KeyError("No PoolRecord with name [%s]" % name)

    def saveReport (self, fileName):
        """
        Save all the gathered informations into a python shelve or a CSV file
        (depending on the @param `fileName` extension)
        """
        import os
        if os.path.splitext(fileName)[-1] == '.csv':
            return self._save_csv_report (fileName)
        return self._save_shelve_report (fileName)
    
    def _save_shelve_report(self, fileName):
        """
        Save all the gathered informations into a python shelve
        Data can then be read like so:
         >>> import shelve
         >>> db = shelve.open( 'myfile.dat', 'r' )
         >>> report = db['report']
         >>> print ('fileSize:',report['fileSize'])
         >>> print ('dataHeader/memSize:',report['dataHeader'].memSize)
         >>> for d in report['data']:
         ...   print ('data:',d.name,d.nEntries,d.memSize)
        """
        import shelve, os
        if os.path.exists (fileName):
            os.unlink (fileName)
        db = shelve.open (fileName)
        db['report'] = {
            'fileInfos'  : self._fileInfos,
            'nbrEvts'    : self.dataHeader.nEntries,
            'dataHeader' : self.dataHeader,
            'data'       : self.data
            }
        db.close()
        return

    def _save_csv_report(self, fileName):
        """
        Save all the gathered informations into a CSV file
        """
        import csv, os
        if os.path.exists (fileName):
            os.unlink (fileName)
        args = {'newline' : ''}
        f = open (fileName, 'w', **args)
        o = csv.writer (f)
        o.writerow (['file name', self._fileInfos['name']])
        o.writerow (['file size', self._fileInfos['size']])
        o.writerow (['nbr evts',  self.dataHeader.nEntries])
        o.writerow (['mem size', 'disk size', 'mem size nozip', 'items',
                     'container name', 'branch type'])

        for d in self.data:
            o.writerow ([d.memSize, d.diskSize, d.memSizeNoZip,
                         d.nEntries, d.name, d.dirType])
        f.close()
        return

    def __del__(self):
        if self.poolFile and hasattr(self.poolFile, 'Close'):
            try:
                self.poolFile.Close()
                self.poolFile = None
            except Exception as err:
                print("WARNING:",err)
                pass
            
    pass # class PoolFile

class DiffFiles(object):
    """
    A helper class to compare 2 POOL files and check that they match, both in
    terms of containers' content and containers' sizes
    """

    def __init__(self, refFileName, chkFileName, verbose = False, ignoreList = None, strict = False):
        object.__init__(self)

        self.verbose = verbose
        self.strict = strict
        refFileName = os.path.expandvars( os.path.expanduser( refFileName ) )
        chkFileName = os.path.expandvars( os.path.expanduser( chkFileName ) )

        if ignoreList is None:
            ignoreList = []
            
        try:
            self.refFile = PoolFile( refFileName )
            self.chkFile = PoolFile( chkFileName )
            self.ignList = sorted( ignoreList )
        except Exception as err:
            print("## Caught exception [%s] !!" % str(err.__class__))
            print("## What:",err)
            print(sys.exc_info()[0])
            print(sys.exc_info()[1])
            raise(err)
        except Exception:
            print("## Caught something !! (don't know what)")
            print(sys.exc_info()[0])
            print(sys.exc_info()[1])
            err  = "Error while opening POOL files !"
            err += " chk : %s%s" % ( chkFileName, os.linesep )
            err += " ref : %s%s" % ( refFileName, os.linesep )
            raise Exception(err)
        
        self.allGood = True
        self.summary = []
        
        self.__checkDiff()
        return

    def __checkDiff(self):

        self.summary += [
            "=" * 80,
            "::: Comparing POOL files...",
            " ref : %s" % self.refFile._fileInfos['name'],
            " chk : %s" % self.chkFile._fileInfos['name'],
            "-" * 80,
            ]

        if self.chkFile.dataHeader.nEntries != \
           self.refFile.dataHeader.nEntries :
            self.summary += [
                "## WARNING: files don't have the same number of entries !!",
                "   ref : %r" % self.refFile.dataHeader.nEntries,
                "   chk : %r" % self.chkFile.dataHeader.nEntries,
                ]
        
        refNames = sorted( [d.name for d in self.refFile.data] )
        chkNames = sorted( [d.name for d in self.chkFile.data] )

        if chkNames != refNames:
            self.summary += [
                "## ERROR: files don't have the same content !!",
                ]
            addNames = [ n for n in chkNames if n not in refNames ]
            if len( addNames ) > 0:
                self.summary += [ "## collections in 'chk' and not in 'ref'" ]
                for n in addNames:
                    self.summary += [ "  + %s" % n ]
            subNames = [ n for n in refNames if n not in chkNames ]
            if len( subNames ) > 0:
                self.summary += [ "## collections in 'ref' and not in 'chk'" ]
                for n in subNames:
                    self.summary += [ "  - %s" % n ]
            self.allGood = False
            pass

        if len(self.ignList) > 0:
                self.summary += [ "## Ignoring the following:" ]
                for n in self.ignList:
                    self.summary += [ "  %s" % n ]

        commonContent = [ d for d in chkNames if (d in refNames and d not in self.ignList)]

        if not self.allGood:
            self.summary += [ "=" * 80 ]
        self.summary += [ "::: comparing common content (mem-size / disk-size)..." ]

        for name in commonContent:
            chkMemSize = self.chkFile.poolRecord(name).memSize
            refMemSize = self.refFile.poolRecord(name).memSize
            chkDiskSize = self.chkFile.poolRecord(name).diskSize
            refDiskSize = self.refFile.poolRecord(name).diskSize

            if chkMemSize != refMemSize or (self.strict and chkDiskSize != refDiskSize):
                self.summary += [
                    "[ERR] %12.3f / %12.3f kb (ref) ==> %12.3f / %12.3f kb (chk) | %s" % \
                    ( refMemSize,refDiskSize,chkMemSize,chkDiskSize, name )
                    ]
                self.allGood = False
            elif self.verbose:
                self.summary += [
                    " [OK] %12.3f/%12.3f kb                                 | %s" % \
                    ( chkMemSize, chkDiskSize, name )
                    ]

        self.summary += [ "=" * 80 ]
        
        ## final decision
        if self.allGood: self.summary += [ "## Comparison : [OK]"  ]
        else:            self.summary += [ "## Comparison : [ERR]" ]

        return self.allGood

    def status(self):
        if self.allGood: return 0
        else:            return 1

    def printSummary(self, out = sys.stdout):
        for i in self.summary:
            out.writelines( i + os.linesep )
            pass
        return
    
class Counter(object):
    """
    A counter just contains an item list (pairs class-name/sg-key) and the size
    """
    size = 0
    def __init__(self, name, itemList):
        object.__init__(self)
        self.name = name
        self.itemList = itemList
    pass # Counter


