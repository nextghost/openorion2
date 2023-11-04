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
# Library/utility for dumping LBX image palettes
# First required argument is LBX file to be dumped
# Any additional arguments list image/cursor assets to be dumped

from PIL import Image
import io
import struct
import sys
import traceback
from dumplbx import LBXArchive

def convert_palette(data):
    ret = list()
    for idx in range(len(data) // 4):
        r = data[4*idx+1] << 2
        g = data[4*idx+2] << 2
        b = data[4*idx+3] << 2
        ret.append((r << 16) | (g << 8) | b)
    return ret

def format_palette(start, data):
    for idx in range(len(data)):
        if idx % 8 == 0:
            if idx > 0:
                print()
            print('%3d: ' % (start + idx), end='')
        print('#%06x ' % data[idx], end='')
    print()

def get_image_pal(data):
    if len(data) < 16:
        raise RuntimeError('Asset is not an image')

    header = struct.unpack_from('<HHHHHH', data, 0)
    width, height = header[:2]
    framecount, frametime, flags = header[3:]

    if 20 + 4 * framecount > len(data):
        raise RuntimeError('Asset is not an image')

    pos = 16 + 4 * framecount

    if flags & 0x1000 == 0:
        return None

    start, size = struct.unpack_from('<HH', data, pos)
    pos += 4
    return start, convert_palette(data[pos:pos+4*size])

def get_cursor_pal(data):
    return 0, convert_palette(data[:1024])

def paldump(filename, idx):
    with open(sys.argv[1], 'rb') as fr:
        lbx = LBXArchive(fr)
        data = lbx.loadAsset(idx)

    if len(data) < 4:
        print('Asset is not an image')
        return

    if data[:4] == b'RIFF':
        print('Asset is not an image')
        return

    cnt, size = struct.unpack_from('<HH', data, 0)
    if cnt == 1 and size == len(data) - 4:
        print('Asset is not an image')
        return

    if len(data) == 12544 and data[:1024:4] == 256*b'\x01':
        start, colors = get_cursor_pal(data)
        format_palette(start, colors)
        return

    try:
        start, colors = get_image_pal(data)
        format_palette(start, colors)
    except:
        pass

if __name__ == '__main__':
    for idx in sys.argv[2:]:
        idx = int(idx)
        print('Asset %s:' % idx)
        paldump(sys.argv[1], idx)
        print()
