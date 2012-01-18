import os, glob
import sys

class OverwriteError(IOError): 
    """
    raised when we try to overwrite files 
    """
    pass

class InputDSError(OSError): 
    pass

                    
class ScriptBuilder(object): 
    """
    build the script for sourcing by the batch system 
    """
    def __init__(self, header_script, finalize_script = None,
                 name = 'run.sh', output_dir = 'weights', 
                 executable = 'doTraining.py', 
                 base_dir = None, 
                 overwrite = False): 
        """
        all paths are relative (built up from base_dir)
        base_dir defaults to the current directory
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

        self.overwrite = overwrite

    def build_script(self, script_dir, input_ds, new_script_name = None):
        """
        all paths relative to base_dir 
        """
        assert new_script_name is None, 'this is not implemented' 

        full_script_dir = os.path.join(self.base_dir, script_dir)
        full_script_path = os.path.join(full_script_dir, self.name)
        full_input_path = os.path.join(self.base_dir, input_ds)

        # setup the directory 
        try: 
            self._build_dir(full_script_dir)
            self._build_dir(os.path.join(full_script_dir,self.output_dir))
        except OverwriteError: 
            if not self.overwrite: 
                raise 

        with open(full_script_path,'w') as product: 
            # copy over the header
            for line in self.header: 
                product.write(line)

            product.write('\n')

            ex_tuple = (self.executable, full_input_path, 
                        self.output_dir)
            product.write('%s %s -o %s' % ex_tuple)
            product.write('\n')

            for line in self.finalize: 
                product.write(line)


    def _build_dir(self,full_dir_path): 
        """
        path should be absolute
        """
        if os.path.isdir(full_dir_path): 
            root_files = glob.glob(full_dir_path + '/*.root*')
            shell_files = glob.glob(full_dir_path + '/*.sh')
            if root_files or shell_files: 
                raise OverwriteError("files found in %s" % 
                                     full_dir_path)

        else:
            os.makedirs(full_dir_path)
                

