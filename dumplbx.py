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
# Library/utility for dumping LBX files
# First required argument is LBX file to be dumped
# Second optional argument is LBX file with default palette
# Third argument is asset ID of image with default palette

from PIL import Image
import io
import struct
import sys
import traceback

def cleanstr(charbuf):
    return charbuf.partition(b'\0')[0].decode('ASCII')

def convert_palette(data):
    ret = list(data)
    for idx in range(len(data) // 4):
        ret[4*idx] = data[4*idx+1] << 2
        ret[4*idx+1] = data[4*idx+2] << 2
        ret[4*idx+2] = data[4*idx+3] << 2
        ret[4*idx+3] = 0xff
    return bytes(ret)

class LBXArchive:
    def __init__(self, fr):
        self._file = fr
        self._index = []

        self._file.seek(0, io.SEEK_SET)
        size, magic = struct.unpack('<HH', self._file.read(4))

        if magic != 0xfead:
            raise RuntimeError('Invalid LBX file')

        self._file.read(4)
        (start,) = struct.unpack('<I', self._file.read(4))

        for idx in range(size):
            (end,) = struct.unpack('<I', self._file.read(4))
            if end < start:
                raise RuntimeError('Invalid LBX entry offset')
            self._index.append((start, end - start))
            start = end

    def assetCount(self):
        return len(self._index)

    def loadAsset(self, idx):
        if idx < 0 or idx >= len(self._index):
            raise KeyError('Asset index %d out of range' % idx)
        offset, size = self._index[idx]
        self._file.seek(offset, io.SEEK_SET)
        return self._file.read(size)

class PaletteError(RuntimeError):
    pass

class MOOImage:
    def __init__(self, data, basepal):
        if len(data) < 16:
            raise RuntimeError('Asset is not an image')

        intcolors = None
        self.extcolors = set()
        self.colors = set()
        header = struct.unpack_from('<HHHHHH', data, 0)
        self.width, self.height = header[:2]
        self.realsize = [self.width, self.height, 0, 0]
        self.framecount, self.frametime, self.flags = header[3:]

        if 20 + 4 * self.framecount > len(data):
            raise RuntimeError('Asset is not an image')

        (start,) = struct.unpack_from('<I', data, 12)
        pos = 16
        frameindex = []

        for idx in range(self.framecount):
            (end,) = struct.unpack_from('<I', data, pos)
            if end < start:
                raise RuntimeError('Invalid LBX entry offset')
            frameindex.append((start, end))
            start = end
            pos += 4

        if len(data) != start:
            raise RuntimeError('Asset is not an image: %d != %d' % (len(data), end))

        if basepal is not None:
            self.palette = basepal[:1024]
            if len(self.palette) < 1024:
                self.palette += (1024 - len(self.palette)) * b'\0'
        else:
            self.palette = 1024 * b'\0'

        if self.flags & 0x1000 != 0:
            start, size = struct.unpack_from('<HH', data, pos)
            intcolors = range(start, start+size)
            pos += 4
            palette = list(self.palette)
            newpal = list(convert_palette(data[pos:pos+4*size]))
            palette[4*start:4*(start+size)] = newpal
            self.palette = bytes(palette)
        elif basepal is None:
            raise PaletteError('Image lacks palette, provide a default one')

        if self.flags & 0x800 != 0:
            self.palette = b'\0\0\0\0' + self.palette[4:]

        buf = self.width * self.height * [0]
        self.framelist = []

        for start, end in frameindex:
            if self.flags & 0x400:
                buf = self.width * self.height * [0]
            (frame, raw) = self.decode_frame(buf, data[start:end])
            self.colors.update(raw)
            self.framelist.append(frame)
            if intcolors is not None:
                self.extcolors.update((x for x in raw if (x not in intcolors)))

        if self.flags & 0x800 != 0:
            self.extcolors.discard(0)
            self.colors.discard(0)

    def decode_frame(self, buf, data):
        # Raw bitmap
        if self.flags & 0x100 != 0:
            ret = data[:self.width*self.height]
            return (ret, ret)

        # Sparse bitmap
        x, y = struct.unpack_from('<HH', data, 0)
        pos = 4
        raw = b''

        if x != 1:
            raise RuntimeError('Invalid frame start marker')

        self.realsize[1] = min(y, self.realsize[1])

        while y < self.height:
            ptr = y * self.width
            x = 0

            while True:
                size, skip = struct.unpack_from('<HH', data, pos)
                pos += 4
                if size == 0:
                    y += skip
                    break
                self.realsize[0] = min(x + skip, self.realsize[0])
                self.realsize[3] = max(y + 1, self.realsize[3])
                x += skip + size
                self.realsize[2] = max(x, self.realsize[2])
                if x > self.width:
                    raise RuntimeError('Scan line overflow at %d:%d' % (x, y))
                ptr += skip
                buf[ptr:ptr+size] = list(data[pos:pos+size])
                raw += data[pos:pos+size]
                ptr += size
                pos += size + size % 2
        self.realsize[2] -= self.realsize[0]
        self.realsize[3] -= self.realsize[1]
        return (bytes(buf), raw)

class MOOCursor(MOOImage):
    def __init__(self, data):
        self.width, self.height = (24, 480)
        self.framecount, self.frametime, self.flags = (1, 0, 0x1100)
        self.palette = convert_palette(data[:1024])
        self.framelist = [data[1024:12544]]

def dump_data(filename, data):
    with open(filename + '.dat', 'wb') as fw:
        fw.write(data)

def dump_wav(filename, data):
    with open(filename + '.wav', 'wb') as fw:
        fw.write(data)

def dump_text(filename, data):
    dump_data(filename, data)
    text, nul, tail = data[4:].partition(b'\0')
    if tail != len(tail) * b'\0':
        return
    with open(filename + '.txt', 'wb') as fw:
        fw.write(text)

def dump_image(filename, img):
    frames = []

    for bitmap in img.framelist:
        pic = Image.new('P', (img.width, img.height))
        pic.putdata(bitmap)
        pic.putpalette(img.palette, 'RGBX')
        frames.append(pic)

    # Multiple frames, save as GIF
    if len(frames) > 1:
        frametime = img.frametime
        if frametime < 10:
            frametime = 100
        frames[0].save(filename + '.gif', append_images=frames[1:],
            save_all=True, loop=0, duration=img.frametime)
    # Single frame, save as PNG
    else:
        frames[0].save(filename + '.png')

def dumplbx(filename, palette):
    print('Dumping LBX archive %s' % filename)
    basename = filename.rpartition('/')[2]
    if '.' in basename:
        basename = basename.rpartition('.')[0]
    basename += '-%d'

    with open(sys.argv[1], 'rb') as fr:
        lbx = LBXArchive(fr)

        for idx in range(lbx.assetCount()):
            assetname = basename % idx
            data = lbx.loadAsset(idx)

            if len(data) < 4:
                dump_data(assetname, data)
                continue

            if data[:4] == b'RIFF':
                dump_wav(assetname, data)
                continue

            cnt, size = struct.unpack_from('<HH', data, 0)
            if cnt == 1 and size == len(data) - 4:
                dump_text(assetname, data)
                continue

            dump_data(assetname, data)

            if len(data) == 12544 and data[:1024:4] == 256*b'\x01':
                print('Cursor pack')
                img = MOOCursor(data)
                dump_image(assetname, img)
                continue

            try:
                img = MOOImage(data, palette)
                if palette is None:
                    palette = img.palette
                dump_image(assetname, img)
                print('Image %d dimensions: %s' % (idx, str(img.realsize)))
                if img.extcolors:
                    print('Asset %d uses external colors: %s' % (idx, str(list(sorted(img.extcolors)))))
                print('Asset %d colors: %s' % (idx, str(list(sorted(img.colors)))))
            except PaletteError as e:
                print('Asset %d: %s' % (idx, e.args[0]))
            except:
                pass

def load_palette(filename, asset, basepal):
    with open(filename, 'rb') as fr:
        lbx = LBXArchive(fr)
        try:
            img = MOOImage(lbx.loadAsset(asset), basepal)
        except PaletteError:
            print('%s:%d does not have a palette' % (filename, asset))
            sys.exit(1)
        except:
            print('%s:%d is not an image' % (filename, asset))
            sys.exit(1)
        return img.palette

if __name__ == '__main__':
    palette = None
    idx = 2

    if len(sys.argv) % 2 != 0:
        print('Usage: %s file_to_dump.lbx [palette_archive.lbx palette_asset_num]' % sys.argv[0], file=sys.stderr)

    while idx + 2 <= len(sys.argv):
        palette = load_palette(sys.argv[idx], int(sys.argv[idx+1]), palette)
        idx += 2

    dumplbx(sys.argv[1], palette)
