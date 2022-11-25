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
from scipy.optimize import curve_fit


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
    if str(s2)[0]=='8':
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


                # add some regression lines
                def rline_throughput():
                    def throughput_model(z, a, b):
                        return a * z / (z + b)

                    popt, pcov = curve_fit(throughput_model, frame['vector size'], frame[f'{measure} global value'] / 10000)
                    x_tp = np.linspace(16, 2048, 1000)
                    y_tp = throughput_model(x_tp, *popt)
                    df_throughput_model = pd.DataFrame({'vector size': x_tp, 'model values': y_tp * 10000})
                    ax.plot(df_throughput_model['vector size'], df_throughput_model['model values'], marker=',', color='tab:green', linestyle='--')
                    ax1.plot(np.nan, color='tab:green', linestyle='--', label=r"""Throughput model: $y=\frac{{{}x}}{{x+{}}}$""".format(int(popt[0] * 10000), int(popt[1])))

                def rline_latency():
                    def latency_model(z, a, b):
                        return a + z * b

                    popt1, pcov1 = curve_fit(latency_model, frame['vector size'], frame['latency average value'])
                    x_lt = np.linspace(16, 2048, 100)
                    y_lt = latency_model(x_lt, *popt1)
                    df_latency_model = pd.DataFrame({'vector size': x_lt, 'model values': y_lt})
                    a, b = '{0:.3f}'.format(popt1[0]), '{0:.3f}'.format(popt1[1])
                    ax1.plot(df_latency_model['vector size'], df_latency_model['model values'], marker=',', color='orange', label=r'Latency model: $y={}+{}x$'.format(a, b))
                    ax1.legend()

                if hasattr(args, "reglines"):
                    if args.reglines:
                        rline_throughput()
                        rline_latency()

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

    subparsers = parser.add_subparsers(dest='indvar')
    size = subparsers.add_parser('size',
                                 help='''Set vector size as independent variable.''')
    size.add_argument('--reglines', help='add lines of best fit for latency and throughput using predefined mathematical model', action='store_true')
    threads = subparsers.add_parser('threads', help='''Set number of threads as independent variable.''')


    args = parser.parse_args()

    if args.indvar==None:
        args.indvar = 'threads'

    params = {'threads': ('vector size', 'threads', '# of Threads', '{} thread value', 'vec', '{} thread value', format_title1),
              'size': ('threads', 'vector size', 'Vector Size (Bytes)', '{} per vector size', 'threads', '{} per vector size', format_title2)}
    graph_parameter, xvar, xlabel, ycomparison, fnsub, col3name, format_title = params[args.indvar]

    generate_graphs(args.xls, args.table)
