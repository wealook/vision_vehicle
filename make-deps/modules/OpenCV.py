# -*- coding: utf-8 -*-
import os, sys, shutil
from module_base import BuilderBase


class Builder(BuilderBase):
    def __init__(self, sysName, currentPath):
        BuilderBase.__init__(self, sysName, currentPath, "OpenCV")

    def url(self):
        return "https://github.com/opencv/opencv/archive/refs/tags/4.5.5.tar.gz"

    def options(self):
        opts = BuilderBase.options(self)
        opts["WITH_FFMPEG"] = False
        opts["WITH_TIFF"] = False
        opts["BUILD_SHARED_LIBS"] = False
        opts["BUILD_EXAMPLES"] = False
        opts["BUILD_TESTS"] = False
        opts["BUILD_PERF_TESTS"] = False
        opts["BUILD_opencv_python2"] = False
        opts["BUILD_opencv_python3"] = False
        opts["OPENCV_GENERATE_PKGCONFIG"] = False
        opts["BUILD_OPENEXR"] = True
        opts["OPENCV_IO_FORCE_OPENEXR"] = True

        if self.sysName == "android":
            opts["BUILD_JAVA"] = False
            opts["BUILD_ANDROID_EXAMPLES"] = False
            opts["ANDROID_NATIVE_API_LEVE"] = 24
            opts["ANDROID_PLATFORM"] = "android-24"
            opts["WITH_OPENCL"] = True
        elif self.sysName == "win64" or self.sysName == "win32":
            opts["BUILD_WITH_STATIC_CRT"] = False
        elif self.sysName == "arm":
            opts["BUILD_opencv_dnn"] = False
            opts["WITH_PROTOBUF"] = True
            opts["WITH_1394"] = False
            opts["BUILD_PROTOBUF"] = False
            opts["OPENCV_GENERATE_PKGCONFIG"] = True
            opts["WITH_GSTREAMER"] = False
            opts["WITH_QUIRC"] = False
            opts["BUILD_opencv_apps"] = False

        elif self.sysName == "linux":
            opts["BUILD_opencv_dnn"] = False
            opts["WITH_PROTOBUF"] = True
            opts["BUILD_PROTOBUF"] = False
            opts["WITH_QUIRC"] = False

        return opts

    def copyCPUConfig(self):
        srcPath = os.path.join(self.buildPath(), "cv_cpu_config.h")
        dstPath = dstPath = os.path.join(self.installBasePath(), "include/opencv4/opencv2/core/cv_cpu_config.h")
        if (self.sysName == "android"):
            dstPath = os.path.join(self.installBasePath(), "sdk/native/jni/include/opencv2/cv_cpu_config.h")
        elif (self.sysName == "win64" or self.sysName == "win32"):
            dstPath = dstPath = os.path.join(self.installBasePath(), "include/opencv2/core/cv_cpu_config.h")

        if (not os.path.exists(dstPath)):
            shutil.copyfile(srcPath, dstPath)

        srcPath = os.path.join(self.buildPath(), "cvconfig.h")
        dstPath = os.path.join(self.installBasePath(), "include/opencv4/opencv2/core/cvconfig.h")
        if (self.sysName == "android"):
            dstPath = os.path.join(self.installBasePath(), "sdk/native/jni/include/opencv2/cvconfig.h")
        elif (self.sysName == "win64" or self.sysName == "win32"):
            dstPath = dstPath = os.path.join(self.installBasePath(), "include/opencv2/core/cvconfig.h")

        if (not os.path.exists(dstPath)):
            shutil.copyfile(srcPath, dstPath)

    def exports(self):
        self.copyCPUConfig()
        ret = {}
        moduleDirName = "{0}_DIR".format(self.moduleName)
        ret[moduleDirName] = self.installedPath()
        ret["OpenCV_STATIC"] = "ON"
        return ret

    def installed(self):
        needNotUpdate = not self.checkUpdate()
        if (self.sysName == "win64" or self.sysName == "win32"):
            return needNotUpdate and os.path.exists(os.path.join(self.installedPath(), "OpenCVConfig.cmake"))
        return needNotUpdate and os.path.exists(self.installedPath())

    def targetPath(self):
        if (self.sysName == "android"):
            return "sdk/native/jni"
        elif (self.sysName == "osx"):
            return "lib/cmake/opencv4"
        elif (self.sysName == "win64" or self.sysName == "win32"):
            return ""
        elif (self.sysName == "arm"):
            return "lib/cmake/opencv4"

        return "lib/cmake/opencv4"

    def dependents(self):
        return []

    def copyPcFile(self):
        installPath = os.path.join(self.pcInstallPath(), "opencv.pc")
        pcFile = "opencv4.pc"
        pcFilePath = os.path.join(self.installBasePath(), "lib", "pkgconfig", pcFile)
        if (os.path.exists(pcFilePath)):
            shutil.copy(pcFilePath, installPath)
        return

    def version(self):
        return 45
