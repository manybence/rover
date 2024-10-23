# -*- coding: utf-8 -*-
"""
Created on Sun May  7 01:39:14 2023

@author: Bence Mány

DTU Master thesis: Algorithm development for ultrasound-based robotic artery detection 
(RAPID project)

Library for M-mode ultrasound detection functions

"""

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import glob
import os
import seaborn as sns
import lib.processing as proc
import lib.file_handling as fh
from scipy.signal import hilbert
from scipy import signal
import csv


#Setting the font size
plt.rcParams.update({'font.size': 22})

#Matched filter template path
path_template = "data/match_filter/template_5_MHz.csv"

pulsation_threshold = 10
    
  
if __name__ == "__main__":
      
    # Load matched filter template
    template = fh.load_match_template(path_template)
    
    # Loading the M-mode scan
    path = fh.find_file()
    time, frame_org, xpos = fh.load_m_image(path)
    
    # Image processing
    image_proc = proc.preprocess_image(frame_org, template)
    
    # Detect pulsation
    energies = []
    for segment in image_proc:    
        energies.append(proc.measure_energy(segment))
    
    # Locate the artery
    depth = -1
    if max(energies) > pulsation_threshold:
        depth = energies.index(max(energies)) * 1000 * proc.wave_velocity / proc.sampling_freq / 2
        
    # Display M-mode image
    notification = "No pulsation detected"
    proc.display(image_proc, time, f"M-mode at XPOS: {xpos} mm")
    if depth > -1:
        plt.axhline(y=depth, color='red', linestyle='--', linewidth=1)
        notification = f"Pulsation detected at depth: {round(depth, 2)} mm"
    
    title = f"M-mode at XPOS: {xpos} mm, "
    plt.title(title + notification)
    

    
    
    

    

        
    
        


