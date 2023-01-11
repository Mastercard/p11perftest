#!/usr/bin/env python3
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
    if str(s2)[0] == '8':
        return f"{s1} on an {s2} Bytes Vector".format(s1, s2)
    else:
        return f"{s1} on a {s2} Bytes Vector".format(s1, s2)


def format_title2(s1, s2):
    if s2 == 1:
        return f"{s1} on {s2} Thread".format(s1, s2)
    else:
        return f"{s1} on {s2} Threads".format(s1, s2)


def create_dataframe(xls, sheetname):
    """create a dataframe from an excel file; are we interested in throughput or transactions?"""
    df = pd.read_excel(xls, sheet_name=sheetname)
    df.sort_values(by=[xvar])
    return df


def determine_measure(testcase):
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
    return measure, unit, col2, col3


def create_graph_frame(df, testcase, item):
    measure, unit, col2, col3 = determine_measure(testcase)
    frame = df.loc[(df['test case'] == testcase) & (df[graph_parameter] == item),
                   [xvar, 'latency average value', col2]]
    frame['tp_upper'] = frame[col2] + df[f'{measure} global error']
    frame['tp_lower'] = frame[col2] - df[f'{measure} global error']
    frame['latency_upper'] = frame['latency average value'] + df['latency average error']
    frame['latency_lower'] = frame['latency average value'] - df['latency average error']

    frame[col3] = frame[col2] / frame[xvar]
    frame['tp_xvar_upper'] = frame[col3] + df[f'{measure} global error'] / frame[xvar]
    frame['tp_xvar_lower'] = frame[col3] - df[f'{measure} global error'] / frame[xvar]
    return frame, measure, unit, col2, col3


def comparison_labels(xlsfp, xlsfp2):
    if not args.comparison:
        xlsfp.label = '', ''
        if args.labels is not None:
            print('Not in comparison mode, ignoring labels. Did you forget to specify -c flag?')
    else:
        if args.labels is None:
            xlsfp.label = 'data set 1', '(data set 1)'
            xlsfp2.label = 'data set 2', '(data set 2)'
        else:
            xlsfp.label = args.labels[0], f'({args.labels[0]})'
            xlsfp2.label = args.labels[1], f'({args.labels[1]})'


def generate_graphs(xlsfp, sheetname, xlsfp2):
    comparison_labels(xlsfp, xlsfp2)
    xls_tuple = xlsfp, xlsfp
    if args.comparison:
        xls_tuple = xlsfp, xlsfp2

    with xls_tuple[0], xls_tuple[1]:
        # read from spreadsheet directly
        df1  = create_dataframe(xlsfp, sheetname)
	### could reintroduce this logic below. removed for now...
        # if args.comparison:
        #     df2, measure2, unit, col2, col3 = create_dataframe(xlsfp2, 'Sheet1')
        #     if not (measure1 == measure2) and (df1[graph_parameter].unique() == df2[graph_parameter].unique()):
        #         raise AssertionError('Please compare similar things.')
        #     measure = measure1
        # else:
        #     measure = measure1

        for testcase in df1["test case"].unique():
            for item in sorted(df1[graph_parameter].unique()):
                print(f"Drawing graph for {testcase} and {graph_parameter} {item}...", end='')
                frame1, measure, unit, col2, col3 = create_graph_frame(df1, testcase, item)
                if args.comparison:
                    frame2, _, _, _, _ = create_graph_frame(df2, testcase, item)

                fig, (ax, ax2) = plt.subplots(2, figsize=(16, 16), height_ratios=(3, 1))

                ax.plot(frame1[xvar], frame1[f'{measure} global value'], marker='v', color='tab:blue')
                if not args.error:
                    ax.plot(frame1[xvar], frame1['tp_upper'], color='tab:blue', alpha=0.4)
                    ax.plot(frame1[xvar], frame1['tp_lower'], color='tab:blue', alpha=0.4)
                    ax.fill_between(frame1[xvar], frame1['tp_upper'], frame1['tp_lower'], facecolor='tab:blue',
                                    alpha=0.4)
                if args.comparison:
                    ax.plot(frame2[xvar], frame2[f'{measure} global value'], marker='^', color='tab:blue',
                            linestyle='--')
                    if not args.error:
                        ax.plot(frame2[xvar], frame2['tp_upper'], color='tab:blue', alpha=0.4, linestyle='--')
                        ax.plot(frame2[xvar], frame2['tp_lower'], color='tab:blue', alpha=0.4, linestyle='--')
                        ax.fill_between(frame2[xvar], frame2['tp_upper'], frame2['tp_lower'],
                                        facecolor='tab:blue', alpha=0.4, linestyle='--')

                title = format_title(testcase, item)
                if args.comparison:
                    title += f': {xlsfp.label[0]} vs {xlsfp2.label[0]}'
                title = "{}\n{}".format(*splithalf(title))
                ax.set_title(title)
                ax.set_xlabel(xlabel)
                ax.set_ylabel(f'Throughput ({unit})')
                ax.grid('on', which='both', axis='x')
                ax.grid('on', which='major', axis='y')

                ax1 = ax.twinx()  # add second plot to the same axes, sharing x-axis
                ax1.plot(np.nan, marker='v', label=f'{measure}, global {xlsfp.label[1]}',
                         color='tab:blue')  # Make an agent in ax
                if args.comparison:
                    ax1.plot(np.nan, marker='^', label=f'{measure}, global {xlsfp2.label[1]}', color='tab:blue',
                             linestyle='--')  # Make an agent in ax

                ax1.plot(frame1[xvar], frame1['latency average value'], label=f'latency {xlsfp.label[1]}',
                         color='black', marker='p')
                if not args.error:
                    ax1.plot(np.nan, label=f'{measure} error', color='tab:blue', alpha=0.4)  # Make an agent in ax
                    ax1.plot(frame1[xvar], frame1['latency_upper'], label='latency error region', color='grey',
                             alpha=0.4)
                    ax1.plot(frame1[xvar], frame1['latency_lower'], color='grey', alpha=0.4)
                    ax1.fill_between(frame1[xvar], frame1['latency_upper'], frame1['latency_lower'],
                                     facecolor='grey', alpha=0.4)

                if args.comparison:
                    ax1.plot(frame2[xvar], frame2['latency average value'], label=f'latency {xlsfp2.label[1]}',
                             color='black', marker='*', linestyle='--')
                    if not args.error:
                        ax1.plot(frame2[xvar], frame2['latency_upper'], color='grey', alpha=0.4, linestyle='--')
                        ax1.plot(frame2[xvar], frame2['latency_lower'], color='grey', alpha=0.4, linestyle='--')
                        ax1.fill_between(frame2[xvar], frame2['latency_upper'], frame2['latency_lower'],
                                         facecolor='grey', alpha=0.4, linestyle='--')

                ax1.set_ylabel('Latency (ms)')
                ax1.legend(loc='lower right')

                # second subplot with tp per item
                ax2.plot(frame1[xvar], frame1[ycomparison.format(measure)], marker='+',
                         label=f'{measure}/{xvar} {xlsfp.label[1]}', color='tab:red')
                if not args.error:
                    ax2.plot(frame1[xvar], frame1['tp_xvar_upper'], color='tab:red',
                             label=f'{measure}/{xvar} error region', alpha=0.4)
                    ax2.plot(frame1[xvar], frame1['tp_xvar_lower'], color='tab:red', alpha=0.4)
                    ax2.fill_between(frame1[xvar], frame1['tp_xvar_upper'], frame1['tp_xvar_lower'],
                                     facecolor='tab:red', alpha=0.4)
                if args.comparison:
                    ax2.plot(frame2[xvar], frame2[ycomparison.format(measure)], marker='x',
                             label=f'{measure}/{xvar} {xlsfp2.label[1]}',
                             color='tab:red', linestyle='--')
                    if not args.error:
                        ax2.plot(frame2[xvar], frame2['tp_xvar_upper'], color='tab:red', alpha=0.4, linestyle='--')
                        ax2.plot(frame2[xvar], frame2['tp_xvar_lower'], color='tab:red', alpha=0.4, linestyle='--')
                        ax2.fill_between(frame2[xvar], frame2['tp_xvar_upper'], frame2['tp_xvar_lower'],
                                         facecolor='tab:red', alpha=0.4)

                ax2.set_xlabel(xlabel)
                ax2.set_ylabel(f'Throughput ({unit})')
                ax2.grid('on', which='both', axis='x')
                ax2.grid('on', which='major', axis='y')
                ax2.legend(loc='upper right')

                # add some regression lines
                def rline_throughput():
                    def throughput_model(z, a, b):
                        return a * z / (z + b)

                    popt, pcov = curve_fit(throughput_model, frame1['vector size'],
                                           frame1[f'{measure} global value'] / 100000)
                    x_tp = np.linspace(16, 2048, 1000)
                    y_tp = throughput_model(x_tp, *popt)
                    df_throughput_model = pd.DataFrame({'vector size': x_tp, 'model values': y_tp * 100000})
                    ax.plot(df_throughput_model['vector size'], df_throughput_model['model values'], marker=',',
                            color='tab:green', linestyle='--')
                    ax1.plot(np.nan, linestyle='--', color='tab:green',
                             label=r"""Throughput model: $y=\frac{{{}x}}{{x+{}}}$""".format(int(popt[0] * 100000),
                                                                                            int(popt[1])))

                def rline_latency():
                    def latency_model(z, a, b):
                        return a + z * b

                    popt1, pcov1 = curve_fit(latency_model, frame1['vector size'], frame1['latency average value'])
                    x_lt = np.linspace(16, 2048, 100)
                    y_lt = latency_model(x_lt, *popt1)
                    df_latency_model = pd.DataFrame({'vector size': x_lt, 'model values': y_lt})
                    a, b = '{0:.3f}'.format(popt1[0]), '{0:.3f}'.format(popt1[1])
                    ax1.plot(df_latency_model['vector size'], df_latency_model['model values'], marker=',',
                             color='orange', linestyle='dashdot', label=r'Latency model: $y={}+{}x$'.format(a, b))
                    ax1.legend(loc='lower right')

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
    parser.add_argument('xls', metavar='FILE', type=argparse.FileType('rb'), help='Path to Excel spreadsheet')
    parser.add_argument('-t', '--table', help='Table name', default='Sheet1')
    parser.add_argument('-e', '--error', help='Remove error regions from plot', action='store_true')

    parser.add_argument('-c', '--comparison',
                        help='Compare two datasets. Provide the path to a second Excel spreadsheet.', metavar='FILE',
                        type=argparse.FileType('rb'))

    subparsers = parser.add_subparsers(dest='indvar')
    size = subparsers.add_parser('size',
                                 help='''Set vector size as independent variable.''')
    size.add_argument('--reglines',
                      help='add lines of best fit for latency and throughput using predefined mathematical model',
                      action='store_true')
    threads = subparsers.add_parser('threads', help='''Set number of threads as independent variable.''')
    parser.add_argument('-l', '--labels', help='Dataset labels (defaults to "data set 1" and "data set 2")', nargs=2)

    args = parser.parse_args()

    if args.indvar is None:
        args.indvar = 'threads'

    params = {'threads':
                  ('vector size', 'threads', '# of Threads', '{} thread value', 'vec', '{} thread value', format_title1),
              'size':
                  ('threads', 'vector size', 'Vector Size (Bytes)', '{} per vector size', 'threads', '{} per vector size',
                   format_title2)}
    graph_parameter, xvar, xlabel, ycomparison, fnsub, col3name, format_title = params[args.indvar]

    if not hasattr(args, 'comparison'):
        args.comparison = False
    generate_graphs(args.xls, args.table, args.comparison)
