#!/usr/bin/env python

import json
import xlsxwriter
import sys

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
    'errorcode' : noop
}

def convert(fromjson, toxlsx):
    with open(fromjson) as f:

        workbook = xlsxwriter.Workbook(toxlsx, options={'nan_inf_to_errors': True})
        worksheet = workbook.add_worksheet()

        row = 0
        col = 0
        title = None

        # recover JSON structure
        testcases = json.loads( f.read() )

        for testcase,keys in testcases.items():
            for key, vectors in keys.items():
                for vectorname, vector in vectors.items():
                    # if no title, insert title row
                    if title is None:
                        title = list(vector.keys())
                        title.insert(row, 'vector')
                        title.insert(row, 'label')
                        title.insert(row, 'testcase')

                        for item in title:
                            worksheet.write(row,col,item)
                            col+=1
                        row+=1
                        col=0

                    # insert data
                    worksheet.write(row,0,testcase)
                    worksheet.write(row,1,key)
                    worksheet.write(row,2,vectorname)
                    col=3
                    for prop,val in vector.items():
                        worksheet.write(row,col,cast[prop](val))
                        col+=1
                    # advance in table
                    row+=1
                    col=0

        workbook.close()
        return row-1

if __name__ == '__main__':

    if len(sys.argv)!=3:
        print(f"Usage: {sys.argv[0]} FROMJSON TOXLSX")
        exit(1)
    else:
        numrows = convert(sys.argv[1], sys.argv[2])
        print(f"converted {numrows} entries from {sys.argv[1]} to {sys.argv[2]}")
