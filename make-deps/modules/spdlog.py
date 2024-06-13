# -*- coding: utf-8 -*-
import os, sys
from module_base import BuilderBase

class Builder(BuilderBase):
    def __init__(self, sysName, currentPath):
        BuilderBase.__init__(self, sysName, currentPath, "spdlog")

    def url(self):
        return "https://github.com/gabime/spdlog/archive/refs/tags/v1.3.1.tar.gz"

    def exports(self):
        ret = BuilderBase.exports(self)
        ret["spd_INCLUDE_DIRS"] = self.installBasePath() + "/include"
        return ret

    def options(self):
        opts = BuilderBase.options(self)
        opts["SPDLOG_BUILD_EXAMPLES"] = False
        opts["SPDLOG_BUILD_BENCH"] = False
        opts["SPDLOG_BUILD_TESTS"] = False

        return opts
