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
        try:
            testcases = json.loads( f.read() );

            for testcase,keys in testcases.items():
                for key, vectors in keys.items():
                    for vectorname, vector in vectors.items():
                        yield f.name,testcase,key,vectorname,vector

        except Exception as e:
            print(f"*** got an error while processing {f.name}: \"{e}\", skipping that file")

class Converter:
    def __init__(self, toxlsx):
        self.toxlsx = toxlsx
        self.row = 0
        self.col = 0
        self.titlerow = False
        self.maxrow = self.row
        self.maxcol = self.col
        self.columns = list()

    def __enter__(self):
        self.workbook = xlsxwriter.Workbook(self.toxlsx, options={'nan_inf_to_errors': True})
        self.worksheet = self.workbook.add_worksheet()
        self.percent_format = self.workbook.add_format({'num_format' : '0.00%' })

        return self

    def __exit__(self ,type, value, traceback):
        self.worksheet.add_table(*self.size, {'style': 'Table Style Medium 16', 'columns' : self.columns})
        self.workbook.close()

    @property
    def size(self):
        return 0,0,self.maxrow-1,self.maxcol-1

    def add_a_row(self, filename, testcase, key, vectorname, vector):

        def recursive_title(vector, prefix=""):
            for subk,subv in vector.items():
                if not isinstance(subv,(dict)):
                    column_title = (prefix + f"{subk} ").strip()
                    column_dict = { 'header':column_title }
                    if column_title.endswith('relerr'): # special case: if relerr in the name, then we show percents
                        column_dict['format'] = self.percent_format

                    # remember for later, when we call __exit__()
                    self.columns.append(column_dict)

                    # write cell
                    self.worksheet.write(self.row, self.col, column_title)
                    self.col+=1
                else:
                    recursive_title(subv, prefix + f"{subk} ")

        def recursive_value(vector, prefix=""):
            for subk,subv in vector.items():
                if not isinstance(subv,(dict)):
                    self.worksheet.write(self.row, self.col, cast.get(subk, noop)(subv))
                    self.col+=1
                else:
                    recursive_value(subv, prefix + f"{subk} ")

        # the title has not been defined yet, let's fetch all the info from the file itself
        if self.titlerow is False:
            self.worksheet.write(self.row,0,'file name')
            self.worksheet.write(self.row,1,'test case')
            self.worksheet.write(self.row,2,'key label')
            self.worksheet.write(self.row,3,'vector name')
            self.columns.extend( [
                { 'header' : 'file name'},
                { 'header' : 'test case'},
                { 'header' : 'key label'},
                { 'header' : 'vector name'},])

            self.col=4

            recursive_title(vector)
            self.maxcol = self.col

            self.titlerow = True
            self.row+=1

        self.worksheet.write(self.row,0,filename)
        self.worksheet.write(self.row,1,testcase)
        self.worksheet.write(self.row,2,key)
        self.worksheet.write(self.row,3,vectorname)
        self.col=4
        # we have defined the title already, proceed with row data
        recursive_value(vector)

        self.row+=1
        self.col=0
        self.maxrow = self.row

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
