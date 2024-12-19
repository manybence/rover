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


def m_mode_imaging(path, template):

    # Define pulsation threshold TODO: more specific number, reasoning
    pulsation_threshold = 0.005
    
    time, frame_org, xpos = fh.load_m_image(path)

    # Image processing
    image_proc = proc.preprocess_image(frame_org, template)
    if (proc.detect_noise(image_proc)):
        warnings.warn("M-mode image is corrupted. Check for gain and filtering settings!")

    # Detect pulsation
    energies = []
    for segment in image_proc:    
        energies.append(proc.measure_energy(segment))

    # Locate the artery
    depth = -1
    if max(energies) > pulsation_threshold:
        depth = energies.index(max(energies)) * 1000 * proc.wave_velocity / proc.sampling_freq
        
    # Display M-mode image
    notification = "No pulsation detected"
    proc.display(image_proc, time, f"M-mode at XPOS: {xpos} mm")
    if depth > -1:
        plt.axhline(y=depth, color='red', linestyle='--', linewidth=1)
        notification = f"Pulsation detected at depth: {round(depth, 2)} mm"

    title = f"M-mode at XPOS: {xpos} mm, "
    plt.title(title + notification)

    return depth, energies, xpos

if __name__ == "__main__":

    #Setting the font size
    plt.rcParams.update({'font.size': 22})

    #Matched filter template path
    path_template = "match_filter/template_5_MHz.csv"

    pulsation_threshold = 10
            
    # Load matched filter template
    template = fh.load_match_template(path_template)

    # Loading the M-mode scan
    path = fh.find_file()

    m_mode_imaging(path, template)

    
    
    

    

        
    
        


