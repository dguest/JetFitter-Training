import os, glob
import ConfigParser
import sys

class OverwriteError(IOError): 
    """
    raised when we try to overwrite files 
    """
    pass

class InputDSError(OSError): 
    pass

def make_directories(config_file = 'training.config', 
                     header_script = 'omega_scripts/run_header.sh', 
                     input_data_list = None): 
    """
    returns a list of directories which were created 
    """
    methods = ['JetNet'] #, 'LikelihoodHistos'] 
    dirs_created = []

    conf = ConfigParser.ConfigParser()
    conf.read(config_file) 
    collections_to_process = conf.get('collections', 'process').split()

    for m in methods: 
        method_dir_name = 'trainingResults' + m

        for jet_dir_name in collections_to_process: 

            for input_data in input_data_list: 
                full_dir_path = (method_dir_name + '/' + jet_dir_name + '/' 
                                 + input_data)
                _build_dir(full_dir_path)
                if header_script: 
                    _setup_dir(full_dir_path, header_script)

def _build_dir(full_dir_path): 
    if os.path.isdir(full_dir_path): 
        if glob.glob(full_dir_path + '/*.root*'):
            raise OverwriteError("files found in %s" % 
                                 full_dir_path)

    else:
        os.makedirs(full_dir_path)
                
def _build_script(headar_script, path, input_ds, 
                  name = 'run.sh', output_dir = 'weights'): 
    full_path = path + '/' + name
    with open(full_path,'w') as product: 

        # copy over the header
        with open(headar_script) as header: 
            for line in header: 
                product.write(line)

        product.write('\n\n')
        product.write('doTraining.py -t %s -o %s' % 
                      (input_ds, output_dir))

def _setup_dir(dir_path, header_script, input_file = None): 
    
    input_file_base = '${HOME}/work/JetFitter/reduceddatasets'
    full_base = os.path.expandvars(input_file_base)
    if not input_file: 
        subset = dir_path.split('/')[-1]
        full_input_path = full_base + '/' + subset
        input_ds_list = glob.glob(full_input_path + '/*.root*')
        if len(input_ds_list) != 1:
            raise InputDSError('wrong number of ds in %s' % 
                               ' '.join(input_ds_list))
        input_ds = input_ds_list[0]


    _build_script(header_script, path = dir_path, input_ds = input_ds)
    if not os.path.isdir(dir_path + '/weights'): 
        os.mkdir(dir_path + '/weights')
