"""
System file tools
"""
#from __future__ import absolute_import

import os
import os.path as osp
import errno
import json
import pickle
import bz2

#------------------------------------------------------------------------------
def mkdir_if_missing(directory):
    """Create the directory if it doesn't exist."""
    if not osp.exists(directory):
        if directory == '':
            return
        try:
            os.makedirs(directory)
        except OSError as error:
            if error.errno != errno.EEXIST:
                raise

#------------------------------------------------------------------------------
def read_json(fpath):
    """Read a JSON file."""
    with open(fpath, 'r') as file:
        obj = json.load(file)
    return obj

#------------------------------------------------------------------------------
def write_json(obj, fpath):
    """Write to a JSON file."""
    mkdir_if_missing(osp.dirname(fpath))
    with open(fpath, 'w') as file:
        json.dump(obj, file, indent=4, separators=(',', ': '), sort_keys=True)

#------------------------------------------------------------------------------
def save_pbz2_file(data, filename):
    """
    Compresse and save data to binary file (pickled bz2 file)
    """
    with bz2.BZ2File(filename, 'w') as filehandle:
        # store the data as binary data stream
        pickle.dump(data, filehandle, pickle.HIGHEST_PROTOCOL)

#------------------------------------------------------------------------------
def load_pbz2_file(filename):
    """
    Loads a pickled bz2 file
    """
    with bz2.BZ2File(filename, 'r') as filehandle:
        return pickle.load(filehandle)

#------------------------------------------------------------------------------
def calculate_if_nobackup(func, args, filepath, recalculate=False):
    """
    Load data from the given file if it exists. If no file is found,
    call the given funtion to generate the data and then save it to
    the file for next time.

    Parameters
    ----------
    filepath: str
        Path of the backup file
    func: Function
        Function to call to generate the desired data
    args: List
        List of arguments to give the function
    """
    if not osp.exists(filepath) or recalculate:
        data = func(*args)
        save_pbz2_file(data, filepath)
    else:
        data = load_pbz2_file(filepath)

    return data

#------------------------------------------------------------------------------
def generate_backup_suffix(options):
    """
    Standardize backup file naming by generating a suffix for naming files.

    Concatenate important options to create a customized filename. The suffix
    ends with the extention '.pbz2' (pickled bz2). Mostly used for the
    function: calculate_if_nobackup()

    Parameters
    ----------
    option: Dict
        Options used for creating the suffix. Must have the keys:
            'dataset': string
                Name of the dataset
            'features': string
                Name of the feature extraction method
            'patch_size': (int, int)
                Size of patches to extract (width, height)
            'isolate_patches': Bool
                Wheter or not to use isolated patch for metrics computations

    Returns
    -------
    string
        Suffix for naming '.pbz2' files

    """
    patch_size = options['patch_size']
    suffix = (options['dataset'] + '_' +
              options['features'] + '_' +
              'p{}x{}'.format(patch_size[0], patch_size[1]) +
              ('_patch' if options['isolate_patches'] else '') +
              '.pbz2')
    return suffix
