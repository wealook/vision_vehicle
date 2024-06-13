# -*- coding: utf-8 -*-
import os, sys
from module_base import BuilderBase


class Builder(BuilderBase):
    def __init__(self, sysName, currentPath):
        BuilderBase.__init__(self, sysName, currentPath, "jsoncpp")

    def url(self):
        return "https://github.com/wealook/make-deps-pkg/releases/download/JsonCpp1.9.5/jsoncpp-1.9.5.tar.gz"

    def options(self):
        opts = BuilderBase.options(self)

        opts['BUILD_SHARED_LIBS'] = False;
        return opts

    def supported(self):
        return self.sysName in ["win64", "win32", "osx", "android", "arm", "linux"]

    def exports(self):
        ret = {}
        ret["jsoncpp_ROOT"] = self.installBasePath()
        # ret["jsoncpp_INCLUDE_DIRS"] = self.installBasePath() + "/include"
        ret["jsoncpp_INCLUDE_DIR"] = self.installBasePath() + "/include"
        ret["jsoncpp_LIBRARY_DIR"] = self.installBasePath() + "/lib"
        ret["jsoncpp_DIR"] = self.installBasePath() + "/lib/cmake/jsoncpp"
        ret["jsoncpp_STATIC_LIBS"] = ' debug ' + self.installBasePath() + "/lib/jsoncppd${CMAKE_STATIC_LIBRARY_SUFFIX}" + \
                                     ' optimized ' + self.installBasePath() + "/lib/jsoncpp${CMAKE_STATIC_LIBRARY_SUFFIX}"
        if (self.sysName == "linux" or self.sysName == "arm"):
            ret["jsoncpp_STATIC_LIBS"] = ' debug ' + self.installBasePath() + "/lib/libjsoncpp${CMAKE_STATIC_LIBRARY_SUFFIX}" + \
                                         ' optimized ' + self.installBasePath() + "/lib/libjsoncpp${CMAKE_STATIC_LIBRARY_SUFFIX}"

        return ret
