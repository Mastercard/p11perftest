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
import signal
import sys
import os
import multiprocessing as mp
import pandas as pd
import matplotlib
matplotlib.use('Agg')  # use a non-interactive backend
import matplotlib.pyplot as plt
from matplotlib.ticker import MaxNLocator
import numpy as np
from scipy.optimize import curve_fit

# Global variables for multiprocess-safe file tracking
manager = mp.Manager()
current_files = manager.list()
files_lock = manager.Lock()
graphs_created = manager.Value('i', 0)  # Shared counter for created graphs


def signal_handler(_sig, _frame):
    """Handle CTRL-C gracefully by cleaning up incomplete files"""
    print('\n\nInterrupted! Cleaning up incomplete files...', file=sys.stderr)
    with files_lock:
        files_to_remove = list(current_files)
        created_count = graphs_created.value
    for filepath in files_to_remove:
        if os.path.exists(filepath):
            try:
                os.remove(filepath)
                print(f'Removed incomplete file: {filepath}', file=sys.stderr)
            except OSError as e:
                print(f'Failed to remove {filepath}: {e}', file=sys.stderr)
    print(f'\nTotal graphs created before interruption: {created_count}', file=sys.stderr)
    print('Exiting.', file=sys.stderr)
    sys.exit(0)


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
    """Format title for vector size."""
    if str(s2)[0] == '8':
        return f"{s1} on an {s2} Bytes Vector"
    else:
        return f"{s1} on a {s2} Bytes Vector"


def format_title2(s1, s2):
    """Format title for threads."""
    if s2 == 1:
        return f"{s1} on {s2} Thread"
    else:
        return f"{s1} on {s2} Threads"

 
def create_dataframe(xls, sheetname, xvar):
    """create a dataframe from an excel file; are we interested in throughput or transactions?"""    
    df = pd.read_excel(xls, sheet_name=sheetname)
    df.sort_values(by=[xvar])
    return df


def determine_measure(testcase, col3name):
    """determine if we are measuring TPS or throughput based on test case name"""    
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


def create_graph_frame(df, testcase, item, xvar, graph_parameter, col3name, p95, p98, p99):
    measure, unit, col2, col3 = determine_measure(testcase, col3name)
    frame = df.loc[(df['test case'] == testcase) & (df[graph_parameter] == item),
                   [xvar, 'latency average value', col2]]
    frame['tp_upper'] = frame[col2] + df[f'{measure} global error']
    frame['tp_lower'] = frame[col2] - df[f'{measure} global error']
    frame['tp_lower'] = frame['tp_lower'].map(lambda x: max(x, 0))
    frame['latency_upper'] = frame['latency average value'] + df['latency average error']
    frame['latency_lower'] = frame['latency average value'] - df['latency average error']
    frame['latency_lower'] = frame['latency_lower'].map(lambda x: max(x, 0))

    frame[col3] = frame[col2] / frame[xvar]
    frame['tp_xvar_upper'] = frame[col3] + df[f'{measure} global error'] / frame[xvar]
    frame['tp_xvar_lower'] = frame[col3] - df[f'{measure} global error'] / frame[xvar]
    frame['tp_xvar_lower'] = frame['tp_xvar_lower'].map(lambda x: max(x, 0))

    percentiles_available = True
    if p95 or p98 or p99:
        try:
            frame['p95'] = df['latency p95 value']
            frame['p98'] = df['latency p98 value']
            frame['p99'] = df['latency p99 value']
        except KeyError:
            print("\n\nPercentiles not present in the spreadsheet, ignoring the percentiles flag.\n\n")
            percentiles_available = False

    return frame, measure, unit, col2, col3, percentiles_available


def _create_single_graph_worker(df1, df2, testcase, item, params):
    """Global worker function for multiprocessing.
    
    This needs to be a module-level function (not a method) so it can be pickled.
    """
    print(f"Drawing graph for {testcase} and {params['graph_parameter']} {item}...", end='')
    frame1, measure, unit, col2, col3, p_avail = create_graph_frame(
        df1, testcase, item, params['xvar'], params['graph_parameter'], 
        params['col3name'], params['p95'], params['p98'], params['p99'])
    frame2 = None
    if params['comparison']:
        frame2, measure2, _, _, _, _ = create_graph_frame(
            df2, testcase, item, params['xvar'], params['graph_parameter'], 
            params['col3name'], params['p95'], params['p98'], params['p99'])

    fig, (ax, ax2) = plt.subplots(2, figsize=(16, 16), height_ratios=(3, 1))

    ax.plot(frame1[params['xvar']], frame1[f'{measure} global value'], marker='v', color='tab:blue')
    if not params['no_error_region']:
        ax.plot(frame1[params['xvar']], frame1['tp_upper'], color='tab:blue', alpha=0.4)
        ax.plot(frame1[params['xvar']], frame1['tp_lower'], color='tab:blue', alpha=0.4)
        ax.fill_between(frame1[params['xvar']], frame1['tp_upper'], frame1['tp_lower'], facecolor='tab:blue',
                        alpha=0.4)
                            
    if params['comparison']:
        ax.plot(frame2[params['xvar']], frame2[f'{measure} global value'], marker='^', color='tab:blue',
                linestyle='--')
        if not params['no_error_region']:
            ax.plot(frame2[params['xvar']], frame2['tp_upper'], color='tab:blue', alpha=0.4, linestyle='--')
            ax.plot(frame2[params['xvar']], frame2['tp_lower'], color='tab:blue', alpha=0.4, linestyle='--')
            ax.fill_between(frame2[params['xvar']], frame2['tp_upper'], frame2['tp_lower'],
                            facecolor='tab:blue', alpha=0.4, linestyle='--')

    title = params['format_title'](testcase, item)
    if params['comparison']:
        title += f': {params["xlsfp_label"][0]} vs {params["xlsfp2_label"][0]}'
    title = "{}\n{}".format(*splithalf(title))
    ax.set_title(title)
    ax.set_xlabel(params['xlabel'])
    ax.set_ylabel(f'Throughput ({unit})', color='tab:blue')
    ax.tick_params(axis='y', labelcolor='tab:blue')
    ax.xaxis.set_major_locator(MaxNLocator(integer=True))
    ax.grid('on', which='both', axis='x')
    ax.grid('on', which='major', axis='y', linestyle='-', color='tab:blue', alpha=0.3)

    ax1 = ax.twinx()  # add second plot to the same axes, sharing x-axis
    ax1.plot(np.nan, marker='v', label=f'{measure}, global {params["xlsfp_label"][1]}',
             color='tab:blue')  # Make an agent in ax
    if params['comparison']:
        ax1.plot(np.nan, marker='^', label=f'{measure}, global {params["xlsfp2_label"][1]}', color='tab:blue',
                 linestyle='--')  # Make an agent in ax

    ax1.plot(frame1[params['xvar']], frame1['latency average value'], label=f'latency average {params["xlsfp_label"][1]}',
             color='black', marker='p')
    
    # Add horizontal dash-dot grid lines for latency average axis
    ax1.grid(True, which='major', axis='y', linestyle='-.', color='black', alpha=0.3, zorder=2)
    
    
    # Create a third y-axis for percentiles if they are enabled
    if params['p95'] or params['p98'] or params['p99']:
        ax_percentiles = ax.twinx()
        # Offset the third axis to the right
        ax_percentiles.spines['right'].set_position(('axes', 1.1))
        
        if params['p95']:
            ax_percentiles.plot(frame1[params['xvar']], frame1['p95'], color='plum', alpha=1.0, label=f'latency p95 {params["xlsfp_label"][1]}', marker='1', zorder=5)
            ax_percentiles.fill_between(frame1[params['xvar']], frame1['p95'], facecolor='palegreen', alpha=0.2, zorder=1)
        if params['p98']:
            ax_percentiles.plot(frame1[params['xvar']], frame1['p98'], color='mediumorchid', alpha=1.0, label=f'latency p98 {params["xlsfp_label"][1]}', marker='2', zorder=5)
            ax_percentiles.fill_between(frame1[params['xvar']], frame1['p98'], facecolor='lightgreen', alpha=0.2, zorder=1)
        if params['p99']:
            ax_percentiles.plot(frame1[params['xvar']], frame1['p99'], color='darkorchid', alpha=1.0, label=f'latency p99 {params["xlsfp_label"][1]}', marker='3', zorder=5)
            ax_percentiles.fill_between(frame1[params['xvar']], frame1['p99'], facecolor='limegreen', alpha=0.2, zorder=1)
        
        ax_percentiles.set_ylabel('Latency Percentiles (ms)', color='darkviolet')
        ax_percentiles.tick_params(axis='y', labelcolor='darkviolet')
        
        # Add horizontal dashed grid lines for percentile axis
        ax_percentiles.grid(True, which='major', axis='y', linestyle='--', color='darkviolet', alpha=0.3, zorder=2)

    if not params['no_error_region']:
        ax1.plot(np.nan, label=f'{measure} error', color='tab:blue', alpha=0.4)  # Make an agent in ax
        ax1.plot(frame1[params['xvar']], frame1['latency_upper'], label='latency error region', color='grey',
                 alpha=0.4)
        ax1.plot(frame1[params['xvar']], frame1['latency_lower'], color='grey', alpha=0.4)
        ax1.fill_between(frame1[params['xvar']], frame1['latency_upper'], frame1['latency_lower'],
                         facecolor='grey', alpha=0.4)

    if params['comparison']:
        ax1.plot(frame2[params['xvar']], frame2['latency average value'], label=f'latency {params["xlsfp2_label"][1]}',
                 color='black', marker='*', linestyle='--')
        if not params['no_error_region']:
            ax1.plot(frame2[params['xvar']], frame2['latency_upper'], color='grey', alpha=0.4, linestyle='--')
            ax1.plot(frame2[params['xvar']], frame2['latency_lower'], color='grey', alpha=0.4, linestyle='--')
            ax1.fill_between(frame2[params['xvar']], frame2['latency_upper'], frame2['latency_lower'],
                             facecolor='grey', alpha=0.4, linestyle='--')

    ax1.set_ylabel('Latency Average (ms)')
    
    # Set x-axis and y-axis limits to remove margins
    ax.set_xlim(left=frame1[params['xvar']].min(), right=frame1[params['xvar']].max())
    ax.set_ylim(bottom=0)
    ax1.set_ylim(bottom=0)
    if params['p95'] or params['p98'] or params['p99']:
        ax_percentiles.set_ylim(bottom=0)
    
    # Merge legends from all axes and attach to the topmost axis
    handles1, labels1 = ax1.get_legend_handles_labels()
    if params['p95'] or params['p98'] or params['p99']:
        handles_percentiles, labels_percentiles = ax_percentiles.get_legend_handles_labels()
        handles1 += handles_percentiles
        labels1 += labels_percentiles
        # Attach legend to ax_percentiles (the topmost axis) instead of ax1
        legend = ax_percentiles.legend(handles1, labels1, loc='lower right', fancybox=False, framealpha=1.0, facecolor='white')
    else:
        legend = ax1.legend(handles1, labels1, loc='lower right', fancybox=False, framealpha=1.0, facecolor='white')
    legend.set_zorder(100)

    # second subplot with tp per item
    if params['indvar'] == 'threads':
        label = f'{measure}/{params["xvar"]} {params["xlsfp_label"][1]}'
    if params['indvar'] == 'size':
        label = 'transactions'

    ax2.plot(frame1[params['xvar']], frame1[params['ycomparison'].format(measure)], marker='+',
             label=label, color='tab:red')

    if not params['no_error_region']:
        if params['indvar'] == 'threads':
            label = f'{measure}/{params["xvar"]} error region'
        if params['indvar'] == 'size':
            label = 'transactions error region'
        ax2.plot(frame1[params['xvar']], frame1['tp_xvar_upper'], color='tab:red',
                 label=label, alpha=0.4)
        ax2.plot(frame1[params['xvar']], frame1['tp_xvar_lower'], color='tab:red', alpha=0.4)
        ax2.fill_between(frame1[params['xvar']], frame1['tp_xvar_upper'], frame1['tp_xvar_lower'],
                         facecolor='tab:red', alpha=0.4)
    if params['comparison']:
        ax2.plot(frame2[params['xvar']], frame2[params['ycomparison'].format(measure)], marker='x',
                 label=f'{measure}/{params["xvar"]} {params["xlsfp2_label"][1]}',
                 color='tab:red', linestyle='--')
        if not params['no_error_region']:
            ax2.plot(frame2[params['xvar']], frame2['tp_xvar_upper'], color='tab:red', alpha=0.4, linestyle='--')
            ax2.plot(frame2[params['xvar']], frame2['tp_xvar_lower'], color='tab:red', alpha=0.4, linestyle='--')
            ax2.fill_between(frame2[params['xvar']], frame2['tp_xvar_upper'], frame2['tp_xvar_lower'],
                             facecolor='tab:red', alpha=0.4)

    ax2.set_xlabel(params['xlabel'])
    if params['indvar'] == 'threads':
        ax2.set_ylabel(f'Throughput ({unit})')
    if params['indvar'] == 'size':
        ax2.set_ylabel('Transactions/s')
    ax2.xaxis.set_major_locator(MaxNLocator(integer=True))
    ax2.set_xlim(left=frame1[params['xvar']].min(), right=frame1[params['xvar']].max())
    ax2.set_ylim(bottom=0)
    ax2.grid('on', which='both', axis='x')
    ax2.grid('on', which='major', axis='y')
    ax2.legend(loc='upper right', fancybox=False, framealpha=1.0, facecolor='white')

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

    if params['reglines']:
        rline_throughput()
        rline_latency()

    plt.tight_layout()
    filename = testcase.lower().replace(' ', '_')
    
    # Track files being created with lock protection
    files_being_saved = []
    
    if 'svg' in params['output_format'] or 'all' in params['output_format']:
        svg_file = os.path.join(params['output_dir'], f'{filename}-{params["fnsub"]}{item}.svg')
        files_being_saved.append(svg_file)
    if 'png' in params['output_format'] or 'all' in params['output_format']:
        png_file = os.path.join(params['output_dir'], f'{filename}-{params["fnsub"]}{item}.png')
        files_being_saved.append(png_file)
    
    # Add files to tracking list with lock
    with files_lock:
        current_files.extend(files_being_saved)
    
    # Save files
    for file_path in files_being_saved:
        if file_path.endswith('.svg'):
            plt.savefig(file_path, format='svg', orientation='landscape')
        elif file_path.endswith('.png'):
            plt.savefig(file_path, format='png', orientation='landscape')
    
    # Remove files from tracking list after successful save
    with files_lock:
        for f in files_being_saved:
            if f in current_files:
                current_files.remove(f)
        # Increment counter of successfully created graphs
        graphs_created.value += 1
    
    plt.cla()
    plt.close(fig)
    print('OK', flush=True)


class GraphGenerator:
    """Class to encapsulate graph generation logic and parameters."""
    
    def __init__(self, xlsfp, sheetname, xlsfp2, xvar, graph_parameter, xlabel, ycomparison, fnsub, col3name, format_title,
                 comparison, labels, no_error_region, p95, p98, p99, output_format, indvar, reglines, output_dir='.'):
        self.xlsfp = xlsfp
        self.sheetname = sheetname
        self.xlsfp2 = xlsfp2
        self.xvar = xvar
        self.graph_parameter = graph_parameter
        self.xlabel = xlabel
        self.ycomparison = ycomparison
        self.fnsub = fnsub
        self.col3name = col3name
        self.format_title = format_title
        self.comparison = comparison
        self.labels = labels
        self.no_error_region = no_error_region
        self.p95 = p95
        self.p98 = p98
        self.p99 = p99
        self.output_format = output_format
        self.indvar = indvar
        self.reglines = reglines
        self.output_dir = output_dir
        
        self._setup_comparison_labels()
    
    def _setup_comparison_labels(self):
        """Setup labels for comparison mode."""
        if not self.comparison:
            self.xlsfp.label = '', ''
            if self.labels is not None:
                print('Not in comparison mode, ignoring labels. Did you forget to specify -c flag?')
        else:
            if self.labels is None:
                self.xlsfp.label = 'data set 1', '(data set 1)'
                self.xlsfp2.label = 'data set 2', '(data set 2)'
            else:
                self.xlsfp.label = self.labels[0], f'({self.labels[0]})'
                self.xlsfp2.label = self.labels[1], f'({self.labels[1]})'
    
    def generate(self, num_processes=None):
        """Generate all graphs using multiprocessing.
        
        Args:
            num_processes: Number of parallel processes to use. If None, uses CPU count.
        """
        xls_tuple = (self.xlsfp, self.xlsfp)
        if self.comparison:
            xls_tuple = (self.xlsfp, self.xlsfp2)

        with xls_tuple[0], xls_tuple[1]:
            # read from spreadsheet directly
            df1 = create_dataframe(self.xlsfp, self.sheetname, self.xvar)
            df2 = None
            if self.comparison:
                df2 = create_dataframe(self.xlsfp2, 'Sheet1', self.xvar)
            ### could reintroduce this logic below. removed for now...
            #     if not (measure1 == measure2) and (df1[graph_parameter].unique() == df2[graph_parameter].unique()):
            #         raise AssertionError('Please compare similar things.')
            #     measure = measure1
            # else:
            #     measure = measure1

            # Build list of all tasks (testcase, item) pairs
            tasks = []
            for testcase in df1["test case"].unique():
                for item in sorted(df1.loc[(df1['test case'] == testcase)][self.graph_parameter].unique()):
                    tasks.append((testcase, item))
            
            # Process tasks in parallel
            if num_processes == 1:
                # Serial execution for debugging
                for testcase, item in tasks:
                    self._create_single_graph(df1, df2, testcase, item)
            else:
                # Parallel execution - extract parameters needed by workers
                worker_params = {
                    'xvar': self.xvar,
                    'graph_parameter': self.graph_parameter,
                    'xlabel': self.xlabel,
                    'ycomparison': self.ycomparison,
                    'fnsub': self.fnsub,
                    'col3name': self.col3name,
                    'format_title': self.format_title,
                    'comparison': self.comparison,
                    'xlsfp_label': self.xlsfp.label,
                    'xlsfp2_label': self.xlsfp2.label if self.comparison else None,
                    'no_error_region': self.no_error_region,
                    'p95': self.p95,
                    'p98': self.p98,
                    'p99': self.p99,
                    'output_format': self.output_format,
                    'indvar': self.indvar,
                    'reglines': self.reglines,
                    'output_dir': self.output_dir
                }
                
                with mp.Pool(processes=num_processes) as pool:
                    pool.starmap(_create_single_graph_worker, 
                                [(df1, df2, testcase, item, worker_params) for testcase, item in tasks])
    
    def _create_single_graph(self, df1, df2, testcase, item):
        """Create and save a single graph for a specific testcase and item (serial execution)."""
        # Build params dict and call the worker function
        worker_params = {
            'xvar': self.xvar,
            'graph_parameter': self.graph_parameter,
            'xlabel': self.xlabel,
            'ycomparison': self.ycomparison,
            'fnsub': self.fnsub,
            'col3name': self.col3name,
            'format_title': self.format_title,
            'comparison': self.comparison,
            'xlsfp_label': self.xlsfp.label,
            'xlsfp2_label': self.xlsfp2.label if self.comparison else None,
            'no_error_region': self.no_error_region,
            'p95': self.p95,
            'p98': self.p98,
            'p99': self.p99,
            'output_format': self.output_format,
            'indvar': self.indvar,
            'reglines': self.reglines,
            'output_dir': self.output_dir
        }
        _create_single_graph_worker(df1, df2, testcase, item, worker_params)


if __name__ == '__main__':
    # Register signal handler for CTRL-C
    signal.signal(signal.SIGINT, signal_handler)
    
    parser = argparse.ArgumentParser(description='Generate graphs from spreadsheet of p11perftest results')
    parser.add_argument('xls', metavar='FILE', type=argparse.FileType('rb'), help='Path to Excel spreadsheet')
    parser.add_argument('-t', '--table', help='Table name.', default=0)
    parser.add_argument('-f', '--format', help='Output format. Defaults to all (png and svg).', choices=['png', 'svg', 'all'], default='all')
    parser.add_argument('-j', '--jobs', type=int, metavar='N', help='Number of parallel jobs to run. Default is CPU count. Use 1 for serial execution.')
    parser.add_argument('-d', '--directory', metavar='DIR', help='Directory where graphs will be saved. Must exist and be writable. Defaults to current directory.', default='.')
    
    parser.add_argument('-p', '--percentiles', help='Display percentile plots on graph. Equivalent to --p95 --p98 --p99.', action='store_true')
    parser.add_argument('--p95', help='Display 95th percentile plot on graph.', action='store_true')
    parser.add_argument('--p98', help='Display 98th percentile plot on graph.', action='store_true')
    parser.add_argument('--p99', help='Display 99th percentile plot on graph.', action='store_true')

    parser.add_argument('--no-error-region', help='Remove error regions from plot.', action='store_true')

    parser.add_argument('-c', '--comparison',
                        help='Compare two datasets. Provide the path to a second Excel spreadsheet.', metavar='FILE',
                        type=argparse.FileType('rb'))

    subparsers = parser.add_subparsers(dest='indvar')
    size = subparsers.add_parser('size',
                                 help='''Set vector size as independent variable.''')
    size.add_argument('--reglines',
                      help='Add lines of best fit for latency and throughput using predefined mathematical model.',
                      action='store_true')
    threads = subparsers.add_parser('threads', help='Set number of threads as independent variable.')
    parser.add_argument('-l', '--labels', metavar=('LABEL1', 'LABEL2'), help='Dataset labels. Defaults to "data set 1" and "data set 2".', nargs=2)
    
    args = parser.parse_args()

    # Validate output directory
    if not os.path.exists(args.directory):
        print(f"Error: Directory '{args.directory}' does not exist.", file=sys.stderr)
        sys.exit(1)
    if not os.path.isdir(args.directory):
        print(f"Error: '{args.directory}' is not a directory.", file=sys.stderr)
        sys.exit(1)
    if not os.access(args.directory, os.W_OK):
        print(f"Error: Directory '{args.directory}' is not writable.", file=sys.stderr)
        sys.exit(1)

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

    if args.percentiles:
        args.p95, args.p98, args.p99 = True, True, True

    # Create graph generator and generate all graphs
    generator = GraphGenerator(args.xls, args.table, args.comparison, 
                              xvar, graph_parameter, xlabel, ycomparison, fnsub, col3name, format_title,
                              args.comparison, args.labels, args.no_error_region, 
                              args.p95, args.p98, args.p99, args.format, args.indvar, 
                              getattr(args, 'reglines', False), args.directory)
    generator.generate(num_processes=getattr(args, 'jobs', None))
    
    # Print summary
    print(f'\nTotal graphs created: {graphs_created.value}')
