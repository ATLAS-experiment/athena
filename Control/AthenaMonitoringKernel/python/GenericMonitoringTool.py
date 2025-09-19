#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging
import json

log = logging.getLogger(__name__)


class GenericMonitoringTool(CompFactory.GenericMonitoringTool):
    '''GenericMonitoringTool configurable class'''

    __slots__ = ('_configFlags', '_convention', '_defaultDuration')

    def __init__(self, flags, name='GenericMonitoringTool', **kwargs):
        self._configFlags = flags
        self._convention = ''
        self._defaultDuration = kwargs.pop('defaultDuration', None)
        super().__init__(name, **kwargs)

    @property
    def convention(self):
        return self._convention

    @convention.setter
    def convention(self, value):
        self._convention = value

    @property
    def defaultDuration(self):
        return self._defaultDuration

    @defaultDuration.setter
    def defaultDuration(self, value):
        self._defaultDuration = value

    def _coreDefine(self, deffunc, *args, **kwargs):
        if 'convention' in kwargs:
            # only if someone really knows what they're doing
            pass
        else:
            duration = kwargs.pop('duration', self.defaultDuration)
            if duration is not None:
                kwargs['convention'] = self.convention + ':' + duration
        # if an overall path for tool is specified, can leave path argument empty
        if getattr(self, 'HistPath', '') != '':
            kwargs.setdefault('path', '')
        toadd = deffunc(self._configFlags, *args, **kwargs)
        if toadd:
            self.Histograms.append(toadd)

    def defineHistogram(self, *args, **kwargs):
        self._coreDefine(defineHistogram, *args, **kwargs)

    def defineTree(self, *args, **kwargs):
        self._coreDefine(defineTree, *args, **kwargs)


class GenericMonitoringArray:
    '''Array of configurables of GenericMonitoringTool objects'''
    def __init__(self, flags, name, dimensions, **kwargs):
        self.Tools = {}
        self.Postfixes, self.Accessors = GenericMonitoringArray._postfixes(dimensions)
        for postfix in self.Postfixes:
            self.Tools[postfix] = GenericMonitoringTool(flags, name+postfix,**kwargs)

    def __getitem__(self,index):
        '''Forward operator[] on class to the list of tools'''
        return self.toolList()[index]

    def toolList(self):
        return list(self.Tools.values())

    def broadcast(self, member, value):
        '''Allows one to set attributes of every tool simultaneously

        Arguments:
        member -- string which contains the name of the attribute to be set
        value -- value of the attribute to be set
        '''
        for tool in self.toolList():
            setattr(tool,member,value)

    def defineHistogram(self, varname, title=None, path=None, pattern=None, **kwargs):
        '''Propogate defineHistogram to each tool, adding a unique tag.
        
        Arguments:
        pattern -- if specified, list of n-tuples of indices for plots to create
        '''
        unAliased = varname.split(';')[0]
        _, aliasBase = _alias(varname)
        if aliasBase is None or aliasBase.strip() == '':
            raise ValueError(f'Unable to define histogram using definition "{varname}" since we cannot determine its name')
        if pattern is not None:
            try:
                iter(pattern)
            except TypeError:
                raise ValueError('Argument to GenericMonitoringArray.defineHistogram must be iterable')
            if not isinstance(pattern, list):
                pattern = list(pattern)
            if len(pattern) == 0: # nothing to do
                return
            if isinstance(pattern[0], (str, int)):
                # assume we have list of strings or ints; convert to list of 1-element tuples
                pattern = [(_2,) for _2 in pattern]
        for postfix, tool in self.Tools.items():
            try:
                accessors = tuple(self.Accessors[postfix])
                if pattern is not None:
                    if accessors not in pattern:
                        continue
                # two options for alias formatting,
                #   a) default convention: (var_0, var_1, etc.)
                #   b) custom formatting: 'alias{0}custom'.format(*(0, 1))
                aliasBaseFormatted = aliasBase.format(*accessors)
                if aliasBaseFormatted==aliasBase:
                    # if format call did not do anything, use default
                    aliased = unAliased+';'+aliasBase+postfix
                else:
                    # if format call changed the alias, use custom
                    aliased = unAliased+';'+aliasBaseFormatted
                if title is not None:
                    kwargs['title'] = title.format(*accessors)
                if path is not None:
                    kwargs['path'] = path.format(*accessors)
            except IndexError as e:
                log.error('In title or path template of histogram {0}, too many positional '\
                    'arguments were requested. Title and path templates were "{1}" and "{2}", '\
                    'while only {3} fillers were given: {4}.'.format(aliasBase, title,\
                    path, len(accessors), accessors))
                raise e

            tool.defineHistogram(aliased, **kwargs)

    @staticmethod
    def _postfixes(dimensions, previous=''):
        '''Generates a list of subscripts to add to the name of each tool

        Arguments:
        dimensions -- List containing the lengths of each side of the array off tools
        previous -- Strings appended from the other dimensions of the array
        '''
        import collections
        assert isinstance(dimensions,list) and len(dimensions)>0, \
            'GenericMonitoringArray must have list of dimensions.'
        try:
            if dimensions==[1]:
                return [''], {'': ['']}
        except AttributeError: 
            #Evidently not [1]
            pass
        postList = []
        accessorDict = collections.OrderedDict()
        first = dimensions[0]
        if isinstance(first,list):
            iterable = first
        elif isinstance(first,int):
            iterable = range(first)
        else:
            #Assume GaudiConfig2.semantics._ListHelper
            iterable = list(first)
        for i in iterable:
            if len(dimensions)==1:
                postList.append(previous+'_'+str(i))
                accessorDict[previous+'_'+str(i)]=[str(i)]
            else:
                postfixes, accessors = GenericMonitoringArray._postfixes(dimensions[1:],previous+'_'+str(i))
                postList.extend(postfixes)
                for acckey, accval in accessors.items():
                    accessorDict[acckey] = [str(i)] + accval
        return postList, accessorDict


## Check if name is an allowed histogram/branch name
#
#  Certain characers are best avoided in ROOT histogram names as it makes interactive
#  use awkward. Also there are additional constraints from OH and MDA archiving for
#  online running (ATR-15173).
#
#  @param flags configuration flags
#  @param name string to check
#  @return set of forbidden characters found
def _invalidName(flags, name):
    blocklist = '/\\'
    if flags.Common.isOnline:
        blocklist += '=,:.()'
    return set(name).intersection(blocklist)


## Generate an alias for a set of variables
#
#  A helper function is useful for this operation, since it is used both by the module
#  function defineHistogram, as well as by the GenericMonitoringArray defineHistogram
#  member function.
#  @param varname unparsed
#  @return varList, alias
def _alias(varname):
    variableAliasSplit = varname.split(';')
    varList = [v.strip() for v in variableAliasSplit[0].split(',')]
    if len(variableAliasSplit)==1:
        return varList, '_vs_'.join(reversed(varList))
    elif len(variableAliasSplit)==2:
        return varList, variableAliasSplit[1]
    else:
        message = 'Invalid variable or alias for {}. Histogram(s) not defined.'
        log.warning(message.format(varname))
        return None, None


## Validate user inputs for "opt" argument of defineHistogram
#
#  Check that the user-provided option for a specific "opt" argument exists in the
#  default dictionary, and that it has the expected type.
#  @param user the option dictionary provided by the user
#  @param default the default dictionary of options
def _validateOptions(user, default):
    for key, userVal in user.items():
        # (1) Check that the requested key exists
        assert key in default,\
            f'Unknown option {key} provided. Choices are [{", ".join(default)}].'
        # (2) Check that the provided type is correct
        userType = type(userVal)
        defaultVal = default[key]
        defaultType = type(defaultVal)
        if isinstance(userVal, bool) or isinstance(defaultVal, bool):
            assert isinstance(userVal, bool) and isinstance(defaultVal, bool),\
                f'{key} provided {userType}, expected bool.'
        else:
            assert isinstance(defaultVal, userType),\
                f'{key} provided {userType}, expected {defaultType}'


## Generate dictionary entries for opt strings
#  @param opt string or dictionary specifying type
#  @return dictionary full of options
def _options(opt):
    # Set the default dictionary of options
    settings = {
        'Sumw2': False,                 # store sum of squares of weights
        'kLBNHistoryDepth': 0,          # length of lumiblock history
        'kAddBinsDynamically': False,   # add new bins if fill is outside axis range
        'kRebinAxes': False,            # increase axis range without adding new bins
        'kCanRebin': False,             # allow all axes to be rebinned
        'kVec': False,                  # add content to each bin from each element of a vector
        'kVecUO': False,                # same as above, but use 0th(last) element for underflow(overflow)
        'kCumulative': False,           # fill bin of monitored object's value, and every bin below it
        'kLive': 0,                     # plot only the last N lumiblocks on y_vs_LB plots
        'kAlwaysCreate': False          # create the histogram, even if it is empty
    }
    if opt is None:
        # If no options are provided, skip any further checks.
        pass
    elif isinstance(opt, dict):
        # If the user provides a partial dictionary, update the default with user's.
        _validateOptions(opt, settings) # check validity of user's options
        settings.update(opt) # update the default dictionary
    elif isinstance(opt, str) and len(opt)>0:
        # If the user provides a comma- or space-separated string of options.
        settings = _options (opt.replace(',',' ').split())

    elif isinstance(opt,str):
        # empty string case 
        pass
    elif isinstance(opt, list):
        # process each item in list
        unknown = []
        for o in opt:
            kv = o.split('=', maxsplit=1)
            key = kv[0]
            if len(kv)==2:
                value = False if kv[1]=='False' else int(kv[1])  # only bool and int supported
            else:
                value = True
            if key in settings:
                assert(type(settings[key])==type(value))  # ensure same type as in defaults
                settings[key] = value
            else:
                unknown.append(key)

        assert len(unknown)==0, f'Unknown option(s) provided: {", ".join(unknown)}.'
    else:
        raise ValueError("Unknown opt type")
    return settings


## Generate histogram definition string for the `GenericMonitoringTool.Histograms` property
#
#  For full details see the GenericMonitoringTool documentation.
#  @param flags    configuration flags object
#  @param varname  one (1D) or two (2D) variable names separated by comma
#                  optionally give histogram name by appending ";" plus the name
#  @param type     histogram type
#  @param path     top-level histogram directory (e.g. EXPERT, SHIFT, etc.)
#  @param title    Histogram title and optional axis title (same syntax as in TH constructor)
#  @param weight   Name of the variable containing the fill weight
#  @param cutmask  Name of the boolean-castable variable that determines if the plot is filled
#  @param opt      String or dictionary of histogram options (see _options())
#  @param treedef  Internal use only. Use defineTree() method.
#  @param xlabels  List of x bin labels.
#  @param ylabels  List of y bin labels.
#  @param zlabels  List of x bin labels.
#  @param merge    Merge method to use for object, if not default. Possible algorithms for offline DQM
#                  are given in https://twiki.cern.ch/twiki/bin/view/Atlas/DQMergeAlgs
def defineHistogram(flags, varname, type='TH1F', path=None,
                    title=None, weight=None,
                    xbins=100, xmin=0, xmax=1, xlabels=None,
                    ybins=None, ymin=None, ymax=None, ylabels=None,
                    zmin=None, zmax=None, zlabels=None,
                    opt=None, convention=None, cutmask=None,
                    treedef=None, merge=None):

    # All of these fields default to an empty string
    stringSettingsKeys = ['xvar', 'yvar', 'zvar', 'type', 'path', 'title', 'weight',
                          'cutMask', 'convention', 'alias', 'treeDef', 'merge']
    # All of these fileds default to 0
    numberSettingsKeys = ['xbins', 'xmin', 'xmax', 'ybins', 'ymin', 'ymax', 'zbins',
                          'zmin', 'zmax']
    # All of these fields default to an empty array
    arraySettingsKeys = ['allvars', 'xlabels', 'xarray', 'ylabels', 'yarray', 'zlabels']
    # Initialize a dictionary with all possible fields
    settings = dict((key, '') for key in stringSettingsKeys)
    settings.update(dict((key, 0.0) for key in numberSettingsKeys))
    settings.update(dict((key, []) for key in arraySettingsKeys))

    # Alias
    varList, alias = _alias(varname)
    if alias is None or alias.strip() == '':
        log.warning(f'Unable to define histogram using definition "{varname}" since we cannot determine its name.')
        return ''

    invalid = _invalidName(flags, alias)
    if invalid:
        log.warning('%s is not a valid histogram name. Illegal characters: %s',
                    alias, ' '.join(invalid))
        return ''

    settings['alias'] = alias

    # Variable names
    nVars = len(varList)
    if nVars>0:
        settings['xvar'] = varList[0]
    if nVars>1:
        settings['yvar'] = varList[1]
    if nVars>2:
        settings['zvar'] = varList[2]
    settings['allvars'] = varList

    # Type
    if flags.Common.isOnline and type in ['TTree']:
        log.warning('Object %s of type %s is not supported for online running and '
                    'will not be added.', varname, type)
        return ''

    # Allowed histogram dimensions
    if type.startswith('TH1'):
        dims = {1}
    elif type.startswith('TH2'):
        dims = {2}
    elif type=='TProfile':
        dims = {2}
    elif type=='TProfile2D':
        dims = {3}
    elif type=='TEfficiency':
        dims = {2, 3}

    assert type=='TTree' or nVars in dims,\
        f'Number of monitored variables {varList} for "{path}/{alias}" '\
        f'does not match histogram dimension for {type}'

    settings['type'] = type

    # Path
    if path is None:
        path = ''
    settings['path'] = path

    # Title
    if title is None:
        title = varname
    settings['title'] = title

    # Weight
    if weight is not None:
        settings['weight'] = weight

    # Cutmask
    if cutmask is not None:
        settings['cutMask'] = cutmask

    # Output path naming convention
    if convention is not None:
        settings['convention'] = convention

    # Bin counts and ranges
    # Possible types allowed for bin counts
    binTypes = (int, list, tuple)

    # X axis count and range
    assert isinstance(xbins, binTypes),'xbins argument must be int, list, or tuple'
    if isinstance(xbins, int): # equal x bin widths
        settings['xbins'], settings['xarray'] = xbins, []
    else: # x bin edges are set explicitly
        settings['xbins'], settings['xarray'] = len(xbins)-1, xbins
    settings['xmin'] = xmin
    settings['xmax'] = xmax

    # Y axis count and range
    if ybins is not None:
        assert isinstance(ybins, binTypes),'ybins argument must be int, list, or tuple'
        if isinstance(ybins, int): # equal y bin widths
            settings['ybins'], settings['yarray'] = ybins, []
        else: # y bin edges are set explicitly
            settings['ybins'], settings['yarray'] = len(ybins)-1, ybins
    if ymin is not None:
        settings['ymin'] = ymin
    if ymax is not None:
        settings['ymax'] = ymax

    # Z axis count and range
    if zmin is not None:
        settings['zmin'] = zmin
    if zmax is not None:
        settings['zmax'] = zmax

    # Then, parse the [xyz]label arguments
    if xlabels is not None and len(xlabels)>0:
        assert isinstance(xlabels, (list, tuple)),'xlabels must be list or tuple'
        settings['xbins'] = len(xlabels)
        settings['xlabels'] = xlabels
    if ylabels is not None and len(ylabels)>0:
        assert isinstance(ylabels, (list, tuple)),'ylabels must be list or tuple'
        settings['ybins'] = len(ylabels)
        settings['ylabels'] = ylabels
        # if user did not specify ymin and ymax, set it here, as cannot have ymin=ymax=0
        if ymin is None: settings["ymin"] = 0
        if ymax is None: settings["ymax"] = settings["ymin"]+1
    if zlabels is not None and len(zlabels)>0:
        assert isinstance(zlabels, (list, tuple)),'zlabels must be list or tuple'
        settings['zlabels'] = zlabels

    # Tree branches
    if treedef is not None:
        assert type=='TTree','cannot define tree branches for a non-TTree object'
        settings['treeDef'] = treedef

    # Add all other options
    settings.update(_options(opt))

    # some things need merging
    if ((settings['kAddBinsDynamically'] or settings['kRebinAxes'] or settings['kCanRebin'])
        and (not flags.Common.isOnline and 'OFFLINE' in settings['convention'])):
        if merge is None:
            log.warning(f'Merge method for {alias} is not specified but needs to be "merge" due to histogram definition; overriding for your convenience')
            merge = 'merge'

    # merge method
    if merge is not None:
        assert type not in ['TEfficiency', 'TTree', 'TGraph'],'only default merge defined for non-histogram objects'
        settings['merge'] = merge

    # LB histograms always need to be published online (ADHI-4947)
    if settings['kLBNHistoryDepth']>0 and flags.Common.isOnline:
        settings['kAlwaysCreate'] = True
        log.debug('Setting kAlwaysCreate for lumiblock histogram "%s"', varname)

    # Check that kLBNHistoryDepth and kLive are both non-negative
    assert settings['kLBNHistoryDepth']>=0, f'Histogram "{alias}" has invalid kLBNHistoryDepth.'
    assert settings['kLive']>=0, f'Histogram "{alias}" has invalid kLive.'
    # kLBNHistoryDepth and kLive options are mutually exclusive. User may not specify both.
    assert settings['kLBNHistoryDepth']==0 or settings['kLive']==0,\
    f'Cannot use both kLBNHistoryDepth and kLive for histogram {alias}.'
    # kLive histograms are only available for Online monitoring.
    assert settings['kLive']==0 or flags.Common.isOnline,\
    f'Cannot use kLive with offline histogram {alias}.'

    return json.dumps(settings)


## Generate tree definition string for the `GenericMonitoringTool.Histograms` property. Convenience tool for 
#
#  For full details see the GenericMonitoringTool documentation.
#  @param flags    configuration flags object
#  @param varname  at least one variable name (more than one should be separated by comma);
#                  optionally give the name of the tree by appending ";" plus the tree name
#  @param treedef  TTree branch definition string. Looks like the standard TTree definition
#                  (see https://root.cern.ch/doc/master/classTTree.html#addcolumnoffundamentaltypes).
#                  In fact if only scalars are given, it is exactly the same as you would use to
#                  define the TTree directly: "varA/F:varB/I:...".  Vectors can be defined by giving
#                  "vector<int>", etc., instead of "I".
#  @param path     top-level histogram directory (e.g. EXPERT, SHIFT, etc.)
#  @param title    Histogram title and optional axis title (same syntax as in TH constructor)
#  @param cutmask  Name of the boolean-castable variable that determines if the plot is filled
#  @param opt      TTree options (none currently)
#  @param convention Expert option for how the objects are placed in ROOT
def defineTree(flags, varname, treedef, path=None, title=None,
               opt='', convention=None,
               cutmask=None):
    return defineHistogram(flags, varname, type='TTree', path=path, title=title,
                           treedef=treedef, opt=opt, convention=convention,
                           cutmask=cutmask)
