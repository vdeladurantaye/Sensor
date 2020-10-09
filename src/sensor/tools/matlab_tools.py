"""
Manage Matlab session
"""
import os.path as osp
import numpy as np
import scipy.io as sio

import matlab.engine

#-------------------------------------------------------------------------------
# Class to manage a Matlab session. The Matlab engine is stored in a static
# class variable so that the same session is reused

class MatlabSession:
    """
    Connect to an existing matlab session or start a new matlab engine
    if no session is available. Also change the workspace directory of
    the matlab session to the directory of this file and runs ml_setup.m

    To make a matalab session available to Python, run this line in matlab:
    >> matlab.engine.shareEngine
    """
    _eng = None

    def __init__(self):
        pass

    @staticmethod
    def get_engine():
        """
        Ensure that the Matlab session is still in the right working
        directory and return the Matlab engine.
        """
        if MatlabSession._eng is None:
            print('\nConnecting to Matlab...')
            MatlabSession._eng = matlab.engine.connect_matlab()

        working_dir = osp.join(osp.dirname(__file__), '..', 'Matlab')
        working_dir = osp.normpath(working_dir)
        # If the matlab session is not already in the right directory
        if MatlabSession._eng.cd().lower() != working_dir.lower():
            # Change working directory and run ml_setup.m
            MatlabSession._eng.cd(working_dir)
            MatlabSession._eng.run('ml_setup', nargout=0)

        return MatlabSession._eng

#------------------------------------------------------------------------------
def matlab_to_numpy(matlab_array):
    """
    Convert Matlab array to numpy array
    """
    # Solution from:
    # https://stackoverflow.com/questions/34155829/
    np_array = np.array(matlab_array._data) # pylint: disable=W0212
    np_array = np_array.reshape(matlab_array.size, order='F')
    return np_array

#------------------------------------------------------------------------------
def numpy_to_matlab(numpy_array):
    """
    Convert numpy array to Matlab array
    """
    matlab_array = matlab.double(numpy_array.tolist())
    return matlab_array

#------------------------------------------------------------------------------
def save_dict_to_mat(dict, filename):
    python_temp_folder = osp.join(osp.dirname(__file__),
                                  '..', 'Matlab', 'python_tmp_files')
    python_temp_folder = osp.normpath(python_temp_folder)
    filepath = osp.join(python_temp_folder, filename)

    sio.savemat(filepath, dict)

#------------------------------------------------------------------------------
def save_to_mat(data, var_name, filename):
    """
    Saves data as a .mat file
    """
    eng = MatlabSession.get_engine()

    print('Saving ' + var_name + ' to ' + filename)

    eng.workspace[var_name] = data
    
    command = "save('{}', '{}');".format(filename, var_name)
    eng.eval(command, nargout=0)
