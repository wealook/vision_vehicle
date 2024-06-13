import sys, os


def getCmdOptions(options):
    options["modules"] = []
    for arg in sys.argv:
        if (arg == "vs2017"):
            options["generator"] = "Visual Studio 15 2017 Win64"
            options["sysName"] = "win64"
        elif (arg == "vs2017_x86"):
            options["generator"] = "Visual Studio 15 2017"
            options["sysName"] = "win32"
        elif (arg == "vs2013"):
            options["generator"] = "Visual Studio 12 2013 Win64"
            options["sysName"] = "win64"
        elif (arg == "vs2019"):
            options["generator"] = "Visual Studio 16 2019"
            options["arch"] = "-A x64"
            options["sysName"] = "win64"
        elif (arg == "vs2022"):
            options["generator"] = "Visual Studio 17 2022"
            options["sysName"] = "win64"
        elif (arg == "vs2019_86"):
            options["generator"] = "Visual Studio 16 2019"
            options["arch"] = "-A x86"
            options["sysName"] = "win32"
        elif (arg == "linux_arm"):
            options["generator"] = "Unix Makefiles"
            options["sysName"] = "arm"
        elif (arg == "linux"):
            options["generator"] = "Unix Makefiles"
            options["sysName"] = "linux"
        elif (arg == "--force"):
            options["forceBuild"] = True
        elif (arg == "--download"):
            options["forceDownload"] = True
        elif (arg.find('make-deps.py') == -1):
            options["modules"].append(arg)

    if (not options["generator"]):
        if sys.platform.startswith('win32'):
            options["sysName"] = "win64"
        elif sys.platform.startswith('linux'):
            options["generator"] = "Unix Makefiles"
            options["sysName"] = "linux"
        else:
            pass
