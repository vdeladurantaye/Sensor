"""
Testing file for the DSM wrapper
"""

import os.path as osp
import sys
import configparser

import cv2 as cv

from sensor.tools import iotools
from sensor import cpp_sensor

def ReadConfig(config_filename):
    """
    Read the configuration parameters from the given file and returns them
    as a dict
    """
    config = configparser.ConfigParser()
    config.read(config_filename)
    
    config_dict = {s:dict(config.items(s)) for s in config.sections()}

    return config_dict
 
 #------------------------------------------------------------------------------
def GetPixelConvergenceStats(dataset, result_filename, random_init, 
                             merge_segments, auto_trigger_segments):
    """
    Runs the pixel segmentation on pairs of images taken from the given dataset
    and calculates the average cycles, cascades and spikes taken for the 
    network to stabilize
    """
    cycle_avg = 0
    cascade_avg = 0
    spike_avg = 0

    pair_count = 0
    cat_count = 0

    results = []
    
    test_pairs = dataset.get_test_pairs()
    for category in test_pairs:
        cat_count = cat_count+1
        print('Category ' + str(cat_count) +'/'+ str(len(test_pairs)))

        category_avg = 0

        category_results = []
        for pair_id, img_pair in enumerate(category['Images']):
            for img_path in img_pair:
                print(img_path)
                #img = cv.imread(img_path)
                pixel_layer = cpp_sensor.PixelLayer(img_path)
                pixel_layer.SegmentLayer()
                
                img_results = {
                    'Image':osp.basename(img_path),
                    'Cycles':pixel_layer.GetNbCycles(),
                    'Cascades':pixel_layer.GetNbCascades(),
                    'Spikes':pixel_layer.GetNbSpikes(),
                    'Convergence':pixel_layer.GetCoefStabilization()}
                
                cycle_avg += pixel_layer.GetNbCycles()
                cascade_avg += pixel_layer.GetNbCascades()
                spike_avg += pixel_layer.GetNbSpikes()
                pair_count += 1

                category_results.append(img_results)

                print(img_results)
                print('Average Cycles: ' + str(cycle_avg/pair_count) +
                ' Cascades: ' + str(cascade_avg/pair_count) + 
                ' Spikes: ' + str(spike_avg/pair_count) +'\n')

        category_avg /= len(category['Images'])

        results.append({'Category':category['Category'],
                        'Cycle cumu':cycle_avg/pair_count,
                        'Cascade cumu':cascade_avg/pair_count,
                        'Spikes cumu':spike_avg/pair_count,
                        'Results':category_results})


    result_filepath = osp.join(dataset.results_dir, result_filename)
    iotools.write_json(results, result_filepath)
