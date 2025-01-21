# -*- coding: utf-8 -*-
"""

@author: Bence Mány, Richard Thulstrup

Library for B-mode ultrasound detection functions

"""
import os
import sys
import matplotlib.pyplot as plt
from lib_gui import signal_processing as proc
from lib_gui import file_handling as fh
import warnings
import numpy as np
import pandas as pd
from scipy.signal import hilbert, butter, filtfilt, detrend
from scipy.ndimage import median_filter
from PIL import Image

# Match filter template for 10 MHz signal
pulse_estimate = [ 191.88044306, 101.13246905,  -31.99125607, -125.13920687, -120.15156195, \
                   -23.58481095,   98.66428005,  166.59694768,  136.93185179,   26.74466111, \
                   -99.5099067,  -166.94983779, -130.9845268,    -4.09250074,  144.78855588, \
                   223.96668827,  174.30050933,   14.01003687, -163.07774791, -245.16313775, \
                  -180.9932315,   -19.51497124,  125.44213571,  161.78157986,   87.6488038,  \
                   -15.19937815,  -56.41633166,  -14.28623576,   52.00505903,   63.34806533, \
                    -2.29865019,  -87.60648695, -108.7888121,   -35.77163949,   73.57575599, \
                   117.88614223,   38.61021823, -125.59411997, -263.43376617, -275.38307592]

def b_mode_imaging(path):

    BMH = 250
    BMW = 250
    
    # Read data file
    data = pd.read_csv(path)
    xpos = data['XPOS'].values
    signals = data.iloc[:, 5:].values
    detrended_signals = detrend(signals, axis=1)

    # Image processing
    lowcut = 4.4e6  # Lower bound of the bandpass filter
    highcut = 5.5e6  # Upper bound of the bandpass filter
    filtered_signals = proc.bandpass_filtering(detrended_signals, lowcut, highcut, fs=60e6)
    enhanced_signals = proc.apply_matched_filter(filtered_signals, pulse_estimate)
    envelope_signals = np.abs(hilbert(enhanced_signals, axis=1))
    input_min, input_max = 70000, 700000
    remapped_image = proc.linear_remap(envelope_signals, input_min, input_max)
    filtered_image = median_filter(remapped_image, size = 10) 

    depth = np.linspace(0, 51.385, remapped_image.shape[1])

    filtered_image_pil = Image.fromarray(filtered_image)
    resized_image = filtered_image_pil.resize((BMH, BMW))
    rotated_image = resized_image.rotate(-90)
    transformed_image = rotated_image.transpose(Image.FLIP_LEFT_RIGHT)
    
    # Display B-mode image
    plt.imshow(transformed_image, cmap='gray')    
    plt.xlabel('X Position (Scaled)')
    plt.ylabel('Depth (Scaled)')
    plt.show()

    notification = ""
    title = f"B-mode at XPOS: {min(xpos)} - {round(max(xpos), 1)} mm, "
    plt.title(title + notification)

    return

    
    
    

    

        
    
        


