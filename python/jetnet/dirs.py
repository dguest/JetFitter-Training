import os, glob
import ConfigParser
import sys

class OverwriteError(IOError): 
    """
    raised when we try to overwrite files 
    """
    pass

def make_directories(config_file = 'training.config'): 
    """
    returns a list of directories which were created 
    """
    methods = ['JetNet', 'LikelihoodHistos'] 
    dirs_created = []

    conf = ConfigParser.ConfigParser()
    conf.read(config_file) 
    collections_to_process = conf.get('collections', 'process').split()

    for m in methods: 
        method_dir_name = 'trainingResults' + m

        for jet_dir_name in collections_to_process: 
            full_dir_path = method_dir_name + '/' + jet_dir_name
            if os.path.isdir(full_dir_path): 
                if glob.glob(full_dir_path + '/*.root*'):
                    raise OverwriteError("files found in %s" % 
                                         full_dir_path)

            else:
                os.makedirs(full_dir_path)
