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
                col2, col3 = 'tps thread value', 'tps global value'
                measure = 'tps'
                unit = 'TPS'
            else:
                # for other algos, we want to know the throughput
                col2, col3 = 'throughput thread value', 'throughput global value'
                measure = 'throughput'
                unit = 'Bytes/s'

            for vectorsize in sorted(df["vector size"].unique()):
                print(f"Drawing graph for {testcase} and vector size {vectorsize}...", end='')
                frame = df.loc[(df['test case'] == testcase) & (df['vector size'] == vectorsize),
                               ['threads', 'latency average value', col2, col3]]

                fig, ax = plt.subplots(figsize=(16, 12))

                ax = frame.plot(colormap='cubehelix', x='threads', y=f'{measure} thread value', marker='o',
                                label=f'{measure}/thread', ax=ax)
                frame.plot(x='threads', y=f'{measure} global value', marker='X', label=f'{measure}, global', ax=ax)
                frame.plot(x='threads', y='latency average value', marker='^', label='latency', secondary_y=True, ax=ax)
                title = "{}\n{}".format(*splithalf(f"{testcase} on a {vectorsize} bytes vector"))
                ax.set_title(title)
                ax.set_xlabel('# of threads')
                ax.set_ylabel(f'Troughput ({unit})')
                ax.right_ax.set_ylabel('Latency (ms)')
                ax.grid('on', which='major', axis='x')
                ax.grid('on', which='major', axis='y')
                ax.right_ax.grid('on', which='major', axis='y', linestyle='--')
                plt.tight_layout()
                filename = testcase.lower().replace(' ', '_')
                plt.savefig(f'{filename}-vec{vectorsize}.svg', format='svg', orientation='landscape')
                plt.savefig(f'{filename}-vec{vectorsize}.png', format='png', orientation='landscape')
                plt.cla()
                plt.close(fig)
                print('OK', flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generate graphs from spreadsheet of p11perftest results')
    parser.add_argument('xls', metavar='FILE', type=argparse.FileType('rb'), help='Path to Excel spreadsheet', )
    parser.add_argument('-t', '--table', help='Table name', default='Sheet1')
    args = parser.parse_args()

    generate_graphs(args.xls, args.table)
