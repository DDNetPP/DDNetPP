#!/usr/bin/env python3

import sys
import os
import hashlib

import numpy
import twmap


class Tile:
    def __init__(self, colon_fmt: str):
        splits = colon_fmt.split(':')
        print(splits)
        self.group_offset = int(splits[0])
        self.layer_offset = int(splits[1])
        self.x =            int(splits[2])
        self.y =            int(splits[3])
        self.index =        int(splits[4])
        self.flags =        int(splits[5])

arg_input_map = ''
arg_output_folder = ''
arg_tiles: list[Tile] = []

if __name__ == '__main__':
    if len(sys.argv) < 4:
        # ./ddpp_scripts/map_set_tiles.py ~/.teeworlds/maps/tmp/a.map . 1:0:2:2:9:0 
        print("usage: map_set_tiles.py input.map output_folder group_offset:layer_offset:tile_x:tile_y:tile_index:tile_flags")
        sys.exit(1)
    arg_input_map =         sys.argv[1]
    arg_output_folder =     sys.argv[2]
    for tile in sys.argv[3:]:
        arg_tiles.append(Tile(tile))

in_map = twmap.Map(arg_input_map)
for tile in arg_tiles:
    tiles = in_map.groups[tile.group_offset].layers[tile.layer_offset].tiles
    tiles[tile.y][tile.x] = [tile.index, tile.flags]
    in_map.groups[tile.group_offset].layers[tile.layer_offset].tiles = tiles

tmp_file = "/tmp/map.map"
in_map.save(tmp_file)
 
sha256_hash = hashlib.sha256()
with open(tmp_file, "rb") as f:
    for byte_block in iter(lambda: f.read(4096),b""):
        sha256_hash.update(byte_block)

map_name = arg_input_map.split('/')[-1].split('.map')[0]
out_filename = f"{map_name}_{sha256_hash.hexdigest()}.map"
out_file_path = os.path.join(arg_output_folder, out_filename)
os.rename(tmp_file, out_file_path)

print(sha256_hash.hexdigest())

