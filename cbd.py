#!python3

"""
cbd.py: CMake Build Driver
"""

import json
import os
import shutil
import subprocess
import sys


cmake = shutil.which('cmake')


def update_env(env):
    ret = os.environ.copy()
    for key, value in env.items():
        if key == 'PATH':
            ret['PATH'] = value + os.pathsep + ret['PATH']
        else:
            ret[key] = value
    return ret


def capture_env(cmd):
    env = os.environ.copy()
    encoding = sys.stdout.encoding
    cmd.extend(['>NUL', '&&', 'set'])
    try:
        output = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, env=env).stdout.read().decode(encoding).splitlines()
        for elem in output:
            bind = elem.strip().split('=')
            env[bind[0]] = bind[1]
        return env
    except subprocess.CalledProcessError as error:
        exit(error.returncode)


def handle_env(target):
    if 'script' in target:
        return capture_env(target['script'])
    if 'env' in target:
        return update_env(target['env'])


def process_options(options):
    ret = []
    for key, value in options.items():
        ret.append('-D{k}={v}'.format(k=key, v=value))
    return ret


def process_generator(generator):
    return '-G{}'.format(generator)


def process_build_dir(build_dir):
    return '-B{}'.format(build_dir)


def configure_cmd(spec):
    ret = [cmake]
    ret.extend(process_options(spec['options']))
    ret.append(process_generator(spec['generator']))
    ret.append(process_build_dir(spec['build_dir']))
    return ret


def build_cmd(spec):
    return [cmake, '--build', spec['build_dir'], '--target', 'all']


def clean_cmd(spec):
    return [cmake, '--build', spec['build_dir'], '--target', 'clean']


def configure(target):
    try:
        cmd = configure_cmd(target['spec'])
        env = handle_env(target)
        subprocess.run(cmd, env=env, check=True)
    except subprocess.CalledProcessError as error:
        exit(error.returncode)


def build(target):
    try:
        cmd = build_cmd(target['spec'])
        env = handle_env(target)
        subprocess.run(cmd, env=env, check=True)
    except subprocess.CalledProcessError as error:
        exit(error.returncode)


def clean(target):
    try:
        cmd = clean_cmd(target['spec'])
        env = handle_env(target)
        subprocess.run(cmd, env=env, check=True)
    except subprocess.CalledProcessError as error:
        exit(error.returncode)


def configure_all(targets):
    for name, target in targets.items():
        print('Configuring', name, flush=True)
        configure(target)


def build_all(targets):
    for name, target in targets.items():
        print('Building', name, flush=True)
        build(target)


def clean_all(targets):
    for name, target in targets.items():
        print('Cleaning', name, flush=True)
        clean(target)


def main():
    argv = sys.argv
    with open("cbd-targets.json", "r") as file:
        targets = json.loads(file.read())
    if len(argv) == 1:
        configure_all(targets)
        build_all(targets)
    elif argv[1] == 'cfg' or argv[1] == 'configure':
        configure_all(targets)
    elif argv[1] == 'build':
        build_all(targets)
    elif argv[1] == 'clean':
        clean_all(targets)


if __name__ == '__main__':
    main()
