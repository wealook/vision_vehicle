# -*- coding: utf-8 -*-
import os, sys, shutil
from module_base import BuilderBase


class Builder(BuilderBase):
    def __init__(self, sysName, currentPath):
        BuilderBase.__init__(self, sysName, currentPath, "yaml-cpp")

    def url(self):
        return "https://github.com/wealook/make-deps-pkg/releases/download/yaml-cpp-0.7.0/yaml-cpp-0.7.0.tar.gz"

    def options(self):
        opts = BuilderBase.options(self)
        opts["YAML_CPP_BUILD_TESTS"] = False
        opts["YAML_CPP_BUILD_TOOLS"] = False
        opts["YAML_CPP_INSTALL"] = True
        opts["YAML_CPP_BUILD_CONTRIB"] = False
        opts["MSVC_SHARED_RT"] = False
        opts["BUILD_SHARED_LIBS"] = False
        return opts

    def exports(self):
        ret = {}
        moduleDirName = "{0}_DIR".format(self.moduleName)
        ret[moduleDirName] = self.installedPath()
        ret["YAML_CPP_ROOT_DIR"] = self.installBasePath()
        ret["YAML_CPP_ROOT_DIR_INCLUDE"] = self.installBasePath() + '/include'
        ret["YAML_CPP_ROOT_DIR_LIBS"] = self.installBasePath() + '/lib'
        return ret

    def installed(self):
        needNotUpdate = not self.checkUpdate()
        if (self.sysName == "win64" or self.sysName == "win32"):
            return needNotUpdate and os.path.exists(os.path.join(self.installedPath(), "yaml-cpp-config.cmake"))
        return needNotUpdate and os.path.exists(self.installedPath())

    def targetPath(self):
        return "share/cmake/yaml-cpp/"

    def dependents(self):
        return []

    def copyPcFile(self):
        pass
        installPath = os.path.join(self.pcInstallPath(), "yaml-cpp.pc.pc")
        pcFile = "opencv4.pc"
        pcFilePath = os.path.join(self.installBasePath(), "lib", "pkgconfig", pcFile)
        if (os.path.exists(pcFilePath)):
            shutil.copy(pcFilePath, installPath)
        return

    def version(self):
        return 0
