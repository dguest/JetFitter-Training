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
    header = 'omega_scripts/run_header.sh'

    dirs_created = []

    conf = ConfigParser.ConfigParser()
    conf.read(config_file) 
    collections_to_process = conf.get('collections', 'process').split()

    script_builder = ScriptBuilder(header_script = header)

    for m in methods: 
        method_dir_name = 'trainingResults' + m

        for jet_dir_name in collections_to_process: 

            for input_data in input_data_list: 
                rel_dir_path = '/'.join( (
                        method_dir_name,
                        jet_dir_name, 
                        input_data
                        ) )
                input_ds = glob.glob('reduceddatasets/%s/*.root' % input_data)
                if len(input_ds) != 1: 
                    print input_ds
                    sys.exit('those ds were a problem')
                else: 
                    (input_ds,) = input_ds
                script_builder.build_script(rel_dir_path, input_ds)
                    
class ScriptBuilder(object): 
    """
    build the script for sourcing by the batch system 
    """
    def __init__(self, header_script, finalize_script = None,
                 name = 'run.sh', output_dir = 'weights', 
                 executable = 'doTraining.py', 
                 base_dir = None): 
        """
        all paths are relative (built up from base_dir)
        """
        self.header = []
        with open(header_script) as header_file: 
            for line in header_file:
                self.header.append(line)

        self.name = name
        self.output_dir = output_dir
        self.executable = executable
        
        if base_dir: 
            self.base_dir = base_dir.rstrip('/')
        else: 
            self.base_dir = os.getcwd()

        self.finalize = []
        if finalize_script: 
            with open(finalize_script) as finalize_file: 
                for line in finalize_file: 
                    self.finalize.append(line)

    def build_script(self, script_dir, input_ds, new_script_name = None):
        """
        all paths relative to base_dir 
        """
        assert new_script_name is None, 'this is not implemented' 

        full_script_dir = os.path.join(self.base_dir, script_dir)
        full_script_path = os.path.join(full_script_dir, self.name)
        full_input_path = os.path.join(self.base_dir, input_ds)

        # setup the directory 
        self._build_dir(os.path.join(full_script_dir,self.output_dir))

        with open(full_script_path,'w') as product: 
            # copy over the header
            for line in self.header: 
                product.write(line)

            product.write('\n')

            ex_tuple = (self.executable, full_input_path, 
                        self.output_dir)
            product.write('%s -t %s -o %s\n' % ex_tuple)

            for line in self.finalize: 
                product.write(line)


    def _build_dir(self,full_dir_path): 
        """
        path should be absolute
        """
        if os.path.isdir(full_dir_path): 
            if glob.glob(full_dir_path + '/*.root*'):
                raise OverwriteError("files found in %s" % 
                                     full_dir_path)

        else:
            os.makedirs(full_dir_path)
                

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
