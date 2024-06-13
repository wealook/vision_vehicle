# -*- coding: utf-8 -*-
from __future__ import (absolute_import, print_function)
import os, sys
import tarfile
import shutil
import errno

if sys.version_info >= (3,):
    import urllib.request as urllib2
    import urllib.parse as urlparse
else:
    import urllib2
    import urlparse

def mergeDicts(one, two, three = None):
    ret = {}
    if (one):
        for key in one:
            ret[key] = one[key]

    if (two):
        for key in two:
            ret[key] = two[key]
    
    if (three):
        for key in three:
            ret[key] = three[key]
            
    return ret

def download_file(url, dest=None):
    """ 
    Download and save a file specified by url to dest directory,
    """
    print("Start to download url: {0}".format(url))
    #context = ssl.SSLContext(ssl.PROTOCOL_TLSv1)
    u = urllib2.urlopen(url)

    scheme, netloc, path, query, fragment = urlparse.urlsplit(url)
    filename = os.path.basename(path)
    if not filename:
        filename = 'downloaded.file'
    if dest:
        filename = os.path.join(dest, filename)

    with open(filename, 'wb') as f:
        meta = u.info()
        meta_func = meta.getheaders if hasattr(meta, 'getheaders') else meta.get_all
        meta_length = meta_func("Content-Length")
        file_size = None
        if meta_length:
            file_size = int(meta_length[0])
        print("Downloading: {0} Bytes: {1}".format(url, file_size))

        file_size_dl = 0
        block_sz = 8192
        while True:
            buffer = u.read(block_sz)
            if not buffer:
                break

            file_size_dl += len(buffer)
            f.write(buffer)

            status = "{0:16}".format(file_size_dl)
            if file_size:
                status += "   [{0:6.2f}%]".format(file_size_dl * 100 / file_size)
            status += chr(13)
            print(status, end="")
        print("Download has successfully completed.")

    return filename

def ensureDownloaded(url, basePath, dest):
    if (not os.path.exists(basePath)):
        os.makedirs(basePath)
        
    destPath = os.path.join(basePath, dest)
    if (not os.path.exists(destPath)):
        print("start to download {0} ...".format(dest))
        filename = download_file(url, dest=basePath)
        zipFilePath = os.path.join(basePath, filename)
        print("downloaded to {0}".format(zipFilePath))

        tar = tarfile.open(zipFilePath,"r")
        name = os.path.commonprefix(tar.getnames())
        tar.extractall(basePath)
        os.rename(os.path.join(basePath, name), destPath)
        
        tar.close()
        print("extracted to {0}".format(destPath))
        return True
    else:
        print("{0} module has been downloaded.".format(dest))
        return False

def copy(src, dest):
    try:
        shutil.copytree(src, dest)
    except OSError as e:
        if e.errno == errno.ENOTDIR:
            shutil.copy(src, dest)
        else:
            print('Directory not copied. Error: %s' % e)
