import logging
import os
import tarfile
import urllib.request
import shutil
import sys

# https://www.cl.cam.ac.uk/~mgk25/ucs-fonts.html
UCS_FONTS = {
    'target': 'ucs_fonts',
    'url': 'https://www.cl.cam.ac.uk/~mgk25/download/ucs-fonts.tar.gz',
    'tarball': 'ucs-fonts.tar.gz',
    'output_dir': 'ucs-fonts'
}

logging.basicConfig(stream=sys.stdout)
logger = logging.getLogger('generate')
logger.setLevel(logging.INFO)


def fetch(url, file):
    with urllib.request.urlopen(url) as src:
        with open(file, 'wb') as dst:
            shutil.copyfileobj(src, dst)


def expand(source, output_dir):
    with tarfile.open(source) as archive:
        def is_within_directory(directory, target):
            
            abs_directory = os.path.abspath(directory)
            abs_target = os.path.abspath(target)
        
            prefix = os.path.commonprefix([abs_directory, abs_target])
            
            return prefix == abs_directory
        
        def safe_extract(tar, path=".", members=None, *, numeric_owner=False):
        
            for member in tar.getmembers():
                member_path = os.path.join(path, member.name)
                if not is_within_directory(path, member_path):
                    raise Exception("Attempted Path Traversal in Tar File")
        
            tar.extractall(path, members, numeric_owner) 
            
        
        safe_extract(archive, path=output_dir)


def ucs_fonts():
    if not os.path.isfile(UCS_FONTS['tarball']):
        logger.info('Downloading %s', UCS_FONTS['url'])
        fetch(UCS_FONTS['url'], UCS_FONTS['tarball'])
    if not os.path.isdir(UCS_FONTS['output_dir']):
        expand(UCS_FONTS['tarball'], UCS_FONTS['output_dir'])


def main():
    argv = sys.argv
    logger.debug('Target: %s', argv[1])
    if argv[1] == UCS_FONTS['target']:
        ucs_fonts()


if __name__ == "__main__":
    main()
