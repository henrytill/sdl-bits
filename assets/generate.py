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

# https://github.com/octaviopardo/EBGaramond12
EB_GARAMOND_12 = {
    'target': 'EBGaramond12',
    'url': 'https://github.com/octaviopardo/EBGaramond12/archive/e608414f52e532b68e2182f96b4ce9db35335593.tar.gz',
    'tarball': 'EBGaramond12.tar.gz',
    'output_dir': 'EBGaramond12'
}


def fetch(url, file):
    with urllib.request.urlopen(url) as src:
        with open(file, 'wb') as dst:
            shutil.copyfileobj(src, dst)


def expand(source, output_dir):
    with tarfile.open(source) as archive:
        archive.extractall(path=output_dir)


def ucs_fonts():
    if not os.path.isfile(UCS_FONTS['tarball']):
        print('Downloading ' + UCS_FONTS['url'])
        fetch(UCS_FONTS['url'], UCS_FONTS['tarball'])
    if not os.path.isdir(UCS_FONTS['output_dir']):
        expand(UCS_FONTS['tarball'], UCS_FONTS['output_dir'])


def eb_garamond_12():
    if not os.path.isfile(EB_GARAMOND_12['tarball']):
        print('Downloading ' + EB_GARAMOND_12['url'])
        fetch(EB_GARAMOND_12['url'], EB_GARAMOND_12['tarball'])
    if not os.path.isdir(EB_GARAMOND_12['output_dir']):
        expand(EB_GARAMOND_12['tarball'], EB_GARAMOND_12['output_dir'])


def main():
    argv = sys.argv
    print('Running ' + argv[0] + ' with target: ' + argv[1])
    if argv[1] == UCS_FONTS['target']:
        ucs_fonts()
    elif argv[1] == EB_GARAMOND_12['target']:
        eb_garamond_12()


if __name__ == "__main__":
    main()
