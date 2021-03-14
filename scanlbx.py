#!/usr/bin/python3
#
# This file is part of OpenOrion2
#
# Copyright (C) 2020 next_ghost
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# ----
#
# Library/utility for scanning LBX files

import io
import struct
import sys

def cleanstr(charbuf):
    return charbuf.partition(b'\0')[0].decode('ASCII')

def readlbx(filename):
    with open(filename, 'rb') as fr:
        data = fr.read()
    count, magic = struct.unpack_from('<HH', data, 0)
    pos = 8

    if count == 0x4d53 and magic == 0x324b:
        print('%s: Smacker video' % filename)
        return

    if magic != 0xfead:
        raise RuntimeError('%s: Invalid LBX file' % filename)

    print('%s: %d assets' % (filename, count))
    (header_end,) = struct.unpack_from('<I', data, pos)
    asset_index = []

    for idx in range(count):
        start, end = struct.unpack_from('<II', data, pos)
        asset_index.append((start, end - start))
        pos += 4

    pos += 4

    for idx in range(pos, header_end):
        if data[idx] != 0:
            print('Extra data in header at position %d' % idx)
            break

    for idx, (start, size) in enumerate(asset_index):
        print('Asset %d: offset %d, size %d bytes' % (idx, start, size))

        if size < 16:
            print('Unknown tiny asset (%d bytes)' % size)
            continue

        if data[start:start+4] == b'RIFF':
            print('WAV data')
            continue

        width, height, tmp, frames = struct.unpack_from('<HHHH', data, start)
        ftime, flags, offset = struct.unpack_from('<HHI', data, start+8)

        if width == 1 and height + 4 == size:
            print('Text?')
            continue

        if 20 + 4 * frames > size:
            print('Unknown asset type')
            continue

        if flags & 0x1000 != 0:
            (palstart, palsize) = struct.unpack_from('<HH', data,
                start+16+4*frames)
            palsize += 1
        else:
            palsize = 0

        if offset == 16 + 4 * (frames + palsize):
            print('Image, %dx%d, %d frames @%dms' % (width, height, frames, ftime))
            if palsize > 0:
                print('%d color palette starting at %d' % (palsize-1, palstart))
            else:
                print('No palette')
            if flags & 0x1000 != flags:
                print('Unknown image flags: %x' % flags)
            if tmp != 0:
                raise RuntimeError('Unknown value: %d' % tmp)
        else:
            print('Unknown asset type')

if __name__ == '__main__':
    for filename in sys.argv[1:]:
        readlbx(filename)
