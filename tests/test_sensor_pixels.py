
import cv2 as cv
import pytest

from sensor import cpp_sensor
from sensor import sensor

#------------------------------------------------------------------------------
def test_misc():
    """
    Test single image segmentation
    """
    image_filename = 'carGray.bmp'
    cpp_sensor.SetWorkingDir('tests/WorkingDir')
        
    sensor_config = sensor.ReadConfig('tests/WorkingDir/SensorPixel.ini')
    
    sensor_config['PixelsParams']['PIXEL_RANDOM_INIT'] = True
    sensor_config['SimulationParams']['SEG_TRIGGER_SAME_LABEL_NEURONS'] = False
    sensor_config['SimulationParams']['SEG_MERGE_SEGMENTS'] = False

    cpp_sensor.SetConfig(sensor_config)
    
    pixel = cpp_sensor.PixelLayer(image_filename)
    monitor = cpp_sensor.SegLayerMonitor("Test", pixel)
    #cpp_sensor.AddDebugger("Debug", pixel)

    pixel.SegmentLayer()
    
    print('\nCycles: {:d} \t Cascades: {:d} \t Spikes: {:d} \t Coef: {:1.4f}'
        .format(pixel.GetNbCycles(), 
                pixel.GetNbCascades(), 
                pixel.GetNbSpikes(),
                pixel.GetCoefStabilization()))

    segments = monitor.GetDisplay()
   
    cv.imshow('Seg', segments)
    cv.waitKey(1000)
    #cv.waitKey(0)
    
#------------------------------------------------------------------------------
def test_segmentation():
    """
    Test single image segmentation
    """
    cpp_sensor.SetWorkingDir('tests/WorkingDir')
    cpp_sensor.LoadConfigFile('SensorPixel.ini')

    image_filename = 'carGray.bmp'
    pixel = cpp_sensor.PixelLayer(image_filename)
    monitor = cpp_sensor.SegLayerMonitor("Test", pixel)
    #cpp_sensor.AddDebugger("Debug", pixel)

    pixel.SegmentLayer()
    
    segments = monitor.GetDisplay()
   
    cv.imshow('Seg', segments)
    cv.waitKey(1000)
   