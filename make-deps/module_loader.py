import os, sys

def sortModules(modules):
    resolved = []
    lastNum = len(modules)
    while(len(modules) > 0):
        unresolved = []
        for m in modules:
            depOK = True
            for d in m.dependents():
                found = False
                for r in resolved:
                    if (d == r.name()):
                        found = True
                        break
                if (not found):
                    depOK = False
            if (depOK):
                resolved.append(m)
            else:        
                unresolved.append(m)
        modules = unresolved
        if (lastNum == len(modules)):
            errstr = ""
            for m in unresolved:
                errstr = errstr + " " + m.name()
            print("there is dependency unresolved: {0}".format(errstr))
            break
        lastNum = len(modules)
    return resolved

def loadModules(currentPath, sysName):
    res = {}
    import os
    # check subfolders
    lst = os.listdir("modules")
    modules = []
    for d in lst:
        s = os.path.join("modules", d)
        if os.path.isdir(s):
            print("doesn't support recursive modules by now, ignored {0}".format(s))
            continue

        if (d[-3:] != ".py"):
            continue

        if sys.version_info >= (3,5):
            import importlib.util
            spec = importlib.util.spec_from_file_location("modules", s)
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)
        else:
            import imp
            module = imp.load_source("modules", s)

        modules.append(module.Builder(sysName, currentPath))
    
    modules = sortModules(modules)

    return modules