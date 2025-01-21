# -*- coding: utf-8 -*-
"""
Created on Sun May  7 01:39:14 2023

@author: Bence Mány

Library for M-mode ultrasound detection functions

"""

import matplotlib.pyplot as plt
from lib_gui import signal_processing as proc
from lib_gui import file_handling as fh
import warnings
import numpy as np
from scipy.signal import convolve

def classify_sample(energy, frequency):
    
    # Binary classification on a sample based on the evaluation criteria
    energy_min = 0.9
    energy_max = 25
    freq_min = 0.7
    freq_max = 1.5
    
    if energy_min < energy * 10e6 < energy_max and freq_min < frequency < freq_max:
        return True
    else: 
        return False
    
def m_mode_imaging(path, template):
    
    time, frame_org, xpos = fh.load_m_image(path)

    # Image processing
    image_proc = proc.preprocess_image(frame_org, template)
    # if (proc.detect_noise(image_proc)):
    #     warnings.warn("M-mode image is corrupted. Check for gain and filtering settings!")

    # Measure absolute difference between A-lines
    rotated = image_proc.T
    diff = []
    for t in range(1, len(rotated)):
        diff.append(abs(rotated[t] - rotated[t-1]))
    diff = np.array(diff).T
    
    # Threshold the differential image
    threshold_high = 0.0
    diff[diff < threshold_high] = 0

    # Measure average difference across depth axis
    vertical_averages = np.mean(diff, axis=0)
    
    # Processing the pulsation
    window_size = 30
    window = np.hanning(window_size)
    window /= window.sum()  # Normalize the window
    mean_centered_signal = vertical_averages - np.mean(vertical_averages) 
    hann_filtered_signal = convolve(mean_centered_signal, window, mode='same')
    
    # Measure freqency and amplitude of pulsation
    energy, freq = proc.measure_energy(hann_filtered_signal)
    print(f"Measured energy: {round(energy*10e6, 2)}, Frequency: {round(freq, 2)} Hz")

    # Display processed M-mode image
    proc.display(image_proc, time, f"M-mode at XPOS: {xpos} mm")

    pulsation = classify_sample(energy, freq)

    title = f"M-mode at XPOS: {xpos} mm, pulsation: {pulsation}"
    plt.title(title)

    return energy, freq


    
    
    

    

        
    
        


