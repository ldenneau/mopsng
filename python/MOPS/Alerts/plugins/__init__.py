"""
MOPS alert checker plugins

Each plugin is a python file or module exporting a single class.
"""

import functools
import imp
import os
import sys


import MOPS.Instance as Instance
from MOPS.Alerts.Support import fetchDerivedObject

from base import Rule


__all__ = ['rules']




def _ismodule(name):
    """
    Return True if name is either a python file (i.e. name ends in '.py') or is
    a directory with a '__init__.py' file inside. Return False in all other
    cases.
    """
    return(name.endswith('.py') or \
           (os.path.isdir(name) and '__init__.py' in os.listdir(name)))


def _importRules(modulePath):
    """
    Import all the Rule subclasses defined in modulePath.
    """
    if(modulePath.endswith('.py')):
        modulePath = modulePath[:-3]
    # <-- end if
    
    module = __import__(modulePath, globals(), locals())
    classes = []
    
    for attrName in dir(module):
        if(attrName.startswith('_') or
           attrName.endswith('.Rule') or
           attrName == 'Rule' or
           attrName == 'TrackletRule' or
           attrName == 'DerivedObjectRule' or
           attrName == 'UnlinkedFastMovers' or
           attrName == 'Impactors' or
           attrName == 'Hyperbolics'):
            continue
        # <-- end if
        attr = getattr(module, attrName)
        try:
            if(issubclass(attr, Rule)):
                classes.append(attr)
            # <-- edn if
        except:
            continue
    # <-- end for
    return(classes)



# Find all the plugins.
cwd = os.path.dirname(__file__)
sys.path.insert(0, cwd)

moduleNames = [f \
               for f in os.listdir(cwd) \
               if f != '__init__.py' and \
                  f != 'base.py' and \
                  _ismodule(os.path.join(cwd, f))]
# Import them all and extract the Rule subclasses.
rules = reduce(lambda a, b: a+b, map(_importRules, moduleNames))




