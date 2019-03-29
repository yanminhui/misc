#!/usr/bin/env python
# -*- encoding: utf-8 -*-
#
# Author: yanminhui <yanminhui163@163.com>
#

from __future__ import print_function, unicode_literals

import argparse
import bz2
import math
import os
import time
import zlib


def get_size(filename):
    # pf = os.popen('lsblk -d -n -o SIZE -b ' + input_file)
    # g_in_size = int(pf.read().strip())
    return os.path.getsize(filename)

def format_bytes(byte_count):
    indicators = ('Bytes', 'KB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB', 'YB')
    step = int(math.floor(math.log(byte_count) / math.log(1024))) if byte_count else 0
    value = round(byte_count / math.pow(1024, step), 2)
    indicator = indicators[step]
    return '{} {}'.format(value, indicator)

def format_seconds(seconds):
    indicators = ('secs', 'mins', 'hour')
    step = int(math.floor(math.log(seconds) / math.log(60))) if 1 < seconds else 0
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

    def set_pos(self, pos):
        self._pos = pos
        self._clock_stop = time.time()

    def percent(self):
        rng = self._rng_last - self._rng_first
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
        self._kwargs = kwargs

    def compress(self, data):
        return zlib.compress(data, **self._kwargs)

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


def _parse_args(**comps):

    ap = argparse.ArgumentParser(description='Measure compression rate.',
                    formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    ap.add_argument('--verbose', help='print progress status',
                    action='count')
    ap.add_argument('--chunk-size', help="data chunk's size (metric: KB)",
                    choices=[int(math.pow(2, n)) for n in range(2, 10)],
                    type=int, default=16)
    ap.add_argument('--name', help="compression algorithm's name",
                    choices=comps.keys(), default='all')
    ap.add_argument('--level', help='controlling the level of compression, all = -2, default = -1',
                    type=int, choices=range(-2, 10), default=-2)
    ap.add_argument('file')
    args = ap.parse_args()

    return args.verbose, args.chunk_size * 1024, args.name, args.level, args.file

def _main():

    comps = {'zlib': ZlibCompressor,
             'bz2': BZ2Compressor}
    verbose, chunk_size, comp_name, comp_level, input_file = _parse_args(
        all=None, **comps)
    input_file_size = get_size(input_file)
    print('File: {}, Length: {}, Chunk Size: {}'.format(input_file,
        format_bytes(input_file_size), format_bytes(chunk_size)))

    # =========================
    # MEASURE COMPRESSION RATE
    # =========================
    formatter = '{name:5} {level:5} {outsize:10} {expired:10} {rate:5} ' \
                '{speed:12} {multiple:5} {percent:5} {remain:10}'
    print(formatter.format(name='NAME', level='LEVEL', outsize='OUTSIZE',
                           expired='EXPIRED', rate='%RATE', speed='SPEED',
                           multiple='MULTI', percent='%PROG',
                           remain='REMAIN'))

    names = comps.keys() if comp_name == 'all' else [comp_name]
    for name, lvl in [(n, l) for n in names for l in comps[n].levels(comp_level)]:
        comp = comps[name](level=lvl)
        with open(input_file, 'rb') as f:
            prog = Progress(input_file_size)
            cur_size = 0
            out_size = 0
            while True:
                buffer = f.read(chunk_size)
                if not buffer:
                    break
                cur_size += len(buffer)
                out_size += len(comp.compress(buffer))

                if not verbose:
                    continue
                prog.set_pos(cur_size)
                speed = cur_size / max(prog.expired(), 1)
                print('\r', formatter.format(name=name, level=lvl,
                        outsize=format_bytes(out_size), expired=format_seconds(prog.expired()),
                        rate='?', speed=format_bytes(speed)+'ps', multiple='?',
                        percent=prog.percent(), remain=format_seconds(prog.remain())),
                    sep='', end='')

            out_size += len(comp.flush())

            prog.set_pos(cur_size)
            rate = round(((input_file_size - out_size) * 100.0) / input_file_size, 2)
            speed = cur_size / max(prog.expired(), 1)
            multiple = round(input_file_size * 1.0 / max(out_size, 1), 2)
            print('\r', formatter.format(name=name, level=lvl,
                    outsize=format_bytes(out_size), expired=format_seconds(prog.expired()),
                    rate=rate, speed=format_bytes(speed) + 'ps', multiple=multiple,
                    percent=prog.percent(), remain=format_seconds(prog.remain())),
                sep='')


if __name__ == '__main__':
    _main()
