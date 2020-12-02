"""
Module for using the Caltech101 dataset
"""

import os
import os.path as osp
#from pathlib import Path
import glob
import numpy as np
import urllib.request
import tarfile
import random
import itertools

from sensor.tools import iotools

#------------------------------------------------------------------------------
class Caltech101():
    """
    Caltech101

    Pictures of objects belonging to 101 categories. About 40 to 800 images 
    per category. Most categories have about 50 images.

    URL: http://www.vision.caltech.edu/Image_Datasets/Caltech101/
    """
    dataset_dir = 'caltech101'

    #-------------------------------------------------------------------------
    def __init__(self, root: str = 'data',
                 nb_pairs: int = 15, 
                 nb_cat: int = 101,
                 verbose: bool = True) -> None:

        self.nb_pairs = nb_pairs
        self.nb_categories = nb_cat

        self.dataset_dir = osp.join(root, self.dataset_dir)
        self.dataset_url = ('http://www.vision.caltech.edu/Image_Datasets'
            '/Caltech101/101_ObjectCategories.tar.gz')
        self.annotation_url = ('http://www.vision.caltech.edu/Image_Datasets'
            '/Caltech101/Annotations.tar')
        self.results_dir = self.dataset_dir
        self.dataset_path = osp.join(self.dataset_dir, '101_ObjectCategories')
        self.annotation_path = osp.join(self.dataset_dir, 'Annotations')

        test_pairs_filename = ('test_set_' + str(nb_pairs) + 'pairs_' +
                               str(nb_cat) + 'cat.json')
        self.test_pairs_filepath = osp.join(self.dataset_dir,
                                            test_pairs_filename)

        # Caltech101 files URL now redirect to Google Drive, which broke the 
        # download code in _download_data(). Caltech101 files should be manually
        # downloaded and extracted until _download_data() is modified to handle
        # downloading files from Google Drive.
        #self._download_data()

        self._load()

    #--------------------------------------------------------------------------
    def get_test_pairs(self):
        """
        Get the testing pairs and annotations filepaths
        """
        annotation_paths = [dir for dir in os.listdir(self.annotation_path)
                            if osp.isdir(osp.join(self.annotation_path,dir))]
        
        test_pairs = []                    
        for cat_id, pairs in enumerate(self.pairs):
            imgs_path = osp.join(self.dataset_path, pairs['Category'])
            anno_path = osp.join(self.annotation_path, 
                                  annotation_paths[cat_id])

            # # Remove the first folder in the path for Matlab
            # imgs_path = imgs_path.relative_to(*imgs_path.parts[:2])
            # anno_path = anno_path.relative_to(*anno_path.parts[:2])
            # imgs_path = osp.join(imgs_path.parts)
            # print(imgs_path)

            imgs = os.listdir(imgs_path)
            annotations = os.listdir(anno_path)

            img_pairs = []
            anno_pairs = []
            for pair in pairs['pairs']:
                img_pairs.append(tuple((
                    osp.join(imgs_path,imgs[pair[0]]), 
                    osp.join(imgs_path,imgs[pair[1]]))))
                anno_pairs.append(tuple((
                    osp.join(anno_path,annotations[pair[0]]), 
                    osp.join(anno_path,annotations[pair[1]]))))

            test_pairs.append({'Category':pairs['Category'], 
                          'Images':img_pairs, 
                          'Annotations':anno_pairs})

        return test_pairs

    #--------------------------------------------------------------------------
    def summarize_results(self, filename):
        """
        Displays a summary of the results from the given file
        """
        
        result_filepath = osp.join(self.results_dir, filename)
        results = iotools.read_json(result_filepath)

        print('Number of categories:', len(results))

        total_mean = 0
        total_iou = 0
        total_loc_err = 0
        total_time = 0
        for category in results:
            cat_mean = 0
            cat_iou = 0
            cat_loc_err = 0
            cat_time = 0
            for pair in category['Results']:
                cat_mean += float(pair['Result']['mean'])
                cat_iou += float(pair['Result']['iou'])
                cat_loc_err += float(pair['Result']['loc_err'])
               # cat_time += float(pair['Result']['time'])

            cat_nb_results = len(category['Results'])
            cat_mean /= cat_nb_results
            cat_iou /= cat_nb_results
            cat_loc_err /= cat_nb_results
            # cat_time /= cat_nb_results 

            total_mean += cat_mean
            total_iou += cat_iou
            total_loc_err += cat_loc_err
            total_time += cat_time
        
        nb_categories = len(results)
        total_mean /= nb_categories
        total_iou /= nb_categories
        total_loc_err /= nb_categories
        # total_time /= nb_categories

        print('Mean: %1.4f \t iou: %1.4f \t err: %1.4f \t' %(
            total_mean, total_iou, total_loc_err))
        print('Total time %d' %(total_time))
        print(total_time)

    #--------------------------------------------------------------------------
    def _download_data(self) -> None:
        """ Downloads the dataset if the directory is not on disk."""
        if osp.exists(self.dataset_path):
            return

        print("Caltech101 dataset not found at '{}'".format(self.dataset_path))
        if not osp.exists(self.dataset_dir):
            print("Creating directory {}".format(self.dataset_dir))
            iotools.mkdir_if_missing(self.dataset_dir)

        print('Downloading Caltech101 dataset...')
        caltech_tarfile = osp.join(self.dataset_dir,
                                 osp.basename(self.dataset_url))
        annotation_tarfile = osp.join(self.dataset_dir,
                                 osp.basename(self.annotation_url))
        urllib.request.urlretrieve(self.dataset_url, caltech_tarfile)
        urllib.request.urlretrieve(self.annotation_url, annotation_tarfile)

        print('Extracting files...')
        tar = tarfile.open(caltech_tarfile)
        tar.extractall(self.dataset_dir)
        tar.close()
        os.remove(caltech_tarfile)

        tar = tarfile.open(annotation_tarfile)
        tar.extractall(self.dataset_dir)
        tar.close()
        os.remove(annotation_tarfile)

    #--------------------------------------------------------------------------
    def _load(self, verbose: bool = True) -> None:
        """
        Validate that images and annotationsare on disk. 
        Load the test pairs.
        """
        if not osp.exists(self.dataset_dir):
            raise FileNotFoundError(
                "'{}' is not found".format(self.dataset_dir))
        if not osp.exists(self.dataset_path):
            raise FileNotFoundError(
                "'{}' is not found".format(self.dataset_path))
        if not osp.exists(self.annotation_path):
            raise FileNotFoundError(
                "'{}' is not found".format(self.annotation_path))

        if not osp.exists(self.test_pairs_filepath):
            self._prepare_test_pairs(self.nb_pairs, self.nb_categories)

        self.pairs = iotools.read_json(self.test_pairs_filepath)

        if verbose:
            print('Caltech101 dataset:')
            print('  --------------------')
            print('  Number of categories: ',str(len(self.pairs)))
            print('  Number of testing pairs: ',str(len(
                self.pairs[0]['pairs'])))
            print('  --------------------')

    #--------------------------------------------------------------------------
    def _prepare_test_pairs(self, nb_pairs, nb_categories) -> None:
        if osp.exists(self.test_pairs_filepath):
            return

        print('Creating', nb_pairs, 'random pairs for',
              nb_categories, 'categories')

        test_pairs = []

        categories = os.listdir(self.dataset_path)
        categories.remove('BACKGROUND_Google')

        if nb_categories > 101: nb_categories = 101

        for cat_id, category in enumerate(categories):
            if cat_id == nb_categories:
                break
            # Get image filenames
            imgs = os.listdir(osp.join(self.dataset_path, category))

            # Generate all possible non-repeating pairs using
            nb_imgs = len(imgs)
            pairs = list(itertools.combinations(range(nb_imgs), 2))

            # TODO: annotations for some images are invalid, remove pairs that 
            # have invalid annotations 
            #cougar 32 (id:31)
            #wrench 10 (id:9)
 
            # Randomly shuffle these pairs
            random.shuffle(pairs)
            # Select nb_pairs
            n = nb_pairs
            pairs = pairs[:n]

            #split = {'Category': category, 'pairs': pairs}
            test_pairs.append({'Category': category, 'pairs': pairs})

        iotools.write_json(test_pairs, self.test_pairs_filepath)
        print('Test pairs file saved to', self.test_pairs_filepath)
