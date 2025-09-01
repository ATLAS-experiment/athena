# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# @file PyUtils.RootUtils
# @author Sebastien Binet
# @purpose a few utils to ease the day-to-day work with ROOT
# @date November 2009

__doc__ = "a few utils to ease the day-to-day work with ROOT"
__author__ = "Sebastien Binet"

__all__ = [
    'import_root',
    'root_compile',
    ]

### imports -------------------------------------------------------------------
import os
import re
from functools import cache

### functions -----------------------------------------------------------------
def import_root(batch=True):
    """a helper method to wrap the 'import ROOT' statement to prevent ROOT
    from screwing up the display or loading graphics libraries when in batch
    mode (which is the default.)

    e.g.
    >>> ROOT = import_root(batch=True)
    >>> f = ROOT.TFile.Open(...)
    """
    import ROOT
    ROOT.gROOT.SetBatch(batch)
    if batch:
        ROOT.PyConfig.IgnoreCommandLineOptions = True
    import cppyy  # noqa: F401
    if os.environ.get('GLIBCXX_USE_CXX11_ABI') == '0':
        cmd = ROOT.gSystem.GetMakeSharedLib()
        if cmd.find('GLIBCXX_USE_CXX11_ABI') < 0:
            cmd = cmd.replace ('$SourceFiles', '$SourceFiles -D_GLIBCXX_USE_CXX11_ABI=0 ')
            ROOT.gSystem.SetMakeSharedLib(cmd)
    return ROOT

_tempfiles = []
_first_compile = True
def root_compile(src=None, fname=None, batch=True):
    """a helper method to compile a set of C++ statements (via ``src``) or
    a C++ file (via ``fname``) via ACLiC
    """
    if src is not None and fname is not None:
        raise ValueError("'src' xor 'fname' should be not None, *not* both")

    if src is None and fname is None:
        raise ValueError("'src' xor 'fname' should be None, *not* both")

    # Cling bug workaround: Cling will try to find a definition for the
    # hidden __gmon_start__ by opening all libraries on LD_LIBRARY_PATH.
    # But it will crash if it encounters a separate-debug library.
    # Work around by adding a dummy definition of __gmon_start__.
    # See !31633.
    global _first_compile
    if _first_compile:
        _first_compile = False
        root_compile ('extern "C" { void __gmon_start__(){}; }', None, True)
    return _root_compile (src, fname, batch)

def _root_compile (src, fname, batch):
    import os
    from .Helpers import ShutUp as root_shutup
    
    ROOT = import_root(batch=batch)
    compile_options = "f"
    if 'dbg' in os.environ.get('CMTCONFIG', 'opt'):
        compile_options += 'g'
    else:
        compile_options += 'O'

    src_file = None
    if src:
        import textwrap
        import tempfile
        src_file = tempfile.NamedTemporaryFile(prefix='root_aclic_',
                                               suffix='.cxx')
        src_file.write(textwrap.dedent(src).encode())
        src_file.flush()
        src_file.seek(0)
        fname = src_file.name

        # Apparently, cling caches files by inode.
        # If you ask it to read a file that has the same inode as one
        # that it has already read, then it will just use the cached
        # contents rather than rereading.  This, however, doesn't play
        # very well if we're reading temp files, where inodes may be reused,
        # giving rise to hard-to-reproduce failures.
        #
        # Try to avoid this by keeping the temp files open until the
        # the program exits.
        _tempfiles.append (src_file)
        pass

    elif fname:
        import os.path as osp
        fname = osp.expanduser(osp.expandvars(fname))
        pass
        
    assert os.access(fname, os.R_OK), "could not read [%s]"%(fname,)
    orig_root_lvl = ROOT.gErrorIgnoreLevel
    ROOT.gErrorIgnoreLevel = ROOT.kWarning
    try:
        with root_shutup():
            sc = ROOT.gSystem.CompileMacro(fname, compile_options)
        if sc == ROOT.kFALSE:
            raise RuntimeError(
                'problem compiling ROOT macro (rc=%s)'%(sc,)
                )
    finally:
        ROOT.gErrorIgnoreLevel = orig_root_lvl
    return
        
@cache
def _pythonize_tfile():
    import cppyy
    root = import_root()
    import PyUtils.Helpers as H
    with H.ShutUp(filters=[
        re.compile(
            'TClass::TClass:0: RuntimeWarning: no dictionary for.*'),
        re.compile(
            'Warning in <TEnvRec::ChangeValue>: duplicate entry.*'
            ),
        ]):
        cppyy.load_library("libRootUtilsPyROOTDict")
        _ = root.RootUtils.PyBytes
        #MN: lines below fail in ROOT6 if PCM from RootUtils is not found
        read_root_file = root.RootUtils._pythonize_read_root_file
        tell_root_file = root.RootUtils._pythonize_tell_root_file
        pass
    def read(self, size=-1):
        """read([size]) -> read at most size bytes, returned as a string.

        If the size argument is negative or omitted, read until EOF is reached.
        Notice that when in non-blocking mode, less data than what was requested
        may be returned, even if no size parameter was given.

        FIXME: probably doesn't follow python file-like conventions...
        """
        SZ = 4096

        if size>=0:
            #size = _adjust_sz(size)
            #print ("-->0",self.tell(),size)
            c_buf = read_root_file(self, size)
            if c_buf and c_buf.sz:
                v = c_buf.buf
                return bytes([ord(v[i]) for i in range(v.size())])
            return ''
        else:
            size = SZ
            out = []
            while True:
                #size = _adjust_sz(size)
                c_buf = read_root_file(self, size)
                if c_buf and c_buf.sz:
                    v = c_buf.buf
                    chunk = bytes([ord(v[i]) for i in range(v.size())])
                    out.append(chunk)
                else:
                    break
            return b''.join(out)
            
    root.TFile.read = read
    del read
    
    root.TFile.seek = root.TFile.Seek
    root.TFile.tell = lambda self: tell_root_file(self)
    ## import os
    ## def tell(self):
    ##     fd = os.dup(self.GetFd())
    ##     return os.fdopen(fd).tell()
    ## root.TFile.tell = tell
    ## del tell
    return 


def _getLeaf (l):
    tname = l.GetTypeName()
    ndat = l.GetNdata()
    if (l.GetLeafCount()  # a varying length array
        or ndat > 1):     # a fixed size array
        if tname in ['UInt_t', 'Int_t', 'ULong_t', 'Long_t', 'ULong64_t', 'Long64_t', 'UShort_t', 'Short_t', 'Bool_t']:
            return tuple(l.GetValueLong64(i) for i in range(ndat))
        elif tname in ['Float_t', 'Double_t', 'Float16_t', 'Double32_t']:
            return tuple(l.GetValue(i) for i in range(ndat))
        elif tname in ['UChar_t', 'Char_t']:
            try:
                return l.GetValueString() # TLeafC for variable size string
            except Exception:
                return tuple(l.GetValueLong64(i) for i in range(ndat)) # TLeafB for 8-bit integers
    elif ndat == 1:  # a single value
        if tname in ['UInt_t', 'Int_t', 'ULong_t', 'Long_t', 'ULong64_t', 'Long64_t', 'UShort_t', 'Short_t', 'Bool_t']:
            return l.GetValueLong64()
        elif tname in ['Float_t', 'Double_t', 'Float16_t', 'Double32_t']:
            return l.GetValue()
        elif tname in ['UChar_t', 'Char_t']:
            try:
                return l.GetValueString()  # TLeafC for variable size string
            except Exception:
                return l.GetValueLong64()  # TLeafB for 8-bit integers

    return None

class RootFileDumper(object):
    """
    A helper class to dump in more or less human readable form the content of
    any TTree.
    """
    
    def __init__(self, fname, tree_name=None):
        object.__init__(self)

        ROOT = import_root()

        # remember if an error or problem occurred during the dump
        self.allgood = True
        
        self.root_file = ROOT.TFile.Open(fname)
        if (self.root_file is None or
            not isinstance(self.root_file, ROOT.TFile) or
            not self.root_file.IsOpen()):
            raise IOError('could not open [%s]'% fname)

        self.__init_obj(tree_name)

        if 0:
            self._trees = []
            keys = [k.GetName() for k in self.root_file.GetListOfKeys()]
            for k in keys:
                o = self.root_file.Get(k)
                if isinstance(o, ROOT.TTree):
                    self._trees.append(k)
                    pass

        return

    def __init_obj(self, obj_name):

        ROOT = import_root()
        from PyUtils.PoolFile import PoolOpts
        TTreeNames = PoolOpts.TTreeNames
        RNTupleNames = PoolOpts.RNTupleNames

        if obj_name is None:
            for name, klass in ((TTreeNames.EventData, ROOT.TTree), (RNTupleNames.EventData, ROOT.RNTuple)):
                if (obj := self.root_file.Get(name)) and isinstance(obj, klass):
                    self.obj_name = name
                    break
            else:
                raise AttributeError('No TTree named %r or RNTuple named %r in file %r' %
                                     (TTreeNames.EventData, RNTupleNames.EventData,
                                      self.root_file.GetName()))
        else:
            if (not (obj := self.root_file.Get(obj_name)) or
                not isinstance(obj, ROOT.TTree) and not isinstance(obj, ROOT.RNTuple)):
                raise AttributeError('No TTree or RNTuple named %r in file %r' %
                                     (obj_name, self.root_file.GetName()))
            self.obj_name = obj_name

        if isinstance(obj, ROOT.RNTuple):
            try:
                self.obj = ROOT.RNTupleReader.Open(obj)
            except AttributeError:
                self.obj = ROOT.Experimental.RNTupleReader.Open(obj)
        elif isinstance(obj, ROOT.TTree):
            self.obj = obj
            # in case it is used somewhere
            self.tree = self.obj

    def _dump(self, obj, itr_entries, leaves=None, retvecs=False, sortleaves=True):
        ROOT = import_root()
        try:
            RNTupleReader = ROOT.RNTupleReader
        except AttributeError:
            RNTupleReader = ROOT.Experimental.RNTupleReader
        if isinstance(obj, ROOT.TTree):
            yield from self._tree_dump(obj, itr_entries, leaves, retvecs, sortleaves)
        elif isinstance(obj, RNTupleReader):
            yield from self._reader_dump(obj, itr_entries, leaves, retvecs, sortleaves)
        else:
            raise NotImplementedError("'_dump' not implemented for object of class=%r" %
                                      (obj.__class__.__name__,))

    def dump(self, tree_name, itr_entries, leaves=None, retvecs=False, sortleaves=True):
        if (tree_name is None and getattr(self, "obj_name", None) is None or
            tree_name is not None and getattr(self, "obj_name", None) != tree_name):
                self.__init_obj(tree_name)
        yield from self._dump(self.obj, itr_entries, leaves, retvecs, sortleaves)

    def _tree_dump(self, tree, itr_entries, leaves=None, retvecs=False, sortleaves=True):

        ROOT = import_root()
        import AthenaPython.PyAthena as PyAthena
        _pythonize = PyAthena.RootUtils.PyROOTInspector.pyroot_inspect2

        tree_name = self.obj_name
        nentries = tree.GetEntries()
        if leaves is not None:
            leaves = [str(b).rstrip('\0') for b in leaves]
            leaves.sort()
        else:
            leaves = sorted([b.GetName().rstrip('\0') for b in tree.GetListOfBranches()])
        
        # handle itr_entries
        if isinstance(itr_entries, str):
            if ':' in itr_entries:
                def toint(s):
                    if s == '':
                        return None
                    try:
                        return int(s)
                    except ValueError:
                        return s
                from itertools import islice
                itr_entries = islice(range(nentries),
                                     *map(toint, itr_entries.split(':')))
            elif ('range' in itr_entries or
                  ',' in itr_entries):
                itr_entries = eval(itr_entries)
            else:
                try:
                    _n = int(itr_entries)
                    itr_entries = range(_n)
                except ValueError:
                    print ("** err ** invalid 'itr_entries' argument. will iterate over all entries.")
                    itr_entries = range(nentries)
        elif isinstance(itr_entries, list):
            itr_entries = itr_entries
        else:
            itr_entries = range(itr_entries)
                
        list_ = list
        map_ = map
        str_ = str
        isinstance_ = isinstance

        for ientry in itr_entries:
            hdr = ":: entry [%05i]..." % (ientry,)
            #print (hdr)
            #print (hdr, file=self.fout)
            err = tree.LoadTree(ientry)
            if err < 0:
                print ("**err** loading tree for entry",ientry)
                self.allgood = False
                break

            nbytes = tree.GetEntry(ientry)
            if nbytes <= 0:
                print ("**err** reading entry [%s] of tree [%s]" % (ientry, tree_name))
                hdr = ":: entry [%05i]... [ERR]" % (ientry,)
                print (hdr)
                self.allgood = False
                continue

            for br_name in leaves:
                hdr = "::  branch [%s]..." % (br_name,)
                #print (hdr)
                #tree.GetBranch(br_name).GetEntry(ientry)
                _vals = list()

                br = tree.GetBranch (br_name)
                if br.GetClassName() != '':
                    # Make sure dictionaries are completely loaded before
                    # trying to fetch it from ROOT.  Otherwise we can run
                    # into cling parse failures due to it synthesizing
                    # incorrect forward declarations.
                    # See ATEAM-1000.
                    getattr (ROOT, br.GetClassName())
                    val = getattr(tree, br_name)
                    _vals += [ ([br_name], val) ]
                else:
                    for l in br.GetListOfLeaves():
                        if (br.GetNleaves() == 1 and (br_name == l.GetName() or
                                                      br_name.endswith('.' + l.GetName()))):
                            _vals += [ ([br_name], _getLeaf (l)) ]
                        else:
                            _vals += [ ([br_name, l.GetName()], _getLeaf (l)) ]
                for py_name, val in _vals:
                    if val is None: continue
                    try:
                        vals = _pythonize(val, py_name, True, retvecs)
                    except Exception as err:
                        print ("**err** for branch [%s] val=%s (type=%s)" % (
                            br_name, val, type(val),
                            ))
                        self.allgood = False
                        print (err)
                    if sortleaves:
                        viter = sorted(vals, key = lambda x: '.'.join(s for s in x[0] if isinstance_(s, str_)))
                    else:
                        viter = vals
                    for o in viter:
                        n = list_(map_(str_, o[0]))
                        v = o[1]
                        yield tree_name, ientry, n, v

                pass # loop over branch names
            pass # loop over entries
    pass # class RootFileDumper

    def _reader_dump(self, reader, itr_entries, leaves=None, retvecs=False, sortleaves=True):

        ROOT = import_root()
        import AthenaPython.PyAthena as PyAthena
        _pythonize = PyAthena.RootUtils.PyROOTInspector.pyroot_inspect2

        ntuple_name = self.obj_name
        nentries = reader.GetNEntries()
        from operator import methodcaller
        if leaves is not None:
            leaves = sorted(leaves)
        else:
            leaves = sorted(map(methodcaller('GetFieldName'), reader.GetDescriptor().GetTopLevelFields()))

        # handle itr_entries
        if isinstance(itr_entries, str):
            if ':' in itr_entries:
                def toint(s):
                    if s == '':
                        return None
                    try:
                        return int(s)
                    except ValueError:
                        return s
                from itertools import islice
                itr_entries = islice(range(nentries),
                                     *map(toint, itr_entries.split(':')))
            elif ('range' in itr_entries or
                  ',' in itr_entries):
                itr_entries = eval(itr_entries)
            else:
                try:
                    _n = int(itr_entries)
                    itr_entries = range(_n)
                except ValueError:
                    print ("** err ** invalid 'itr_entries' argument. will iterate over all entries.")
                    itr_entries = range(nentries)
        elif isinstance(itr_entries, list):
            itr_entries = itr_entries
        else:
            itr_entries = range(itr_entries)

        list_ = list
        map_ = map
        str_ = str
        isinstance_ = isinstance

        try:
            from ROOT import RException
        except ImportError:
            from ROOT.Experimental import RException

        try:
            entry = reader.CreateEntry()
        except AttributeError:
            entry = reader.GetModel().CreateEntry()

        for ientry in itr_entries:
            try:
                reader.LoadEntry(ientry, entry)
            except RException as err:
                from traceback import format_exception
                import sys
                print("Exception reading entry=%05i of ntuple %r\n%s" %
                      (ientry, ntuple_name, "".join(format_exception(err))), file=sys.stderr)
                self.allgood = False
                continue

            for br_name in leaves:
                py_name = [br_name]
                token = entry.GetToken(br_name)
                typeName = entry.GetTypeName(token)
                # Make sure dictionaries are completely loaded before
                # trying to fetch it from ROOT.  Otherwise we can run
                # into cling parse failures due to it synthesizing
                # incorrect forward declarations.
                # See ATEAM-1000.
                getattr(ROOT, typeName)
                val = entry[token]
                if val is not None:
                    try:
                        vals = _pythonize(val, py_name, True, retvecs)
                    except Exception as err:
                        print("**err** for branch [%s] val=%s (type=%s)" %
                              (br_name, val, type(val)))
                        self.allgood = False
                        print(err)
                    if sortleaves:
                        viter = sorted(vals, key = lambda x: '.'.join(s for s in x[0] if isinstance_(s, str_)))
                    else:
                        viter = vals
                    for o in viter:
                        n = list_(map_(str_, o[0]))
                        v = o[1]
                        yield ntuple_name, ientry, n, v


### test support --------------------------------------------------------------
def _test_main():
    root = import_root()  # noqa: F841
    def no_raise(msg, fct, *args, **kwds):
        caught = False
        err = None
        try:
            fct(*args, **kwds)
        except Exception as xerr:
            err = xerr
            caught = True
        assert not caught, "%s:\n%s\nERROR" % (msg, err,)

    no_raise("problem pythonizing TFile", fct=_pythonize_tfile)
    no_raise("problem compiling dummy one-liner",
             root_compile, "void foo1() { return ; }")
    no_raise("problem compiling dummy one-liner w/ kwds",
             fct=root_compile, src="void foo1a() { return ; }")
    import tempfile
    # PvG workaround for ROOT-7059
    dummy = tempfile.NamedTemporaryFile(prefix="foo_",suffix=".cxx")  # noqa: F841
    with tempfile.NamedTemporaryFile(prefix="foo_",suffix=".cxx") as tmp:
        tmp.write (b"void foo2() { return ; }\n")
        tmp.flush()
        no_raise("problem compiling a file",
                 fct=root_compile, fname=tmp.name)

    print ("OK")
    return True

if __name__ == "__main__":
    _test_main()
    
