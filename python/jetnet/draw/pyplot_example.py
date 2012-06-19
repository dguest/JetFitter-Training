#!/usr/bin/env python2.7
from ROOT import TFile, TH1D
from numpy import array, log
import matplotlib.pyplot as plt

root_file = TFile('flavor_wt_hists.root')
flavors = ['charm','bottom','light']

def get_bin_array(red_hist, green_hist, blue_hist): 
    def get_n_bins(hist): 
        x_bins = red_hist.GetNbinsX()
        y_bins = red_hist.GetNbinsY()
        return x_bins, y_bins

    hists = [red_hist, green_hist, blue_hist]

    all_equal = (
      get_n_bins(red_hist) == get_n_bins(blue_hist) == get_n_bins(green_hist)
      )
    assert all_equal, 'oh dear!'

    x_bins, y_bins = get_n_bins(red_hist)

    bin_array = []
    for y_bin in xrange(1,y_bins + 1): 
        x_row = []
        bin_array.append(x_row)
        for x_bin in xrange(1,x_bins + 1): 
            x_row.append( 
                [h.GetBinContent(x_bin, y_bin) for h in hists])
                          
    np_array = array(bin_array)
    return np_array

h = [root_file.Get('JetEta_vs_JetPt_{}'.format(f)) for f in flavors]
np_array  = get_bin_array(*h)
array_max_vals = [np_array[:,:,i].max() for i in xrange(3)]
for i,the_max in enumerate(array_max_vals): 
    np_array[:,:,i] = np_array[:,:,i] / the_max
# np_array = log(np_array)
fig = plt.figure()
pt_range = (15,200)
eta_range = (-2.5, 2.5)
pt_span = pt_range[1] - pt_range[0]
eta_span = eta_range[1] - eta_range[0]
all_extent = (pt_range[0], pt_range[1],
              eta_range[0],eta_range[1])
all_aspect = pt_span / eta_span * 2/3
plot = plt.imshow(np_array, 
                  extent = all_extent, 
                  aspect = all_aspect, 
                  interpolation='nearest')
plt.xlabel(r'$p_\mathrm{T}$ [GeV]', fontsize = 32)
plt.xticks(fontsize = 16)
plt.ylabel(r'$\eta$', fontsize = 32)
plt.yticks(fontsize = 16)

empty = array([])
plt.plot(empty, 'r-', label = 'charm' , lw = 10)
plt.plot(empty, 'g-', label = 'bottom', lw = 10) 
plt.plot(empty, 'b-', label = 'light' , lw = 10)

plt.legend()
plt.axis(all_extent)

print plot
plt.savefig('somthing.pdf', bbox_inches = 'tight')

# print np_array
