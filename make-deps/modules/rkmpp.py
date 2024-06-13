# -*- coding: utf-8 -*-
import os, sys
from module_base import BuilderBase
import dep_utils as utils


class Builder(BuilderBase):
    def __init__(self, sysName, currentPath):
        BuilderBase.__init__(self, sysName, currentPath, "rkmpp")

    def url(self):
        return "https://github.com/rockchip-linux/mpp/archive/refs/tags/1.0.5.tar.gz"

    def targetPath(self):
        return "include/rockchip/rk_mpi.h"

    def exports(self):
        ret = BuilderBase.exports(self)
        ret["rkmpp_ROOT"] = self.installBasePath()
        ret["rkmpp_INCLUDE_DIRS"] = self.installBasePath() + "/include"
        # ret["yuv_STATIC_LIBS"] = ' debug ' + self.installBasePath() + "/lib/yuvd${CMAKE_STATIC_LIBRARY_SUFFIX}" + \
        #                          ' optimized ' + self.installBasePath() + "/lib/yuv${CMAKE_STATIC_LIBRARY_SUFFIX}"
        # if (self.sysName == "linux") or (self.sysName == "arm"):
        #     ret["yuv_STATIC_LIBS"] = ' debug ' + self.installBasePath() + "/lib/libyuv${CMAKE_STATIC_LIBRARY_SUFFIX}" + \
        #                          ' optimized ' + self.installBasePath() + "/lib/libyuv${CMAKE_STATIC_LIBRARY_SUFFIX}"
        return ret

    def options(self):
        opts = BuilderBase.options(self)

        return opts

    def build(self, generator, options):
        super().build(generator, options)
        # sourcePath = self.srcPath()
        # installPath = self.installBasePath()
        files = os.listdir(self.srcPath() + '/osal/inc/')

        for file in files:
            absFile = self.srcPath() + '/osal/inc/' + file
            if os.path.isfile(absFile):
                utils.copy(absFile, self.installBasePath() + '/include/rockchip/')
