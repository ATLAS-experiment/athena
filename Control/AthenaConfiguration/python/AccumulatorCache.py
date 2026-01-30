#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

from AthenaCommon.Logging import logging
_msg = logging.getLogger('AccumulatorCache')

import functools
import time
from abc import ABC, abstractmethod
from copy import deepcopy
from collections.abc import Hashable, Iterable
from collections import defaultdict
from dataclasses import dataclass

try:
    from GaudiKernel.DataHandle import DataHandle
except ImportError:
    class DataHandle: pass  # for analysis releases


class NotHashable(Exception):
    """Exception thrown when AccumulatorCache is applied to non-hashable function call"""
    def __init__(self, value):
        super().__init__ (self)
        self.value = value


class AccumulatorCachable(ABC):
    """Abstract base for classes needing custom AccumulatorCache behavior."""

    @abstractmethod
    def _cacheEvict(self):
        """This method is called by AccumulatorCache when an object is removed
        from the cache. Implement this for custom cleanup actions."""
        pass


class AccumulatorDecorator:
    """Class for use in function decorators, implements memoization.

    Instances are callable objects that use the
    hash value calculated from positional and keyword arguments
    to implement memoization. Methods for suspending and 
    resuming memoization are provided.
    """

    _memoize = True

    VERIFY_NOTHING = 0
    VERIFY_HASH = 1

    @dataclass
    class CacheStats:
        hits  : int = 0
        misses: int = 0
        t_hits: float = 0
        t_misses: float = 0

    _stats = defaultdict(CacheStats)

    def __init__(self, func, size, verify, deepCopy):
        """See AccumulatorCache decorator for documentation of arguments."""

        functools.update_wrapper(self , func)
        self._maxSize = size
        self._func = func
        self._cache = {}
        self._resultCache = {}
        self._verify = verify
        self._deepcopy = deepCopy

        if self._verify not in [self.VERIFY_NOTHING, self.VERIFY_HASH]:
            raise RuntimeError(f"Invalid value for verify ({verify}) in AccumulatorCache for {func}")

    def getInfo(self):
        """Return a dictionary with information about the cache size and cache usage"""
        return {"cache_size" : len(self._cache),
                "misses" : self._stats[self].misses,
                "hits" : self._stats[self].hits,
                "function" : self._func,
                "result_cache_size" : len(self._resultCache)}

    @classmethod
    def printStats(cls):
        """Print cache statistics"""
        header = "%-70s |    Hits (time) |  Misses (time) |" % "AccumulatorCache"
        print("-"*len(header))
        print(header)
        print("-"*len(header))
        # Print sorted by hit+miss time:
        for func, stats in sorted(cls._stats.items(), key=lambda s:s[1].t_hits+s[1].t_misses, reverse=True):
            name = f"{func.__module__}.{func.__name__}"
            if len(name) > 70:
                name = '...' + name[-67:]
            print(f"{name:70} | {stats.hits:6} ({stats.t_hits:4.1f}s) | "
                  f"{stats.misses:6} ({stats.t_misses:4.1f}s) |")
        print("-"*len(header))

    @classmethod
    def suspendCaching(cls):
        """Suspend memoization for all instances of AccumulatorDecorator."""
        cls._memoize = False

    @classmethod
    def resumeCaching(cls):
        """Resume memoization for all instances of AccumulatorDecorator."""
        cls._memoize = True

    @classmethod
    def clearCache(cls):
        """Clear all accumulator caches"""
        for decor in cls._stats:
            decor._evictAll()
            decor._cache.clear()
            decor._resultCache.clear()

        cls._stats.clear()

    def _getHash(x):
        if hasattr(x, "athHash"):
            return x.athHash()
        elif isinstance(x, Hashable):
            return hash(x)
        elif isinstance(x, DataHandle):
            return hash(repr(x))
        raise NotHashable(x)

    def _evict(x):
        """Called when x is removed from the cache"""
        if isinstance(x, AccumulatorCachable):
            x._cacheEvict()
        elif isinstance(x, Iterable) and not isinstance(x, str):
            for el in x:
                AccumulatorDecorator._evict(el)

    def _evictAll(self):
        for v in self._cache.values():
            AccumulatorDecorator._evict(v)

    def __get__(self, obj, objtype):
        """Support instance methods."""
        return functools.partial(self.__call__, obj)

    def __call__(self, *args, **kwargs):
        cacheHit = None
        try:
            t0 = time.perf_counter()
            res, cacheHit = self._callImpl(*args, **kwargs)
            return res
        except NotHashable as e:
            _msg.warning(f"Argument value '{repr(e.value)}' in {self._func} is not hashable. "
                         "No caching is performed!")
            cacheHit = False
            return self._func(*args, **kwargs)  # perform regular function call
        finally:
            t1 = time.perf_counter()
            if cacheHit is True:
                self._stats[self].hits += 1
                self._stats[self].t_hits += (t1-t0)
            elif cacheHit is False:
                self._stats[self].misses += 1
                self._stats[self].t_misses += (t1-t0)

    def _callImpl(self, *args, **kwargs):
        """Implementation of __call__.

        Returns: (result, cacheHit)
        """

        # AccumulatorCache enabled?
        if not AccumulatorDecorator._memoize:
            return (self._func(*args , **kwargs), None)

        # frozen set makes the order of keyword arguments irrelevant
        hsh = hash( (tuple(AccumulatorDecorator._getHash(a) for a in args),
                     frozenset((hash(k), AccumulatorDecorator._getHash(v)) for k,v in kwargs.items())) )

        res = self._cache.get(hsh, None)
        if res is not None:
            cacheHit = None
            if AccumulatorDecorator.VERIFY_HASH == self._verify:
                resHsh = self._resultCache[hsh]
                chkHsh = AccumulatorDecorator._getHash(res)
                if chkHsh != resHsh:
                    _msg.debug("Hash of function result, cached using AccumulatorDecorator, changed.")
                    cacheHit = False
                    res = self._func(*args , **kwargs)
                    self._cache[hsh] = res
                    self._resultCache[hsh] = AccumulatorDecorator._getHash(res)
                else:
                    cacheHit = True
            else:
                cacheHit = True

            if self._deepcopy:
                return deepcopy(res), cacheHit
            else:
                # shallow copied CA still needs to undergo merging
                from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
                if isinstance(res, ComponentAccumulator):
                    res._wasMerged=False
                return res, cacheHit

        else:
            _msg.debug('Hash not found in AccumulatorCache for function %s' , self._func)
            if len(self._cache) >= self._maxSize:
                _msg.warning("Cache limit (%d) reached for %s.%s",
                             self._maxSize, self._func.__module__, self._func.__name__)
                oldest = self._cache.pop(next(iter(self._cache)))
                AccumulatorDecorator._evict(oldest)

            res = self._func(*args , **kwargs)

            if AccumulatorDecorator.VERIFY_HASH == self._verify:
                if len(self._resultCache) >= self._maxSize:
                    del self._resultCache[next(iter(self._resultCache))]
                self._resultCache[hsh] = AccumulatorDecorator._getHash(res)
                self._cache[hsh] = res
            else:
                self._cache[hsh] = res

            return (deepcopy(res) if self._deepcopy else res, False)

    def __del__(self):
        self._evictAll()


def AccumulatorCache(func = None, maxSize = 128,
                     verifyResult = AccumulatorDecorator.VERIFY_NOTHING, deepCopy = True):
    """Function decorator, implements memoization.

    Keyword arguments:
        maxSize: maximum size for the cache associated with the function (default 128)
        verifyResult:   takes two possible values
                        
                        AccumulatorDecorator.VERIFY_NOTHING -   default, the cached function result is returned with no verification
                        AccumulatorDecorator.VERIFY_HASH -      before returning the cached function value, the hash of the
                                                                result is checked to verify if this object was not modified
                                                                between function calls
        deepCopy:   if True (default) a deep copy of the function result will be stored in the cache.
    
    Returns:
        An instance of AccumulatorDecorator.
    """

    def wrapper_accumulator(func):
        return AccumulatorDecorator(func, maxSize, verifyResult, deepCopy)

    return wrapper_accumulator(func) if func else wrapper_accumulator
