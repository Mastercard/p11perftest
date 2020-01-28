#!/usr/bin/env python

import json
import xlsxwriter
import sys
import argparse

noop = lambda x : x

cast = {
    'label' : noop,
    'vector_size': int,
    'vector_unit': noop,
    'algorithm' : noop,
    'threads' : int,
    'iterations' : int,
    'totalcount' : int,
    'errorcode' : noop,
    'wallclock' : float,
    'timer_resolution' : float,
    'timer_unit' : noop,
    'latency_average_value' : float,
    'latency_average_unit' : noop,
    'latency_average_error' : float,
    'latency_average_relerr' : float,
    'latency_maximum_value' : float,
    'latency_maximum_unit' : noop,
    'latency_maximum_error' : float,
    'latency_maximum_relerr' : float,
    'latency_minimum_value' : float,
    'latency_minimum_unit' : noop,
    'latency_minimum_error' : float,
    'latency_minimum_relerr' : float,
    'tps_thread_value' : float,
    'tps_thread_unit' : noop,
    'tps_thread_error' : float,
    'tps_thread_relerr' : float,
    'tps_global_value' : float,
    'tps_global_unit' : noop,
    'tps_global_error' : float,
    'tps_global_relerr' : float,
    'throughput_thread_value' : float,
    'throughput_thread_unit' : noop,
    'throughput_thread_error' : float,
    'throughput_thread_relerr' : float,
    'throughput_global_value' : float,
    'throughput_global_unit' : noop,
    'throughput_global_error' : float,
    'throughput_global_relerr' : float,
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
        self.title = None

    def __enter__(self):
        self.workbook = xlsxwriter.Workbook(self.toxlsx, options={'nan_inf_to_errors': True})
        self.worksheet = self.workbook.add_worksheet()
        return self

    def __exit__(self ,type, value, traceback):
        self.workbook.close()

    def add_a_row(self, filename, testcase, key, vectorname, vector):
        if self.title is None:  # the title has not been defined yet, let's fetch all the info from the file itself
            self.title = list(vector.keys())
            self.title.insert(self.row, 'vector name')
            self.title.insert(self.row, 'label')
            self.title.insert(self.row, 'testcase')
            self.title.insert(self.row, 'filename')

            for item in self.title:
                # for resolution and vector, we have one level only
                if item in ('timer', 'vector') :
                    for sub1k,sub1v in vector[item].items():
                        self.worksheet.write(self.row, self.col, f"{item} {sub1k}")
                        self.col+=1

                # for latency, tps and throughput we have two levels down to explore.
                elif item in ('latency', 'tps', 'throughput') :
                    for sub1k,sub1v in vector[item].items():
                        for sub2k,sub2v in sub1v.items():
                            self.worksheet.write(self.row, self.col, f"{item} {sub1k} {sub2k}")
                            self.col+=1
                else:
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
            # for resolution, we have one level only
            if prop in ('timer', 'vector'):
                for sub1k,sub1v in val.items():
                    self.worksheet.write(self.row,self.col,cast[f"{prop}_{sub1k}"](sub1v))
                    self.col+=1
            # for latency, tps and throughput we have two levels down to explore.
            elif prop in ('latency', 'tps', 'throughput'):
                for sub1k,sub1v in val.items():
                    for sub2k,sub2v in sub1v.items():
                        self.worksheet.write(self.row,self.col,cast[f"{prop}_{sub1k}_{sub2k}"](sub2v))
                        self.col+=1
            else:
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
