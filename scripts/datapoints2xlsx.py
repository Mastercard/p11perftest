#!/usr/bin/env python3

#
# Copyright (c) 2025 Mastercard
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# datapoints2xlsx.py will extract datapoints from JSON files and create an Excel spreadsheet.
# Each column contains the datapoints for a specific test case/key/vector combination.
# Columns are grouped with a wide title row.
#

import json
import xlsxwriter
import sys
import argparse


def retrieve_datapoints(listofjsons):
    """
    Generator that yields tuples of (filename, testcase, key, vectorname, datapoints_array)
    for each test case that contains datapoints.
    """
    for f in listofjsons:
        try:
            testcases = json.loads(f.read())

            # Handle format with thread grouping ('* thread-s' keys)
            if list(testcases.keys())[0].endswith('thread-s'):
                for threadgroupname, threadgroup in testcases.items():
                    for testcase, keys in threadgroup.items():
                        for key, vectors in keys.items():
                            for vectorname, vector in vectors.items():
                                if isinstance(vector, dict) and 'datapoints' in vector and isinstance(vector['datapoints'], list):
                                    yield f.name, threadgroupname, testcase, key, vectorname, vector['datapoints']
            # Legacy format without thread grouping
            else:
                for testcase, keys in testcases.items():
                    for key, vectors in keys.items():
                        # Check if this level contains datapoints directly (flat structure)
                        if isinstance(vectors, dict) and 'datapoints' in vectors and isinstance(vectors['datapoints'], list):
                            yield f.name, None, testcase, key, key, vectors['datapoints']
                        # Otherwise, iterate through vector names
                        elif isinstance(vectors, dict):
                            for vectorname, vector in vectors.items():
                                if isinstance(vector, dict) and 'datapoints' in vector and isinstance(vector['datapoints'], list):
                                    yield f.name, None, testcase, key, vectorname, vector['datapoints']

        except Exception as e:
            print(f"*** got an error while processing {f.name}: \"{e}\", skipping that file")


class DatapointsConverter:
    def __init__(self, toxlsx):
        self.toxlsx = toxlsx
        self.col = 0
        self.columns = []

    def __enter__(self):
        self.workbook = xlsxwriter.Workbook(self.toxlsx, options={'nan_inf_to_errors': True})
        self.worksheet = self.workbook.add_worksheet('latency (ms)')
        
        # Format for the header row
        self.header_format = self.workbook.add_format({
            'bold': True,
            'bg_color': '#D7E4BC',
            'border': 1
        })
        
        return self

    def __exit__(self, type, value, traceback):
        self.workbook.close()

    def add_datapoints_column(self, filename, threadgroup, testcase, key, vectorname, datapoints):
        """
        Add a column of datapoints to the spreadsheet.
        Row 0: Column header with test identification
        Row 1+: Individual datapoint values
        """
        # Create the column header
        if threadgroup:
            column_header = f"{filename} | {threadgroup} | {testcase} | {key} | {vectorname}"
        else:
            column_header = f"{filename} | {testcase} | {key} | {vectorname}"
        
        # Write the column header in row 0
        self.worksheet.write(0, self.col, column_header, self.header_format)
        
        # Write the datapoints starting from row 1
        for idx, datapoint in enumerate(datapoints):
            self.worksheet.write_number(1 + idx, self.col, float(datapoint))
        
        # Store column info for potential table creation
        self.columns.append({
            'header': column_header,
            'count': len(datapoints)
        })
        
        # Move to next column
        self.col += 1

    def finalize(self):
        """
        Adjust column widths for better readability.
        """
        # Set a reasonable width for all columns
        for col_idx in range(self.col):
            self.worksheet.set_column(col_idx, col_idx, 20)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Extract datapoints from p11perftest JSON files to Excel spreadsheet format',
        epilog='Each column contains the datapoints for one test case. '
               'The first row shows the grouped title with test identification.'
    )
    parser.add_argument('input', metavar='JSONFILE', 
                       help='Path to JSON input file(s) containing datapoints', 
                       nargs='+', 
                       type=argparse.FileType('r'))
    parser.add_argument('output', metavar='XLSXFILE', 
                       help='Path to Excel XLSX spreadsheet file')
    args = parser.parse_args()

    numcols = 0
    total_datapoints = 0
    
    with DatapointsConverter(args.output) as converter:
        for datapoint_info in retrieve_datapoints(args.input):
            filename, threadgroup, testcase, key, vectorname, datapoints = datapoint_info
            converter.add_datapoints_column(filename, threadgroup, testcase, key, vectorname, datapoints)
            numcols += 1
            total_datapoints += len(datapoints)
        
        converter.finalize()

    if numcols == 0:
        print(f"Warning: No datapoints found in input files. The output file may be empty.")
    else:
        print(f"Extracted {total_datapoints} datapoints from {numcols} test case(s) to {args.output}")
