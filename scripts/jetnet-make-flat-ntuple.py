#!/usr/bin/env python2.7

"""
runs full chain on <file list>

options set in 'jetnet.cfg'

"""

# hide this godawful abomination of a framework
import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True


from jetnet import training, pyprep, profile, rds, process
from jetnet.perf import rejection, performance
import os, sys
from warnings import warn

from argparse import ArgumentParser
from ConfigParser import SafeConfigParser

class UglyWarning(UserWarning): pass

_default_observers = [ 
    'IP3D', 
    'SV1', 
    'JetFitterCOMBNN', 
    'MV1', 
    'MV2', 
    ]

def make_flat_ntuple(
    input_files, 
    pt_divisions, 
    weight_file = '', 
    jet_collection = 'BTag_AntiKt4TopoEMJetsReTagged', 
    jet_tagger = 'JetFitterCharm', 
    output_path = None, 
    rds_path = 'reduced_dataset.root', 
    observer_discriminators = _default_observers, 
    do_test = False, 
    skim_function = pyprep.make_flat_ntuple, 
    ): 
    args = locals()             # may be needed for recursive calls

    double_variables, int_variables = rds.get_allowed_rds_variables(
        input_files = input_files, 
        full_dir_name = '_'.join([jet_collection,jet_tagger]))

    # --- make weights if a name is given 
    if weight_file and not os.path.isfile(weight_file): 
        
        # build a light ntuple if one doesn't exist
        if os.path.isfile(rds_path): 
            small_rds_path = rds_path 

        else: 
            print 'making flat ntuple to build weight file'

            rds_dir, rds_name = os.path.split(rds_path)
            small_rds = '.'.join(rds_name.split('.')[:-1]) + '_small.root'
            small_rds_path = os.path.join(rds_dir,small_rds)
            if not os.path.isfile(small_rds_path): 
                pyprep.make_flat_ntuple(
                    input_files = input_files, 
                    jet_collection = jet_collection, 
                    jet_tagger = jet_tagger, 
                    output_file = small_rds_path)
            

        from jetnet import cxxprofile
        cxxprofile.pro2d(
            in_file = small_rds_path, 
            tree = 'SVTree', 
            plots = [( ('JetPt', 30,15.0,200),
                       ('JetEta',10,-2.5,2.5) )], 
            tags = ['bottom','charm','light'], 
            out_file = weight_file, 
            show_progress = True)

    # --- rds part

    rds_dir, rds_file = os.path.split(rds_path)
    if rds_dir and not os.path.isdir(rds_dir): 
        os.mkdir(rds_dir)

    if os.path.isfile(rds_path): 
        raise IOError(
            "{} already exists, refusing to overwrite".format(rds_path) )
    else: 
        skim_function(
            input_files = input_files, 
            weight_file = weight_file, 
            double_variables = double_variables, 
            int_variables = int_variables, 
            observer_discriminators = observer_discriminators, 
            pt_divisions = pt_divisions, 
            jet_collection = jet_collection, 
            jet_tagger = jet_tagger, 
            output_file = rds_path, 
            debug = do_test, 
            )

            
                         

    
if __name__ == '__main__': 

    description = __doc__
    epilog = 'Author: Daniel Guest (dguest@cern.ch)'

    parser = ArgumentParser(description = description, 
                            epilog = epilog)
    parser.set_defaults(
        test = False, 
        )

    parser.add_argument('input_files', nargs = 1)
    parser.add_argument('output_file', default = 'flat_dataset.root', 
                        help = 'default: %(default)s', nargs = '?')
    parser.add_argument('--test', action = 'store_true')
    parser.add_argument(
        '--binned', action = 'store_true', 
        help = 'make ntuple binned in pt and eta')
    parser.add_argument('--config', 
                        help = 'use this configuration file'
                        ' (default: %(default)s)', 
                        default = 'jetnet.cfg')

    options = parser.parse_args(sys.argv[1:])

    do_test = options.test

    if options.binned: 
        skim_function = pyprep.prep_ntuple
    else: 
        skim_function = pyprep.make_flat_ntuple

    rds_path = options.output_file

    config_file_name = options.config
    flavor_weights = {}
    if os.path.isfile(config_file_name): 
        config = SafeConfigParser(
            {'flavor_wt_file':''})
        config.read(config_file_name)
        pt_divisions = map(
            float,config.get('preprocessing','pt_divisions').split())

        observer_discriminators = config.get(
            'preprocessing','observer_discriminators').split()
        jet_tagger = config.get('preprocessing','jet_tagger')
        jet_collection = config.get('preprocessing','jet_collection')
        flavor_wt_file = config.get('caching','flavor_wt_file')
        if 'ARRAYID' in jet_tagger: 
            the_array_id = os.environ['PBS_ARRAYID'].rjust(2,'0')
            jet_tagger = jet_tagger.replace('ARRAYID',the_array_id)
            rds_path = os.path.join(jet_tagger,rds_path)
    else: 
        sys.exit('could not find config file %s' % config_file_name)


    input_files = []
    with open(options.input_files[0]) as file_list: 
        for line in file_list: 
            input_files.append(line.strip())

    print 'making flat ntuple' 
    make_flat_ntuple(
        input_files, 
        weight_file = flavor_wt_file, 
        do_test = do_test, 
        observer_discriminators = observer_discriminators, 
        jet_collection = jet_collection, 
        jet_tagger = jet_tagger, 
        rds_path = rds_path, 
        pt_divisions = pt_divisions, 
        skim_function = skim_function)
