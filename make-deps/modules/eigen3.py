# -*- coding: utf-8 -*-
import os, sys
from module_base import BuilderBase

class Builder(BuilderBase):
    def __init__(self, sysName, currentPath):
        BuilderBase.__init__(self, sysName, currentPath, "eigen3")

    def url(self):
        return "https://github.com/wealook/make-deps-pkg/releases/download/Eigen3.3.7/eigen-3.3.7.tar.gz"

    def supported(self):
        return self.sysName in ["win64", "win32", "osx", "android", "arm", "linux"]
        
    def options(self):
        opts = BuilderBase.options(self)
        opts["BUILD_TESTING"] = False
        
        return opts

    def targetPath(self):
        return "share/eigen3/cmake"

    def dependents(self):
        if self.sysName in ["win64", "win32", "osx"]:
            return ["FFTW3"]
        return []

    def exports(self):
        ret = {}
        moduleDirName = "{0}_DIR".format(self.moduleName)
        ret[moduleDirName] = self.installedPath()
        ret["Eigen3_DIR"] = self.installedPath()
        ret["EIGEN_INCLUDE_DIR"] = '/'.join([self.installPath(), "include", "eigen3"])
        os.environ["EIGEN_ROOT"] = '/'.join([self.installPath(), "include", "eigen3"])
        return ret
