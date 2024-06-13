# -*- coding: utf-8 -*-
from __future__ import (absolute_import, print_function)
from cmd_options import getCmdOptions
from module_loader import loadModules
import os

currentPath = os.path.realpath(os.path.dirname(__file__))
homePath = os.path.expanduser("~")
basePath = os.environ.get("WLP_MAKE_DEPS_WORKING_DIR") or os.path.join(homePath, "wlp_makedeps")

options = {}
exports = {}
getCmdOptions(options)
modules = loadModules(basePath, options["sysName"])
for module in modules:
    if (not options["modules"] or module.name() in options["modules"]):
        module.process(options, exports)
    module.checkExports(exports)
    module.copyPcFile()

installedPath = os.path.join(basePath, options["sysName"], "deps.cmake")
with open(installedPath, 'w') as f:
    for moduleKey in exports:
        moduleContent = exports[moduleKey]
        for key in moduleContent:
            itemStr = "set({0} {1})\n".format(key, moduleContent[key])
            f.write(itemStr)
        f.write("\n")
