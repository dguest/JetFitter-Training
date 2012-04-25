from ROOT import TH2D, TFile
from random import random

class CorPlot(TH2D): 
    def __init__(self, name, x_map, x_bins, x_min, x_max, 
                 y_map, y_bins, y_min, y_max, 
                 wt_map = lambda x: 1, 
                 fill_test = lambda x: True): 
        super(CorPlot,self).__init__(str(random()),name,x_bins, x_min, x_max,
                                     y_bins, y_min, y_max)
        self.name = name
        self.x_map  = self._str_to_map(x_map)
        self.y_map  = self._str_to_map(y_map)
        self.wt_map = self._str_to_map(wt_map)
        self.fill_test = fill_test

    def _str_to_map(self,var_name): 
        """
        If var_name is a string, returns a function that gets that variable
        from the buffer. Otherwise returns var_name. 
        """
        if not isinstance(var_name,str): 
            return var_name

        def the_map(fill_buffer): 
            return getattr(fill_buffer,var_name)
        return the_map

    def fill(self,fill_buffer): 
        test_val = self.fill_test(fill_buffer)
        if test_val: 
            self.Fill(self.x_map(fill_buffer), 
                      self.y_map(fill_buffer), 
                      self.wt_map(fill_buffer))
            

def make_tag_wt_correlations(ntuple_name, tree_name = 'performance', 
                             output_name = 'cor_plots.root', 
                             max_entries = 1000, 
                             quiet = False): 

    
    root_file = TFile(ntuple_name)
    root_tree = root_file.Get(tree_name)

    wt_names = [
        'NNb', 
        'NNc', 
        'NNu', 
        ]

    cor_plots = []
    for i, x_wt in enumerate(wt_names,1): 
        for y_wt in wt_names[i:]: 
            
            for tag in [None,'bottom','charm','light']: 
                
                name = '%s vs %s' % (x_wt, y_wt)
                if tag: 
                    name += ' ' + tag

                    def fill_test(entry, tag = tag): 
                        value = getattr(entry,tag)
                        # print 'check for', tag
                        return value

                else: 
                    def fill_test(entry): 
                        return True

                the_plot = CorPlot(
                    name = name, fill_test = fill_test, 
                    x_map = x_wt, x_bins = 100, x_min = 0, x_max = 1, 
                    y_map = y_wt, y_bins = 100, y_min = 0, y_max = 1, 
                    ) 
                the_plot.GetXaxis().SetTitle(x_wt)
                the_plot.GetYaxis().SetTitle(y_wt)

                cor_plots.append(the_plot)


    for entry_n, entry in enumerate(root_tree): 
        if entry_n % 1000 == 0 and not quiet: 
            print '%i entries processed' % entry_n
        if max_entries and entry_n > max_entries: 
            break 

        for plot in cor_plots: 
            plot.fill(entry)

    out_file = TFile(output_name,'recreate')
    for plot in cor_plots: 
        plot.SetName(plot.name.replace(' ','_'))
        out_file.WriteTObject(plot)
    
