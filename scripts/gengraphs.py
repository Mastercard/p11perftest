#!/usr/bin/env python
# coding: utf-8

#
# Copyright (c) 2021 Mastercard
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

import argparse
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

def splithalf(string):
    """split a sentence in two halves"""
    midpos = len(string) // 2
    curpos = 0

    for wordlen in map(len, string.split(' ')):
        curpos += wordlen + 1
        if curpos > midpos:
            break
    return string[:curpos - 1], string[curpos:]



def format_title1(s1, s2):
    if str(s2)[0]==8:
        return f"{s1} on an {s2} Bytes Vector".format(s1, s2)
    else:
        return f"{s1} on a {s2} Bytes Vector".format(s1, s2)
    
def format_title2(s1, s2):
    if s2==1:
        return f"{s1} on {s2} Thread".format(s1, s2)
    else:
        return f"{s1} on {s2} Threads".format(s1, s2)

def generate_graphs(xlsfp, sheetname):
    with xlsfp:
        # read from spreadsheet directly
        df = pd.read_excel(xlsfp, sheet_name=sheetname)

        for testcase in df["test case"].unique():
            if "signature" in testcase.lower() or "hmac" in testcase.lower():
                # for signature and HMAC algos, we are interested only in knowing the TPS
                measure = 'tps'
                unit = 'TPS'
                col2, col3 = 'tps global value', col3name.format(measure)
            else:
                # for other algos, we want to know the throughput
                measure = 'throughput'
                unit = 'Bytes/s'
                col2, col3 = 'throughput global value', col3name.format(measure)


            for item in sorted(df[graph_parameter].unique()):
                print(f"Drawing graph for {testcase} and {graph_parameter} {item}...", end='')
                frame = df.loc[(df['test case'] == testcase) & (df[graph_parameter] == item),
                               [xvar, 'latency average value', col2]]
                frame['latency_upper'] = frame['latency average value'] + df['latency average error']
                frame['latency_lower'] = frame['latency average value'] - df['latency average error']
                frame[col3] = frame[col2] / frame[xvar]


                fig, (ax, ax2) = plt.subplots(2, figsize=(16, 16), height_ratios=(3, 1))


                ax.plot(frame[xvar], frame[f'{measure} global value'], marker='X', color='tab:blue')
                title = "{}\n{}".format(*splithalf(format_title(testcase, item)))
                ax.set_title(title)
                ax.set_xlabel(xlabel)
                ax.set_ylabel(f'Throughput ({unit})')
                ax.grid('on', which='both', axis='x')
                ax.grid('on', which='major', axis='y')

                ax1 = ax.twinx() # add second plot to the same axes, sharing x-axis
                ax1.plot(np.nan, marker='X', label=f'{measure}, global', color='tab:blue')  # Make an agent in ax
                ax1.plot(frame[xvar], frame['latency average value'], label='latency', color='black', marker='^')
                ax1.plot(frame[xvar], frame['latency_upper'], label='latency error region', color='grey', alpha=0.5)
                ax1.plot(frame[xvar], frame['latency_lower'], color='grey', alpha=0.5)
                plt.fill_between(frame[xvar], frame['latency_upper'], frame['latency_lower'],
                                 facecolor='grey', alpha=0.5)
                ax1.set_ylabel('Latency (ms)')
                ax1.legend()


                # second subplot with tp per item
                ax2.plot(frame[xvar], frame[ycomparison.format(measure)], marker='o', label=f'{measure}/vector size', color='tab:red')
                ax2.set_xlabel(xlabel)
                ax2.set_ylabel(f'Throughput ({unit})')
                ax2.grid('on', which='both', axis='x')
                ax2.grid('on', which='major', axis='y')
                ax2.legend()


                plt.tight_layout()
                filename = testcase.lower().replace(' ', '_')
                plt.savefig(f'{filename}-{fnsub}{item}.svg', format='svg', orientation='landscape')
                plt.savefig(f'{filename}-{fnsub}{item}.png', format='png', orientation='landscape')
                plt.cla()
                plt.close(fig)
                print('OK', flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generate graphs from spreadsheet of p11perftest results')
    parser.add_argument('xls', metavar='FILE', type=argparse.FileType('rb'), help='Path to Excel spreadsheet', )
    parser.add_argument('-t', '--table', help='Table name', default='Sheet1')
    parser.add_argument('-s', '--size', action='store_true',
                        help='''Generate graphs showing vector size vs throughput/latency
                                 (default: threads vs throughput/latency)''')
    args = parser.parse_args()

    params = {False: ('vector size', 'threads', '# of Threads', '{} thread value', 'vec', '{} thread value', format_title1),
              True: ('threads', 'vector size', 'Vector Size (Bytes)', '{} per vector size', 'threads', '{} per vector size', format_title2)}
    graph_parameter, xvar, xlabel, ycomparison, fnsub, col3name, format_title = params[args.size]

    generate_graphs(args.xls, args.table)
