#!/usr/bin/env python
# coding: utf-8

import argparse
import sqlite3
import pandas as pd
import matplotlib.pyplot as plt

def splithalf(string):
    """split a sentence in two halves"""
    midpos=len(string)//2
    curpos=0

    for wordlen in map(len,string.split(' ')):
        curpos += wordlen + 1
        if curpos>midpos:
            break
    return string[:curpos-1], string[curpos:]


def generate_graphs(db, sheetname):
# Create your connection.
    with sqlite3.connect(db) as cnx:

        df = pd.read_sql_query(f"SELECT * FROM {sheetname}", cnx)

        for testcase in df["test case"].unique():
            for vectorsize in sorted(df["vector size"].unique()):
                print(f"Drawing graph for {testcase} and vector size {vectorsize}...", end='')
                frame = df.loc[ (df['test case']==testcase) & (df['vector size']==vectorsize), ["threads", "latency average value", "tps thread value", "tps global value"] ]

                fig, ax = plt.subplots(figsize=(16,12))

                ax = frame.plot(colormap='cubehelix', x='threads', y='tps thread value', marker='o', label='TPS/thread', ax=ax)
                frame.plot(x='threads', y='tps global value', marker='X', label='TPS, global', ax=ax)
                frame.plot(x='threads', y='latency average value', marker='^', label='latency', secondary_y=True, ax=ax)
                title = "{}\n{}".format(*splithalf(f"{testcase} on a {vectorsize} bytes vector"))
                ax.set_title(title)
                ax.set_xlabel('# of threads')
                ax.set_ylabel('Troughput (TPS)')
                ax.right_ax.set_ylabel('Latency (ms)')
                ax.grid('on', which='major', axis='x')
                ax.grid('on', which='major', axis='y')
                ax.right_ax.grid('on', which='major', axis='y', linestyle='--')
                plt.tight_layout()
                filename=testcase.lower().replace(' ','_')
                plt.savefig(f'{filename}-vec{vectorsize}.svg', format='svg', orientation='landscape')
                plt.savefig(f'{filename}-vec{vectorsize}.png', format='png', orientation='landscape')
                plt.cla()
                plt.close(fig)
                print('OK', flush=True)

if __name__ == '__main__':

    parser = argparse.ArgumentParser(description='Generate graphs from SQLITE database of p11perftest results')
    parser.add_argument('db', help='Path to SQLite3 database')
    parser.add_argument('-t', '--table', help='Table name', default='Sheet1')
    args = parser.parse_args()

    generate_graphs(args.db, args.table)


