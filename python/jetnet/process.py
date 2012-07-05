# Author: Daniel Guest (dguest@cern.ch)

# hide this godawful abomination of a framework
import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True
ROOT.gROOT.SetBatch()           # don't draw (crashes subprocess)

from jetnet import training, pyprep, profile, utils
from jetnet.perf import rejection, performance
import os, sys, glob, re, itertools
from os.path import isdir, isfile, splitext, basename
from warnings import warn
import multiprocessing, ConfigParser

class RDSProcess(multiprocessing.Process): 
    def __init__(self, 
                 reduced_dataset, 
                 working_dir, 
                 training_variables, 
                 flavor_weights, 
                 testing_dataset = None,
                 do_more_diagnostics = None, 
                 do_test = False, 
                 config_file = None): 
        super(RDSProcess,self).__init__()

        self._reduced_dataset = reduced_dataset
        self._working_dir = working_dir
        self._training_variables = training_variables
        self._do_test = do_test
        self._flavor_weights = flavor_weights
        self._testing_ds = testing_dataset
        self._do_more_diagnostics = do_more_diagnostics

        # these things are set by the config file
        self._training_subdir = 'training'
        self._testing_subdir = 'testing'
        self._nodes = None
        self._n_training_events = 1000000
        self._other_opt_dict = {}
        self._profile_extension = 'NewTune'

        if config_file: 
            parser = ConfigParser.SafeConfigParser()
            parser.read(config_file)
            training_opt = dict(parser.items('training'))
            try: 
                self._nodes = [int(i) for i in training_opt['nodes'].split()]

            except KeyError: 
                warn("'nodes' not found in {}".format(config_file), 
                     stacklevel = 2)


            try: 
                self._n_training_events = int(training_opt['n_events'])
            except KeyError: 
                war_str = ("'n_events' not found in {} [training], " 
                           "defaulting to {}")
                warn(war_str.format(config_file, self._n_training_events), 
                     FutureWarning, stacklevel = 2)

            allowed_others =  {
                "n_patterns_per_update", 
                "learning_rate", 
                "learning_rate_decrease"}
            
            other_opt_dict = {}
            for opt_name, opt_val in training_opt.iteritems(): 
                if opt_name in allowed_others: 
                    if opt_name[0:2] == 'n_': 
                        converter = int
                    else: 
                        converter = float
                    other_opt_dict[opt_name] = converter(opt_val)
            self._other_opt_dict = other_opt_dict

            cfg_basename = splitext(basename(config_file))[0]
            if 'train_dir' in training_opt: 
                self._training_subdir = training_opt['train_dir']
            else: 
                self._training_subdir = 'training_' + cfg_basename
                
            if parser.has_option('testing','test_dir'): 
                self._testing_subdir = parser.get('testing','test_dir')
            else: 
                self._testing_subdir = 'testing_' + cfg_basename

        if not self._nodes: 
            warn('\'nodes\' list should be given in the config file '
                 'under [training]', 
                 FutureWarning, stacklevel = 2)
            self._nodes = [20,10]


        self.out_queue = multiprocessing.Queue()

    def run(self): 

        reduced_dataset = self._reduced_dataset
        working_dir = self._working_dir
        training_variables = self._training_variables
        do_test = self._do_test

        # --- profiling 
        profile_dir = os.path.join(working_dir,'profile')
        if not os.path.isdir(profile_dir): 
            os.mkdir(profile_dir)

        profile_file = os.path.join(profile_dir, 'profiled.root')
        if not os.path.isfile(profile_file):
            rds_dir = os.path.split(reduced_dataset)[0]
            alt_profile_file = os.path.join(rds_dir, 'profiled.root')
            if os.path.isfile(alt_profile_file): 
                profile_file = alt_profile_file

        mean_rms_file = os.path.join(profile_dir, 'mean_rms.txt')
        if not do_test: 
            if not os.path.isfile(mean_rms_file): 
                if not os.path.isfile(profile_file): 
                    print '--- making profile file for normalization ---'
                    profile.make_profile_file(reduced_dataset, profile_file)
            
                profile.build_mean_rms_from_profile(
                    profile_file = profile_file, 
                    text_file_name = mean_rms_file)

        # --- training part 
        training_dir = os.path.join(working_dir,self._training_subdir)
        if not os.path.isdir(training_dir): 
            os.mkdir(training_dir)
    
        normalization_file = os.path.join(training_dir, 'normalization.txt')
        if not os.path.isfile(normalization_file) and not do_test: 
            if not os.path.isfile(profile_file): 
                profile.make_profile_file(reduced_dataset, profile_file)
    
            profile.make_normalization_file(
                profile_file, 
                normalization_file = normalization_file, 
                whitelist = training_variables)
                                            
        normalization_dict = {}
        if os.path.isfile(normalization_file): 
            with open(normalization_file) as norm_file: 
                for line in norm_file: 
                    line = line.strip()
                    if not line: continue
                    name = line.split()[0]
                    offset, scale = (float(n) for n in line.split()[1:])
                    normalization_dict[name] = (offset, scale)
    
        print 'normalization:'
        text_size = max(len(x) for x in normalization_dict.keys()) + 1
        for value, (offset, scale) in normalization_dict.iteritems(): 
            print '%-*s offset: % -10.4g scale: % -10.4g' % (
                text_size, value, offset, scale)
    
        weights_path = os.path.join(training_dir, 'weightMinimum.root')
        if not os.path.isfile(weights_path): 
            print '--- running training ---'
            training.run_training(reduced_dataset = reduced_dataset, 
                                  output_directory = training_dir, 
                                  normalization = normalization_dict, 
                                  flavor_weights = self._flavor_weights, 
                                  nodes = self._nodes, 
                                  debug = do_test, 
                                  events = self._n_training_events, 
                                  other_opt_dict = self._other_opt_dict)
    
        # --- diagnostics part 
        testing_dir = os.path.join(working_dir, self._testing_subdir)
        if not os.path.isdir(testing_dir): 
            os.mkdir(testing_dir)
    

        if not self._testing_ds: 
            self._testing_ds = reduced_dataset

        augmented_tree = os.path.join(testing_dir, 'perf_ntuple.root') 
        if not os.path.isfile(augmented_tree): 
            print '--- augmenting reduced dataset with nn classifiers ---'
            # should wrap this in a function to close the file when done
            
            from ROOT import TFile
            testing_ds_file = TFile(self._testing_ds)

            the_tree = testing_ds_file.Get('SVTree')
            all_vars_in_tree = utils.get_leaves_in_tree(the_tree)

            # --- filter out branches we don't care about
            output_subset = ['bottom','light','charm']
            subset_regex = '|'.join([
                    '^discriminator(?!Jet)', 
                    '^log[BCU][bcu]', 
                ])
            branch_filter = re.compile(subset_regex)
            for branch in itertools.chain(*all_vars_in_tree.values()): 
                if branch_filter.findall(branch): 
                    output_subset.append(branch)

            from jetnet import pynn
            pynn.augment_tree(
                in_file = self._testing_ds, 
                nn_file = weights_path, 
                out_file = augmented_tree, 
                ints = all_vars_in_tree['Int_t'], 
                doubles = all_vars_in_tree['Double_t'], 
                extension = self._profile_extension , 
                subset = output_subset, 
                show_progress = True) 

        profiled_path = os.path.splitext(augmented_tree)[0] + '_profile.root'
        
        if not os.path.isfile(profiled_path): 
            print '--- profiling performance ntuple ---'
            profile.make_profile_file(reduced_dataset = augmented_tree, 
                                      profile_file = profiled_path)

        output_paths = {
            'profile': profiled_path, 
            'perf_ntuple': augmented_tree, 
            }


        if self._do_more_diagnostics is not None: 
            warn("do_more_diagnostics doesn't do anything now, please remove", 
                 SyntaxWarning, 
                 stacklevel = 5, 
                 # stacklevel 5 needed to get through the multiprocess call
                 ) 
                 

        self.out_queue.put(output_paths)


