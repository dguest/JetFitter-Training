#!/usr/bin/env python2.6

from jetnet import pynn
import os, sys

rds = '../reduceddatasets/%s/reduceddataset_AntiKt4TopoEMJets_forNN.root' 
wt = (
    '../trainingResultsJetNet/AntiKt4TopoEMJets/%s/weights/'
    'weightMinimum.root' 
    )


authors = ['giacinto', 'dan']

for ds_author in authors: 
    for training_ds_author in authors:

        out_name = '%s_trained_%s_ds.root' % (training_ds_author, ds_author)

        input_weights = wt % training_ds_author
        input_dataset = rds % ds_author

        if not os.path.isfile(input_dataset): 
            sys.exit('%s does not exist' % input_dataset)
        
        if not os.path.isfile(input_weights): 
            sys.exit('%s does not exist' % input_weights)

        pynn.makeNtuple( input_weights = input_weights, 
                         input_dataset = input_dataset, 
                         output_file = out_name, 
                         # output_tree = 'performance' 
                         ) 
