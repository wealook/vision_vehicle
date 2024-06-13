import os, sys, subprocess
import dep_utils as utils
import shutil


class BuilderBase(object):

    def __init__(self, sysName, basePath, moduleName):
        self.sysName = sysName
        self.moduleName = moduleName
        self.basePath = basePath

    def name(self):
        return self.moduleName

    def options(self):
        opts = {
            "CMAKE_DEBUG_POSTFIX": 'd',
            "CMAKE_POSITION_INDEPENDENT_CODE": "TRUE"
        }
        return opts

    def version(self):
        return 0

    def supported(self):
        return True

    def targetPath(self):
        return "lib/cmake/" + self.moduleName

    def installBasePath(self):
        insBasePath = os.path.join(self.basePath, self.sysName, self.moduleName)
        if (self.sysName == "win64" or self.sysName == "win32"):
            insBasePath = insBasePath.replace("\\", "/")
        return insBasePath

    def installedPath(self):
        return self.installBasePath() + "/" + self.targetPath()

    def checkUpdate(self):
        workingPath = os.path.join(self.basePath, "build", self.name())
        currentVersion = self.cachedVersion(workingPath)
        needUpdate = (currentVersion < self.version())
        # print("old version is {0}, new version is {1} {2}".format(currentVersion, self.version(), self.moduleName))
        return needUpdate

    def installed(self):
        workingPath = os.path.join(self.basePath, "build", self.name())

        return (not self.checkUpdate()) and os.path.exists(self.installedPath())

    def url(self):
        return ""

    def updateVersion(self, folder):
        filePath = os.path.join(folder, "version.txt")
        with open(filePath, 'w') as versionFile:
            versionFile.write("{0}".format(self.version()))

    def cachedVersion(self, folder):
        filePath = os.path.join(folder, "version.txt")
        version = 0
        if (os.path.exists(filePath)):
            with open(filePath, 'r') as versionFile:
                verStr = versionFile.read()
                if (len(verStr) > 0):
                    version = int(verStr)

        return version

    def download(self, delTree):
        workingPath = os.path.join(self.basePath, "build", self.name())
        currentVersion = self.cachedVersion(workingPath)
        needUpdate = (currentVersion < self.version())
        print("old version is {0}, new version is{1}".format(currentVersion, self.version()))
        if ((needUpdate or delTree) and os.path.exists(workingPath)):
            shutil.rmtree(workingPath, ignore_errors=True)

        print("start to build {0} ({1})...".format(self.name(), workingPath))
        downloaded = utils.ensureDownloaded(self.url(), workingPath, "src")
        if downloaded:
            self.updateVersion(workingPath)
            self.patch()

    def exports(self):
        ret = {}
        moduleDirName = "{0}_DIR".format(self.moduleName)
        ret[moduleDirName] = self.installedPath()
        os.environ[moduleDirName] = self.installedPath()
        return ret

    def checkExports(self, exports):
        if (self.installed()):
            exports[self.moduleName] = self.exports()

    def dependents(self):
        return []

    def patch(self):
        pass

    def invoke_make(self, config):
        buildCmd = "cmake --build . --config {0} --target install".format(config)
        if (self.sysName == "android"):
            buildCmd = buildCmd + " -- -j4"
        elif (self.sysName == "win64"):
            buildCmd = buildCmd + " -- /maxcpucount:8"
        elif (self.sysName == "arm"):
            buildCmd = buildCmd + " -- -j4"
        elif (self.sysName == "linux"):
            buildCmd = buildCmd + " -- -j4"
        print(buildCmd)

        p = subprocess.Popen(buildCmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=True)
        while True:
            line = p.stdout.readline()
            if not line:
                break
            try:
                if (self.sysName == "win64"):
                    print(line.decode('gbk').rstrip())
                else:
                    print(line.decode().rstrip())
            except Exception as e:
                print(str(e).encode().decode('utf-8').rstrip())
                pass
        p.wait()
        # os.system(buildCmd)

    def srcPath(self):
        return os.path.join(self.basePath, "build", self.name(), "src")

    def pcInstallPath(self):
        pcPath = os.path.join(self.basePath, self.sysName, "pkgconfig")
        if (not os.path.exists(pcPath)):
            os.makedirs(pcPath)
        return pcPath

    def buildPath(self):
        return os.path.join(self.basePath, "build", self.name(), "build" + self.sysName)

    def installPath(self):
        return self.installBasePath()

    def build(self, generator, options):
        sourcePath = self.srcPath()
        installPath = self.installBasePath()
        buildPath = self.buildPath()

        if (not os.path.exists(buildPath)):
            os.mkdir(buildPath)
        os.chdir(buildPath)

        optdef = ""
        options["CMAKE_INSTALL_PREFIX"] = installPath
        if (self.sysName == "linux"):
            options["CMAKE_BUILD_TYPE"] = "Release"

        if (self.sysName == "arm"):
            options["CMAKE_BUILD_TYPE"] = "Release"

        for key in options:
            val = options[key]
            if type(val) == str:
                optdef += "-D{0}=\"{1}\" ".format(key, val)
            elif type(val) == bool:
                valbool = "OFF"
                if (val):
                    valbool = "ON"
                optdef += "-D{0}={1} ".format(key, valbool)
            else:
                optdef += "-D{0}={1} ".format(key, val)

        if (not generator):
            configureCmd = "cmake {0} {1}".format(sourcePath, optdef)
        else:
            configureCmd = "cmake {0} {1} {2}".format(sourcePath, generator, optdef)
        print("start to configure with cmake ... ")
        print(configureCmd)
        os.system(configureCmd)

        # arm we only build release
        if (self.sysName != "arm" and self.sysName != "android" and self.sysName != "linux"):
            self.invoke_make("Debug")
        self.invoke_make("Release")

        os.chdir(self.basePath)

    def process(self, options, exports):
        if not self.supported():
            print("---- module [{0}] is not supported for [{1}] system".format(self.name(), self.sysName))
            return

        if (not "forceBuild" in options and self.installed()):
            print("\n---- module [{0}] has been installed\n".format(self.moduleName))
            return

        print("{0} was not found, start to build it ...".format(self.name()))
        mergedOptions = self.options()
        if ("sysOptions" in options):
            mergedOptions = utils.mergeDicts(mergedOptions, options["sysOptions"])
        for depMod in self.dependents():
            mergedOptions = utils.mergeDicts(mergedOptions, exports[depMod])

        delTree = ("forceDownload" in options) and options["forceDownload"]
        self.download(delTree)
        print(mergedOptions)
        generator = None
        if ("generator" in options):
            generator = "-G\"{0}\"".format(options["generator"])
        if ("forceBuild" in options):
            targetInstallPath = self.installBasePath()
            print("force rebuild, will delete: ", targetInstallPath)
            shutil.rmtree(targetInstallPath, ignore_errors=True)
        self.build(generator, mergedOptions)

    def copyPcFile(self):
        installPath = self.pcInstallPath()
        pcFile = self.moduleName + ".pc"
        pcFilePath = os.path.join(self.installBasePath(), "lib", "pkgconfig", pcFile)
        if (os.path.exists(pcFilePath)):
            shutil.copy(pcFilePath, installPath)
        return
