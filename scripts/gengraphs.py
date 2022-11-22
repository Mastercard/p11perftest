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


def splithalf(string):
    """split a sentence in two halves"""
    midpos = len(string) // 2
    curpos = 0

    for wordlen in map(len, string.split(' ')):
        curpos += wordlen + 1
        if curpos > midpos:
            break
    return string[:curpos - 1], string[curpos:]


def generate_graphs(xlsfp, sheetname):
    with xlsfp:
        # read from spreadsheet directly
        df = pd.read_excel(xlsfp, sheet_name=sheetname)

        for testcase in df["test case"].unique():
            if "signature" in testcase.lower() or "hmac" in testcase.lower():
                # for signature and HMAC algos, we are interested only in knowing the TPS
                col2, col3 = 'tps global value', col3name.format(measure)
                measure = 'tps'
                unit = 'TPS'
            else:
                # for other algos, we want to know the throughput
                col2, col3 = 'throughput global value', col3name.format(measure)
                measure = 'throughput'
                unit = 'Bytes/s'

            for item in sorted(df[graph_parameter].unique()):
                print(f"Drawing graph for {testcase} and {graph_parameter} {item}...", end='')
                frame = df.loc[(df['test case'] == testcase) & (df[graph_parameter] == item),
                               [xvar, 'latency average value', col2]]
                frame[col3] = frame[col2] / frame[xvar]

                fig, ax = plt.subplots(figsize=(16, 12))

                ax = frame.plot(colormap='cubehelix', x=xvar, y=ycomparison.format(measure), marker='o',
                                label=f'{measure}/{xvar}', ax=ax)
                frame.plot(x=xvar, y=f'{measure} global value', marker='X', label=f'{measure}, global', ax=ax)
                frame.plot(x=xvar, y='latency average value', marker='^', label='latency', secondary_y=True, ax=ax)
                title = "{}\n{}".format(*splithalf(f"{testcase} on a {item} bytes vector")) # CHANGE
                ax.set_title(title)
                ax.set_xlabel(xlabel)
                ax.set_ylabel(f'Troughput ({unit})')
                ax.right_ax.set_ylabel('Latency (ms)')
                ax.grid('on', which='major', axis='x')
                ax.grid('on', which='major', axis='y')
                ax.right_ax.grid('on', which='major', axis='y', linestyle='--')
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

    params = [('vector size', 'threads', 'Vector Size (Bytes)', '{} thread value', 'vec', '{} thread value'),
              ('threads', 'vector size', '# of Threads', '{} per vector size', 'threads', '{} per vector size')]

    graph_parameter, xvar, xlabel, ycomparison, fnsub, col3name = params[args.size]

    generate_graphs(args.xls, args.table)
