#!/usr/bin/env python2.6

from jetnet import pynn

rds = 'reduceddatasets/giacinto/reduceddataset_AntiKt4TopoEMJets_forNN.root'
wt = (
    'trainingResultsJetNet/AntiKt4TopoEMJets/giacinto/weights/'
    'weightMinimum.root' 
    )

out = 'testout' 

pynn.makeNtuple( input_weights = wt, 
                 input_dataset = rds, 
                 output_file = 'test.root', 
                 # output_tree = 'performance' 
                 ) 
