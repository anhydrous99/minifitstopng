from joblib import Parallel, delayed
from tqdm import tqdm
import subprocess
import argparse
import stk


def run_minifitstopng(evt3, output, scale):
    subprocess.run([f"./minifitstopng --evt3 \"{evt3}\" --output \"{output}\" --scale \"{scale}\""],
                   shell=True, check=True)


def main(evt3_files, output, scale, jobs):
    Parallel(n_jobs=jobs)(delayed(run_minifitstopng)(evt3, output, scale) for evt3 in tqdm(evt3_files))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('evt3', type=stk.build)
    parser.add_argument('output')
    parser.add_argument('-s', '--scale', default='log')
    parser.add_argument('-j', '--jobs', default=1, type=int)
    args = parser.parse_args()
    main(args.evt3, args.output, args.scale, args.jobs)
