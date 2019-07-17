#!/usr/bin/env python3
# script that takes multiple race record files (.dtb) and merges them together
# written by heinrich5991

import sys

from collections import namedtuple
from decimal import Decimal

if sys.version_info < (3, 7):
    from collections import OrderedDict as dict

def chunks(l, n):
    for i in range(0, len(l), n):
        yield l[i:i+n]

class Record(namedtuple('Record', 'name time checkpoints')):
    @staticmethod
    def parse(lines):
        if len(lines) != 3:
            raise ValueError("wrong amount of lines for record")
        name = lines[0]
        time = Decimal(lines[1])
        checkpoints_str = lines[2].split(' ')
        if len(checkpoints_str) != 26 or checkpoints_str[25] != "":
            raise ValueError("wrong amount of checkpoint times: {}".format(len(checkpoints_str)))
        checkpoints_str = checkpoints_str[:25]
        checkpoints = tuple(Decimal(c) for c in checkpoints_str)
        return Record(name=name, time=time, checkpoints=checkpoints)

    def unparse(self):
        return "\n".join([self.name, str(self.time), " ".join([str(cp) for cp in self.checkpoints] + [""]), ""])

def read_records(file):
    contents = file.read().splitlines()
    return [Record.parse(c) for c in chunks(contents, 3)]

def main():
    import argparse
    p = argparse.ArgumentParser(description="Merge multiple DDNet race database files")
    p.add_argument("db", metavar="DB", nargs='+', help="Databases to merge")
    p.add_argument("--stats", action='store_true', help="Display some stats at the end of the merge process")
    args = p.parse_args()

    total = 0
    num_overridden = 0
    num_rev_overridden = 0
    num_duplicates = 0

    merged_records = dict()
    for db in args.db:
        with open(db) as f:
            records = read_records(f)
        for record in records:
            total += 1
            if record.name in merged_records:
                if record == merged_records[record.name]:
                    num_duplicates += 1
                elif record.time < merged_records[record.name].time:
                    num_overridden += 1
                    merged_records[record.name] = record
                else:
                    num_rev_overridden += 1
            else:
                merged_records[record.name] = record

    for entry in merged_records.values():
        print(entry.unparse(), end="")

    if args.stats:
        print("Total number of processed entries: {}".format(total), file=sys.stderr)
        if num_overridden:
            print("Number of overridden entries: {}".format(num_overridden), file=sys.stderr)
        if num_rev_overridden:
            print("Number of stale entries: {}".format(num_rev_overridden), file=sys.stderr)
        if num_duplicates:
            print("Number of duplicate entries: {}".format(num_duplicates), file=sys.stderr)
        print("Total number of entries: {}".format(len(merged_records)), file=sys.stderr)

if __name__ == '__main__':
    sys.exit(main())

