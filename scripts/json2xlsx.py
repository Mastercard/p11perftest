#!/usr/bin/env python

import json
import xlsxwriter
import sys
import argparse

noop = lambda x : x

cast = {
    'label' : noop,
    'vector': noop,
    'algorithm' : noop,
    'vector_size': int,
    'label' : noop,
    'threads' : int,
    'iterations' : int,
    'avg_elapsed' : float,
    'max_elapsed' : float,
    'avg_latency' : float,
    'avg_threadtps' : float,
    'min_globaltps' : float,
    'errorcode' : noop,
    'wallclock' : float
}


def retrieve_rows(listofjsons):
    for f in listofjsons:
        # recover JSON structure
        testcases = json.loads( f.read() )
    
        for testcase,keys in testcases.items():
            for key, vectors in keys.items():
                for vectorname, vector in vectors.items():
                    yield f.name,testcase,key,vectorname,vector


class Converter:
    def __init__(self, toxlsx):
        self.toxlsx = toxlsx
        self.row = 0
        self.col = 0
        self.title = None

    def __enter__(self):
        self.workbook = xlsxwriter.Workbook(self.toxlsx, options={'nan_inf_to_errors': True})
        self.worksheet = self.workbook.add_worksheet()
        return self

    def __exit__(self ,type, value, traceback):
        self.workbook.close()

    def add_a_row(self, filename, testcase, key, vectorname, vector):
        if self.title is None:
            self.title = list(vector.keys())
            self.title.insert(self.row, 'vector')
            self.title.insert(self.row, 'label')
            self.title.insert(self.row, 'testcase')
            self.title.insert(self.row, 'filename')

            for item in self.title:
                self.worksheet.write(self.row,self.col,item)
                self.col+=1
            self.row+=1
            self.col=0

        # insert data
        self.worksheet.write(self.row,0,filename)
        self.worksheet.write(self.row,1,testcase)
        self.worksheet.write(self.row,2,key)
        self.worksheet.write(self.row,3,vectorname)
        self.col=4
        for prop,val in vector.items():
            self.worksheet.write(self.row,self.col,cast[prop](val))
            self.col+=1
            # advance in table
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
