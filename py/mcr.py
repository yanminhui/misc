#!/usr/bin/env python
# -*- encoding: utf-8 -*-
#
# Author: yanminhui <yanminhui163@163.com>
#

from __future__ import print_function, unicode_literals

import argparse
import bz2
import gzip
import math
import io
import os
import sys
import time
import zlib

if 3 <= sys.version_info.major and 3 <= sys.version_info.minor:
    import lzma


def get_size(filename):
    size = os.path.getsize(filename)
    if size:
        return size
    with open(filename, 'rb') as f:
        f.seek(0, os.SEEK_END)
        return f.tell()
    return size

def iterate_files(fpath):
    if not os.path.isdir(fpath):
        yield fpath
    for root, dirs, files in os.walk(fpath):
        for file_ in files:
            yield os.path.join(root, file_)
        for dir in dirs:
            iterate_files(os.path.join(root, dir))

def get_size_recurse(fpath):
    return sum(map(get_size, iterate_files(fpath)))

def format_bytes(byte_count):
    indicators = ('Bytes', 'KB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB', 'YB')
    step = 0
    if byte_count:
        step = int(math.floor(math.log(byte_count) / math.log(1024)))
    value = round(byte_count / math.pow(1024, step), 2)
    indicator = indicators[step]
    return '{} {}'.format(value, indicator)

def format_seconds(seconds):
    indicators = ('secs', 'mins', 'hour')
    step = 0
    if 1 <= seconds:
        step = int(math.floor(math.log(seconds) / math.log(60)))
    if len(indicators) < step + 1:
        step = len(indicators) - 1
    value = round(seconds / math.pow(60, step), 2)
    indicator = indicators[step]
    return '{} {}'.format(value, indicator)


class Progress(object):

    def __init__(self, rng_last=100, rng_first=0):
        self._rng_first = rng_first
        self._rng_last = rng_last

        self._pos = rng_first
        self._clock_start = time.time()
        self._clock_stop = self._clock_start

    def range(self):
        return self._rng_first, self._rng_last

    def set_pos(self, pos):
        self._pos = pos
        self._clock_stop = time.time()

    def pos(self):
        return self._pos

    def percent(self):
        rng = max(self._rng_last - self._rng_first, 1)
        return round(((self._pos - self._rng_first) * 100) / rng, 2)

    def expired(self):
        return round(self._clock_stop - self._clock_start, 2)

    def remain(self, **kwargs):
        p = kwargs.get('percent') if 'percent' in kwargs else self.percent()
        e = kwargs.get('expired') if 'expired' in kwargs else self.expired()
        return round(((100 - p) * e) / max(p, 0.01), 2)

    def report_status(self):
        p = self.percent()
        e = self.expired()
        r = self.remain(percent=p, expired=e)
        return 'progress: {} %, expired: {}, remain: {}'.format(
            p, format_seconds(e), format_seconds(r))

class Compressor(object):

    @classmethod
    def levels(cls, filter):
        lvls = range(-1, 10)
        if filter == -2:
            return lvls
        elif filter in lvls:
            return [filter]
        return [-1]  # default

    def __init__(self, engine=None):
        self._engine = engine

    def set_engine(self, engine):
        self._engine = engine

    def engine(self):
        return self._engine

    def compress(self, data, **kwargs):
        pass

    def flush(self):
        return ''

class ZlibCompressor(Compressor):

    def __init__(self, **kwargs):
        super(ZlibCompressor, self).__init__()
        self._lvl = self.levels(kwargs.get('level'))[0]
        
    def compress(self, data):
        return zlib.compress(data, self._lvl)

class GzipCompressor(Compressor):

    @classmethod
    def levels(cls, filter):
        lvls = range(0, 10)
        if filter == -2:
            return lvls
        elif filter in lvls:
            return [filter]
        return [9]  # default

    def __init__(self, **kwargs):
        super(GzipCompressor, self).__init__()
        self._lvl = self.levels(kwargs.get('level'))[0]

    def compress(self, data):
        buf = io.BytesIO()
        with gzip.GzipFile(fileobj=buf, mode='wb',
                compresslevel=self._lvl) as f:
            f.write(data)
        return buf.getvalue()

class BZ2Compressor(Compressor):

    @classmethod
    def levels(cls, filter):
        lvls = range(1, 10)
        if filter == -2:
            return lvls
        elif filter in lvls:
            return [filter]
        return [9]  # default
    
    def __init__(self, **kwargs):
        lvl = kwargs.get('level')
        if lvl:
            v = self.levels(lvl)[0]
            super(BZ2Compressor, self).__init__(bz2.BZ2Compressor(v))
        super(BZ2Compressor, self).__init__(bz2.BZ2Compressor())

    def compress(self, data):
        eng = super(BZ2Compressor, self).engine()
        return eng.compress(data)

    def flush(self):
        eng = super(BZ2Compressor, self).engine()
        return eng.flush()

class LZMACompressor(Compressor):

    @classmethod
    def levels(cls, filter):
        return [0]  # default

    def __init__(self, **kwargs):
        kwargs.pop('level')
        super(LZMACompressor, self).__init__(lzma.LZMACompressor(**kwargs))

    def compress(self, data):
        eng = super(LZMACompressor, self).engine()
        return eng.compress(data)

    def flush(self):
        eng = super(LZMACompressor, self).engine()
        return eng.flush()

def _parse_args(**comps):

    ap = argparse.ArgumentParser(description='Measure compression ratio.',
            formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    ap.add_argument('--verbose', help='print progress status',
        action='count')
    ap.add_argument('--chunk-size', help="data chunk's size (metric: KB)",
        choices=[int(math.pow(2, n)) for n in range(2, 10)],
        type=int, default=16)
    ap.add_argument('--name', help="compression algorithm's name",
        choices=comps.keys(), default='all')
    ap.add_argument('--level',
        help='controlling the level of compression, all = -2, default = -1',
        type=int, choices=range(-2, 10), default=-2)
    ap.add_argument('file', help='file or directory')
    args = ap.parse_args()

    # fix python2.7 support unicode
    input_file = args.file
    if not isinstance(args.file, type(u'')):
        input_file = args.file.decode('utf-8')

    return (args.verbose, args.chunk_size * 1024,
        args.name, args.level, input_file)

def _print(name=None, level=None, out_size=None, prog=None, **kwargs):

    formatter = '{name:5} {level:5} {outsize:10} {expired:10} {savings:8} ' \
                '{speed:12} {ratio:5} {progress:5} {remain:10}'
    if name is None:
        values = dict(name='NAME', level='LEVEL', outsize='OUTSIZE',
                      expired='EXPIRED', savings='%SAVINGS', speed='SPEED',
                      ratio='RATIO', progress='%PROG', remain='REMAIN'
        )
        return print(formatter.format(**values), **kwargs)

    input_fsize = prog.range()[1]
    cur_size = prog.pos()

    expired = prog.expired()
    percent = prog.percent()
    remain = prog.remain(expired=expired, percent=percent)
    speed = cur_size / max(expired, 1)

    values = dict(name=name, level=level, outsize=format_bytes(out_size),
                  expired=format_seconds(expired),
                  progress=percent,
                  remain=format_seconds(remain),
                  speed = format_bytes(speed) + 'ps')

    if cur_size == input_fsize:
        values['savings'] = round(((input_fsize - out_size) * 100.0) /
                input_fsize, 2)
        values['ratio'] = round(input_fsize * 1.0 / max(out_size, 1), 2)
    else:
        values['savings'] = '?'
        values['ratio'] = '?'
    return print('\r', formatter.format(**values), **kwargs)

def _main():

    comps = {'zlib': ZlibCompressor,
             'gzip': GzipCompressor,
             'bz2': BZ2Compressor}
    if 3 <= sys.version_info.major and 3 <= sys.version_info.minor:
        comps['lzma'] = LZMACompressor

    verbose, chunk_size, comp_name, comp_level, input_file = _parse_args(
        all=None, **comps)
    input_fsize = get_size_recurse(input_file)

    print('File: {}, Length: {}, Chunk Size: {}'.format(input_file,
        format_bytes(input_fsize), format_bytes(chunk_size)))

    # =========================
    # MEASURE COMPRESSION RATE
    # =========================
    _print()

    names = comps.keys() if comp_name == 'all' else [comp_name]
    for name, lvl in [(n, l) for n in names
            for l in comps[n].levels(comp_level)]:
        comp = comps[name](level=lvl)
        prog = Progress(input_fsize)
        cur_size = 0
        out_size = 0
        for file_ in iterate_files(input_file):
            with open(file_, 'rb') as f:
                while True:
                    buffer = f.read(chunk_size)
                    if not buffer:
                        break
                    cur_size += len(buffer)
                    out_size += len(comp.compress(buffer))

                    if verbose:
                        prog.set_pos(cur_size)
                        _print(name, lvl, out_size, prog, end='')
        out_size += len(comp.flush())
        prog.set_pos(cur_size)
        _print(name, lvl, out_size, prog)


if __name__ == '__main__':
    _main()
