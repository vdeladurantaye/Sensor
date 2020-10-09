"""
Utility functions for displaying info
"""
import os.path as osp
import numpy as np
import cv2 as cv
import math
import colorsys


#------------------------------------------------------------------------------
def DisplayListAsImage(title, data, width, height):
    img = np.reshape(data, (width, height)).T
    img = img*255
    img = img.astype(np.uint8)

    scale_factor = 2.5
    scaled_size = (
        int(img.shape[1]*scale_factor), 
        int(img.shape[0]*scale_factor))
    img = cv.resize(img, scaled_size)
    
    im_color = cv.applyColorMap(img, cv.COLORMAP_HOT)
    
    cv.imshow(title, im_color)


#------------------------------------------------------------------------------
def DisplayDspGrid(title, img1, img2, lvl, translations, trans_ids):
    scale_factor = 1

    img1_width = img1.shape[1]
    img1_height = img1.shape[0]

    grid_square = 2**(lvl-1)
    grid_size = grid_square*grid_square

    grid_width = math.floor(img1_width/grid_square)
    grid_height = math.floor(img1_height/grid_square)

    node_id_start = int((4**(lvl-1)-1)/3)
    node_id_end = int((4**(lvl)-1)/3)

    trans_ids = trans_ids[node_id_start:node_id_end]

    for x in range(grid_square):
        for y in range(grid_square):
            x_start = x * grid_width
            y_start = y * grid_height
            grid_id = y*grid_square + x

            hsv_color = grid_id/grid_size
            color = colorsys.hsv_to_rgb(hsv_color, 1, 1)
            color = tuple(i * 255 for i in color)

            cv.rectangle(img1, (x_start, y_start), 
                ((x_start+grid_width-1), (y_start+grid_height-1)), 
                color, 1)

            trans_id = trans_ids[grid_id]
            dx = translations[trans_id*2]
            dy = translations[trans_id*2+1]

            x_start = int(x * grid_width * scale_factor + dx)
            y_start = int(y * grid_height * scale_factor + dy)
            x_end = int(x_start + grid_width * scale_factor-1)
            y_end = int(y_start + grid_height * scale_factor-1)

            cv.rectangle(img2, (x_start, y_start), 
                (x_end,y_end), 
                color, 1)

    cv.imshow(title, img1)
    cv.imshow(title+"2", img2)
    
    
#------------------------------------------------------------------------------
def DisplayOdlmDspGrid(title, img1, img2, lvl, translations):
    scale_factor = 1

    img1_width = img1.shape[1]
    img1_height = img1.shape[0]

    grid_square = 2**(lvl-1)
    grid_size = grid_square*grid_square

    grid_width = math.floor(img1_width/grid_square)
    grid_height = math.floor(img1_height/grid_square)

    node_id_start = int((4**(lvl-1)-1)/3)
    node_id_end = int((4**(lvl)-1)/3)

    for x in range(grid_square):
        for y in range(grid_square):
            x_start = x * grid_width
            y_start = y * grid_height
            grid_id = node_id_start + x*grid_square + y

            hsv_color = grid_id/grid_size
            color = colorsys.hsv_to_rgb(hsv_color, 1, 1)
            color = tuple(i * 255 for i in color)

            cv.rectangle(img1, (x_start, y_start), 
                ((x_start+grid_width-1), (y_start+grid_height-1)), 
                color, 1)

            dx = translations[grid_id][0] - x_start
            dy = translations[grid_id][1] - y_start

            x_start = int(x * grid_width * scale_factor + dx)
            y_start = int(y * grid_height * scale_factor + dy)
            x_end = int(x_start + grid_width * scale_factor-1)
            y_end = int(y_start + grid_height * scale_factor-1)

            cv.rectangle(img2, (x_start, y_start), 
                (x_end,y_end), 
                color, 1)

    cv.imshow(title, img1)
    cv.imshow(title+"2", img2)
    

