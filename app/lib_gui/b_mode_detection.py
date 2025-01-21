# -*- coding: utf-8 -*-
"""

@author: Bence Mány, Richard Thulstrup

Library for B-mode ultrasound detection functions

"""
from lib_gui import signal_processing as proc
from lib_gui import file_handling as fh

def b_mode_imaging(path, template):
    
    # Read data file
    frame_org, xpos = fh.load_b_image(path)

    # Process B-mode image
    image_proc = proc.preprocess_image(frame_org, template)

    # Display processed B-mode image
    proc.display_b_mode(image_proc, xpos, f"B-mode at XPOS: {min(xpos)}-{max(xpos)} mm")

    
    
    

    

        
    
        


