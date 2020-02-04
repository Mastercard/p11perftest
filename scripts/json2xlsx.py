#!/usr/bin/env python

import json
import xlsxwriter
import sys
import argparse

noop = lambda x : x

cast = {
    'iterations' : int,
    'total iterations' : int,
    'threads' : int,
    'size' : int,
    'value' : float,
    'error' : float,
    'relerr': float
}


def retrieve_rows(listofjsons):
    for f in listofjsons:
        # recover JSON structure
        testcases = json.loads( f.read() );

        for testcase,keys in testcases.items():
            for key, vectors in keys.items():
                for vectorname, vector in vectors.items():
                    yield f.name,testcase,key,vectorname,vector


class Converter:
    def __init__(self, toxlsx):
        self.toxlsx = toxlsx
        self.row = 0
        self.col = 0
        self.titlerow = False

    def __enter__(self):
        self.workbook = xlsxwriter.Workbook(self.toxlsx, options={'nan_inf_to_errors': True})
        self.worksheet = self.workbook.add_worksheet()
        return self

    def __exit__(self ,type, value, traceback):
        self.workbook.close()

    def add_a_row(self, filename, testcase, key, vectorname, vector):

        def recursive_title(vector, prefix):
            for subk,subv in vector.items():
                if not isinstance(subv,(dict)):
                    self.worksheet.write(self.row, self.col, (prefix + f"{subk} ").strip())
                    self.col+=1
                else:
                    recursive_title(subv, prefix + f"{subk} ")

        def recursive_value(vector, prefix):
            for subk,subv in vector.items():
                if not isinstance(subv,(dict)):
                    self.worksheet.write(self.row, self.col, cast.get(subk, noop)(subv))
                    self.col+=1
                else:
                    recursive_value(subv, prefix + f"{subk} ")

        # the title has not been defined yet, let's fetch all the info from the file itself
        if self.titlerow is False:
            self.worksheet.write(self.row,3,'vector name')
            self.worksheet.write(self.row,2,'label')
            self.worksheet.write(self.row,1,'test case')
            self.worksheet.write(self.row,0,'file name')
            self.col=4

            recursive_title(vector, "")

            self.titlerow = True

        else:
            self.worksheet.write(self.row,3,vectorname)
            self.worksheet.write(self.row,2,key)
            self.worksheet.write(self.row,1,testcase)
            self.worksheet.write(self.row,0,filename)
            self.col=4
            # we have defined the title already, proceed with row data
            recursive_value(vector, "")

        self.row+=1
        self.col=0


if __name__ == '__main__':

    parser = argparse.ArgumentParser(description='Convert p11perftest JSON files to Excel spreadsheet format')
    parser.add_argument('input', metavar='JSONFILE', help='Path to JSON input file(s)', nargs='+', type=argparse.FileType('r'))
    parser.add_argument('output', metavar='XLSXFILE', help='Path to Excel XLSX spreadsheet file')
    args = parser.parse_args()

    numrows = 0
    with Converter(args.output) as converter:
        for row in retrieve_rows(args.input):
            converter.add_a_row(*row)
            numrows += 1

    print(f"converted {numrows} entries to {args.output}")
