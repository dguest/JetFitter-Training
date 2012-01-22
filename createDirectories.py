#!/usr/bin/env python2.6

from jetnet import dirs
import ConfigParser, glob, os, sys

def make_directories(config_file = 'training.config', 
                     header_script = 'omega_scripts/run_header.sh', 
                     input_data_list = None): 
    """
    returns a list of directories which were created 
    """

    methods = ['JetNet'] #, 'LikelihoodHistos'] 
    header = 'omega_scripts/run_header.sh'

    dirs_created = []

    conf = ConfigParser.ConfigParser()
    conf.read(config_file) 
    collections_to_process = conf.get('collections', 'process').split()

    script_builder = dirs.ScriptBuilder(header_script = header, 
                                        executable = 'doTraining.py', 
                                        overwrite = True)

    for m in methods: 
        method_dir_name = 'trainingResults' + m

        for jet_dir_name in collections_to_process: 

            for input_data in input_data_list: 
                rel_dir_path = os.path.join(
                    method_dir_name,
                    jet_dir_name, 
                    input_data
                    ) 
                input_ds = glob.glob('reduceddatasets/%s/*.root' % input_data)
                if len(input_ds) != 1: 
                    print input_ds
                    sys.exit('those ds were a problem')
                else: 
                    (input_ds,) = input_ds
                script_builder.build_script(rel_dir_path, input_ds)


if __name__ == '__main__': 
    make_directories(input_data_list = ['dan','giacinto'])
