"""
Module for using the VIPeR dataset
"""
from typing import List, Dict, Any

from collections import namedtuple
import os
import os.path as osp
import glob
import urllib.request
import zipfile
import numpy as np

from utils import iotools

#------------------------------------------------------------------------------
def convert_matlab_partition(
        json_filename: str,
        pbz2_filename: str) -> List[Dict[str, Any]]:
    """
    Loads a JSON partition file generated from Matlab reid benchmark and
    saves it as a python pickled bz2 file. Returns the converted partition
    as a dict.

    To generate a JSON file from Matlab R2016, use JSONlab:
    https://github.com/fangq/jsonlab

    >> addpath('/path/to/jsonlab');
    >> load('TrainTestSplits/Partition_viper.mat');
    >> savejson('partition',partition,'partitions_viper.json')
    """
    partition_list = []
    data = iotools.read_json(json_filename)
    for partition in data['partition']:
        idx_train = partition['idx_train']
        idx_test = partition['idx_test']
        idx_probe = partition['idx_probe']
        idx_gallery = partition['idx_gallery']
        ix_pos_pair = partition['ix_pos_pair']
        ix_neg_pair = partition['ix_neg_pair']

        partition_dict = {'idx_train': idx_train,
                          'idx_test': idx_test,
                          'idx_probe': idx_probe,
                          'idx_gallery': idx_gallery,
                          'ix_pos_pair': ix_pos_pair,
                          'ix_neg_pair': ix_neg_pair}

        partition_list.append(partition_dict)

    iotools.save_pbz2_file(partition_list, pbz2_filename)

    return partition_list

#------------------------------------------------------------------------------
def load_matlab_partition(filename: str) -> List[Dict[str, Any]]:
    """
    Loads a pbz2 matlab partition file generated by the
    convert_matlab_partition() function.
    """
    if not osp.exists(filename):
        raise RuntimeError("'{}' not found!".format(filename))

    return iotools.load_pbz2_file(filename)

#------------------------------------------------------------------------------
class VIPeR():
    """
    VIPeR

    Reference:
    Gray et al. Evaluating appearance models for recognition, reacquisition,
    and tracking. PETS 2007.

    URL: https://vision.soe.ucsc.edu/node/178

    Dataset statistics:
    # identities: 632
    # images: 632 x 2 = 1264
    # cameras: 2
    """
    dataset_dir = 'VIPeR'

    #-------------------------------------------------------------------------
    def __init__(self, root: str = 'data',
                 nb_pairs: int = 632,
                  verbose: bool = True) -> None:

        self.nb_pairs = nb_pairs
        self.nb_categories = 2

        self.dataset_dir = osp.join(root, self.dataset_dir)
        self.dataset_url = 'http://users.soe.ucsc.edu/~manduchi/VIPeR.v1.0.zip'
        self.cam_a_path = osp.join(self.dataset_dir, 'cam_a')
        self.cam_b_path = osp.join(self.dataset_dir, 'cam_b')

        self.reference_a = osp.join(self.dataset_dir, 'cam_a', '379_0.bmp')
        self.reference_b = osp.join(self.dataset_dir, 'cam_b', '152_90.bmp')

        #self._download_data()

        self.partitions = [] #type: List[Dict[str, Any]]
        self._load(verbose)

        #self.load_partition(split_id, verbose)
        #self._prepare_split()

        # (self.train,
        #  self.query,
        #  self.gallery,
        #  self.num_train_pids,
        #  self.num_query_pids,
        #  self.num_gallery_pids) = self._load_split(split_id, verbose)

    #--------------------------------------------------------------------------
    def get_test_pairs(self):
        """
        Get the testing pairs
        """        
        test_pairs = []             
        
        cam_a_list = glob.glob(osp.join(self.cam_a_path, '*.bmp'))
        cam_b_list = glob.glob(osp.join(self.cam_b_path, '*.bmp'))
        # self.img_list = cam_a_list + cam_b_list

       # for cam_a_img in enumerate(cam_a_list):
        # Generate the pairs
        img_pairs_a = [(self.reference_a, img) for img in cam_a_list]
        # Truncate list to appropriate number of pairs
        img_pairs_a = img_pairs_a[:self.nb_pairs]

        test_pairs.append({'Category': 'cam_a', 
                        'Images':img_pairs_a})#, 
                        #'Annotations':anno_pairs})
                        
        img_pairs_b = [(self.reference_b, img) for img in cam_b_list]
        # Truncate list to appropriate number of pairs
        img_pairs_b = img_pairs_b[:self.nb_pairs]

        test_pairs.append({'Category': 'cam_b', 
                        'Images':img_pairs_b})


        # for cat_id, pairs in enumerate(self.pairs):
        #     imgs_path = osp.join(self.dataset_path, pairs['Category'])
        #     anno_path = osp.join(self.annotation_path, 
        #                           annotation_paths[cat_id])

        #     # # Remove the first folder in the path for Matlab
        #     # imgs_path = imgs_path.relative_to(*imgs_path.parts[:2])
        #     # anno_path = anno_path.relative_to(*anno_path.parts[:2])
        #     # imgs_path = osp.join(imgs_path.parts)
        #     # print(imgs_path)

        #     imgs = os.listdir(imgs_path)
        #     annotations = os.listdir(anno_path)

        #     img_pairs = []
        #     anno_pairs = []
        #     for pair in pairs['pairs']:
        #         img_pairs.append(tuple((
        #             osp.join(imgs_path,imgs[pair[0]]), 
        #             osp.join(imgs_path,imgs[pair[1]]))))
        #         anno_pairs.append(tuple((
        #             osp.join(anno_path,annotations[pair[0]]), 
        #             osp.join(anno_path,annotations[pair[1]]))))

        #     test_pairs.append({'Category':pairs['Category'], 
        #                   'Images':img_pairs, 
        #                   'Annotations':anno_pairs})

        return test_pairs
    #--------------------------------------------------------------------------
    # def get_partition(self, split_id: int = 0) -> ReidDataset.Split:
    #     """
    #     Get a dataset training/testing split
    #     """
    #     ImgInfo = namedtuple(
    #         'ImgInfo', [
    #             'path', 'person_id', 'cam_id', 'img_id'])
    #     imgs_info = [ImgInfo(img,                    # Image filename
    #                          id % 632,               # Person id
    #                          1 if id < 632 else 2,   # Camera id
    #                          id)                     # Image id
    #                  for id, img in enumerate(self.img_list)]

    #     idx_train = self.partitions[split_id]['idx_train']
    #     train = [imgs_info[id]
    #              for id, train in enumerate(idx_train) if train == 1]

    #     idx_test = self.partitions[split_id]['idx_test']
    #     test = [imgs_info[id]
    #             for id, test in enumerate(idx_test) if test == 1]

    #     idx_probe = self.partitions[split_id]['idx_probe']
    #     probe = [test[id]
    #              for id, probe in enumerate(idx_probe[0])
    #              if probe == 1]

    #     idx_gallery = self.partitions[split_id]['idx_gallery']
    #     gallery = [test[id]
    #                for id, gal in enumerate(idx_gallery[0])
    #                if gal == 1]

    #     return ReidDataset.Split(train, test, probe, idx_probe,
    #                              gallery, idx_gallery)

    # #--------------------------------------------------------------------------
    # def load_partition(self, split_id=0, verbose=True):
    #     """
    #     Loads a given partition number from the partitions loaded from
    #     self.partition_path
    #     """
    #     if self.partitions is None:
    #         self.partitions = load_matlab_partition(self.partition_path)

    #     cam_a_imgs = glob.glob(osp.join(self.cam_a_path, '*.bmp'))
    #     cam_b_imgs = glob.glob(osp.join(self.cam_b_path, '*.bmp'))
    #     self.img_list = cam_a_imgs + cam_b_imgs

    #     ImgInfo = namedtuple('ImgInfo', 'path person_id cam_id img_id')
    #     imgs_info = [ImgInfo(img,                    # Image filename
    #                          id % 632,               # Person id
    #                          1 if id < 632 else 2,   # Camera id
    #                          id)                     # Image id
    #                  for id, img in enumerate(self.img_list)]

    #     idx_train = self.partitions[split_id]['idx_train']
    #     self.train = [imgs_info[id]
    #                   for id, train in enumerate(idx_train) if train == 1]

    #     idx_test = self.partitions[split_id]['idx_test']
    #     self.test = [imgs_info[id]
    #                  for id, test in enumerate(idx_test) if test == 1]

    #     self.idx_probe = self.partitions[split_id]['idx_probe']
    #     self.probe = [self.test[id]
    #                   for id, probe in enumerate(self.idx_probe[0])
    #                   if probe == 1]

    #     self.idx_gallery = self.partitions[split_id]['idx_gallery']
    #     self.gallery = [self.test[id]
    #                     for id, gal in enumerate(self.idx_gallery[0])
    #                     if gal == 1]

    #     ix_pos_pair = self.partitions[split_id]['ix_pos_pair']
    #     self.pos_pairs = [tuple(item) for item in ix_pos_pair]

    #     ix_neg_pair = self.partitions[split_id]['ix_neg_pair']
    #     self.neg_pairs = [tuple(item) for item in ix_neg_pair]

    #     if verbose:
    #         print("Training statistics:")
    #         print("  ------------------------------")
    #         print("  subset   | # ids | # images")
    #         print("  ------------------------------")
    #         print("  train    | {:5d} | {:8d}".format(len(self.train)//2,
    #                                                   len(self.train)))
    #         print("  query    | {:5d} | {:8d}".format(len(self.test)//2,
    #                                                   len(self.test)))
    #         print("  ------------------------------")

    #--------------------------------------------------------------------------
    # def _download_data(self) -> None:
    #     """ Downloads the dataset if the directory is not on disk."""
    #     dataset_path = osp.join(self.dataset_dir, 'VIPeR')
    #     if osp.exists(dataset_path):
    #         return

    #     print("VIPeR dataset not found at '{}'".format(dataset_path))
    #     if not osp.exists(self.dataset_dir):
    #         print("Creating directory {}".format(self.dataset_dir))
    #         iotools.mkdir_if_missing(self.dataset_dir)

    #     print("Downloading VIPeR dataset...")
    #     viper_zipfile = osp.join(self.dataset_dir,
    #                              osp.basename(self.dataset_url))
    #     urllib.request.urlretrieve(self.dataset_url, viper_zipfile)

    #     print("Extracting files...")
    #     zip_ref = zipfile.ZipFile(viper_zipfile, 'r')
    #     zip_ref.extractall(self.dataset_dir)
    #     zip_ref.close()
    #     os.remove(viper_zipfile)

    #--------------------------------------------------------------------------
    def _load(self, verbose: bool = True) -> None:
        """
        Validate that images are on disk. Load the image list and the
        training/testing split sets.
        """
        if not osp.exists(self.dataset_dir):
            raise RuntimeError(
                "'{}' is not available".format(self.dataset_dir))
        if not osp.exists(self.cam_a_path):
            raise RuntimeError(
                "'{}' is not available".format(self.cam_a_path))
        if not osp.exists(self.cam_b_path):
            raise RuntimeError(
                "'{}' is not available".format(self.cam_b_path))

        #self._prepare_test_pairs(self.nb_pairs, self.nb_categories)

      #  self.pairs = iotools.read_json(self.test_pairs_filepath)
       # self.partitions = load_matlab_partition(self.partition_path)

        if verbose:
            print("VIPeR dataset:")
            # print("  --------------------")
            # print("  Folder   | # images")
            # print("  ---------------------")
            # print("  cam_a    | {:5d}".format(len(cam_a_list)))
            # print("  cam_b    | {:5d}".format(len(cam_b_list)))
            # print("  ---------------------")

            # split = self.get_partition(0)
            # train_pids = [img.person_id for img in split.train]
            # test_pids = [img.person_id for img in split.test]
            # probe_pids = [img.person_id for img in split.probe]
            # gal_pids = [img.person_id for img in split.gallery]

            # nb_train_imgs = len(train_pids)
            # nb_train_pids = len(set(train_pids))
            # nb_test_imgs = len(test_pids)
            # nb_test_pids = len(set(test_pids))

            # nb_probe_imgs = len(probe_pids)
            # nb_probe_pids = len(set(probe_pids))

            # nb_gal_imgs = len(gal_pids)
            # nb_gal_pids = len(set(gal_pids))

            # nb_total_imgs = nb_train_imgs + nb_test_imgs
            # nb_total_pids = nb_train_pids + nb_test_pids

            # print("Trainging/Testing split:")
            # print("  ------------------------------")
            # print("  subset   | # ids | # images")
            # print("  ------------------------------")
            # print("  train    | {:5d} | {:8d}".format(nb_train_pids,
            #                                           nb_train_imgs))
            # print("  query    | {:5d} | {:8d}".format(nb_probe_pids,
            #                                           nb_probe_imgs))
            # print("  gallery  | {:5d} | {:8d}".format(nb_gal_pids,
            #                                           nb_gal_imgs))
            # print("  ------------------------------")
            # print("  total    | {:5d} | {:8d}".format(nb_total_pids,
            #                                           nb_total_imgs))
            # print("  ------------------------------")


    #--------------------------------------------------------------------------
    def _prepare_test_pairs(self, nb_pairs, nb_categories) -> None:
        # if osp.exists(self.test_pairs_filepath):
        #     return

        print('Creating', nb_pairs, 'random pairs for',
              nb_categories, 'Camera angles')

        test_pairs = []

        
        cam_a_list = glob.glob(osp.join(self.cam_a_path, '*.bmp'))
        cam_b_list = glob.glob(osp.join(self.cam_b_path, '*.bmp'))
        # self.img_list = cam_a_list + cam_b_list
        categories = ['cam_a', 'cam_b']

        for cat_id, category in enumerate(categories):
            if cat_id == nb_categories:
                break
            # Get image filenames
            cam_a_list = glob.glob(osp.join(self.cam_a_path, '*.bmp'))

            # Generate all possible non-repeating pairs using
            nb_imgs = len(cam_a_list)
            pairs = [(self.reference_img, img) for img in cam_a_list]

            # TODO: annotations for some images are invalid, remove pairs that 
            # have invalid annotations 
            #cougar 32 (id:31)
            #wrench 10 (id:9)
 
            # Randomly shuffle these pairs
           # random.shuffle(pairs)
            # Select nb_pairs
            n = nb_pairs
            pairs = pairs[:n]

            #split = {'Category': category, 'pairs': pairs}
            test_pairs.append({'Category': category, 'pairs': pairs})

        # iotools.write_json(test_pairs, self.test_pairs_filepath)
        # print('Test pairs file saved to', self.test_pairs_filepath)

    # #--------------------------------------------------------------------------
    # def _prepare_split(self) -> None:
    #     if not osp.exists(self.split_path):
    #         print("Creating 10 random splits")

    #         cam_a_imgs = sorted(glob.glob(osp.join(self.cam_a_path, '*.bmp')))
    #         cam_b_imgs = sorted(glob.glob(osp.join(self.cam_b_path, '*.bmp')))
    #         assert len(cam_a_imgs) == len(cam_b_imgs)
    #         num_pids = len(cam_a_imgs)
    #         print("Number of identities: {}".format(num_pids))
    #         num_train_pids = num_pids // 2

    #         splits = []
    #         for _ in range(10):
    #             order = np.arange(num_pids)
    #             np.random.shuffle(order)
    #             train_idxs = order[:num_train_pids]
    #             test_idxs = order[num_train_pids:]
    #             assert not bool(set(train_idxs) & set(test_idxs)
    #                             ), "Error: train and test overlap"

    #             train = []
    #             for pid, idx in enumerate(train_idxs):
    #                 cam_a_img = cam_a_imgs[idx]
    #                 cam_b_img = cam_b_imgs[idx]
    #                 train.append((cam_a_img, pid, 0))
    #                 train.append((cam_b_img, pid, 1))

    #             test = []
    #             for pid, idx in enumerate(test_idxs):
    #                 cam_a_img = cam_a_imgs[idx]
    #                 cam_b_img = cam_b_imgs[idx]
    #                 test.append((cam_a_img, pid, 0))
    #                 test.append((cam_b_img, pid, 1))

    #             split = {'train': train, 'query': test, 'gallery': test,
    #                      'num_train_pids': num_train_pids,
    #                      'num_query_pids': num_pids - num_train_pids,
    #                      'num_gallery_pids': num_pids - num_train_pids
    #                      }
    #             splits.append(split)

    #         print("Totally {} splits are created".format(len(splits)))
    #         iotools.write_json(splits, self.split_path)
    #         print("Split file saved to {}".format(self.split_path))

    # #--------------------------------------------------------------------------
    # def _load_split(self, split_id: int = 0, verbose: bool = True) -> tuple:
    #     splits = iotools.read_json(self.split_path)
    #     if split_id >= len(splits):
    #         raise ValueError("split_id exceeds range, received {}, \
    #             but expected between 0 and {}".format(split_id, len(splits) - 1))
    #     split = splits[split_id]

    #     train = split['train']
    #     query = split['query']  # query and gallery share the same images
    #     gallery = split['gallery']

    #     train = [tuple(item) for item in train]
    #     query = [tuple(item) for item in query]
    #     gallery = [tuple(item) for item in gallery]

    #     num_train_pids = split['num_train_pids']
    #     num_query_pids = split['num_query_pids']
    #     num_gallery_pids = split['num_gallery_pids']

    #     num_train_imgs = len(train)
    #     num_query_imgs = len(query)
    #     num_gallery_imgs = len(gallery)

    #     num_total_pids = num_train_pids + num_query_pids
    #     num_total_imgs = num_train_imgs + num_query_imgs

    #     if verbose:
    #         print("=> VIPeR loaded")
    #         print("Dataset statistics:")
    #         print("  ------------------------------")
    #         print("  subset   | # ids | # images")
    #         print("  ------------------------------")
    #         print("  train    | {:5d} | {:8d}".format(num_train_pids,
    #                                                   num_train_imgs))
    #         print("  query    | {:5d} | {:8d}".format(num_query_pids,
    #                                                   num_query_imgs))
    #         print("  gallery  | {:5d} | {:8d}".format(num_gallery_pids,
    #                                                   num_gallery_imgs))
    #         print("  ------------------------------")
    #         print("  total    | {:5d} | {:8d}".format(num_total_pids,
    #                                                   num_total_imgs))
    #         print("  ------------------------------")

    #     return (train,
    #             query,
    #             gallery,
    #             num_train_pids,
    #             num_query_pids,
    #             num_gallery_pids)
