import glob
import os
import subprocess
import pandas as pd
import numpy as np
import scipy.signal as signal
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from nicegui import app, ui
import json
from lib_gui import file_handling as fh
from lib_gui import m_mode_detection as md
from lib_gui import b_mode_detection as bd
from lib_gui import signal_processing as proc
import psutil
import re


# Define the directory containing the files
FILES_DIRECTORY = '/home/rapid/projects/rover/log/'
executable_path = '/home/rapid/projects/rover/app/main'
default_path = '/home/rapid/projects/rover/app/lib_gui/default_parameters.json'
match_template_path = "/home/rapid/projects/rover/app/lib_gui/match_filter/template_5_MHz.csv"

def read_default_values():
    
    # Read dictionary of default values from JSON file
    try:
        with open(default_path, 'r') as f:
            defaults = json.load(f)
        return defaults
    except Exception as e:
        print(f"An error occurred: {e}")
        return {} 

def find_latest_bitmap(directory):
    files = glob.glob(os.path.join(directory, '*.png'))
    if not files:
        return None
    
    # Sort files alphabetically and return the last one
    latest_file = sorted(files)[-1]
    return latest_file

def find_latest_set_data(directory):

    # Find all the csv files in the folder
    files = glob.glob(os.path.join(directory, '*.csv'))
    if not files:
        raise FileNotFoundError("No CSV files found in directory.")
    
    # Sort files alphabetically and find the last one
    latest_file = sorted(files)[-1]

    # Extract base filename (without _X if it’s part of a set)
    base_filename_match = re.match(r"(log\d+)(_?\d*)\.csv", os.path.basename(latest_file))
    if not base_filename_match:
        raise FileNotFoundError("The naming convention is not recognized in the last file.")

    base_filename = base_filename_match.group(1)

    # Find all files in the same set
    set_files = sorted([f for f in files if re.match(rf'{re.escape(base_filename)}(?:_\d+)?\.csv$', os.path.basename(f))])
    
    return set_files, os.path.join(directory, base_filename)

def find_latest_data(directory):
    files = glob.glob(os.path.join(directory, '*.csv'))
    if not files:
        return None
    
    # Sort files alphabetically and return the last one
    latest_file = sorted(files)[-1]
    return latest_file
        
def process_b_mode_image():
    try:
        file_path = find_latest_data(FILES_DIRECTORY)
        print("Processing B-mode image: ", file_path)

        bd.b_mode_imaging(file_path)
        plt.savefig(file_path.replace('.csv', '.png').replace('//dat', '//pic'), format='png', bbox_inches='tight', pad_inches=0)
        plt.close()
    except Exception as e:
        print(f"Error while processing image: {e}")
    
def process_m_mode_image():

    # Find latest image
    try:
        file_path = find_latest_data(FILES_DIRECTORY)
        print("Processing M-mode image: ", file_path)
               
        # Load matched filter template
        template = fh.load_match_template(match_template_path)

        # Perform imaging
        depth = md.m_mode_imaging(file_path, template)
        plt.savefig(file_path.replace('.csv', '.png').replace('//dat', '//pic'), format='png', bbox_inches='tight', pad_inches=0)
        plt.close()
    except Exception as e:
        print(f"Error while processing image: {e}")

def process_m_mode_scan():

    try:
        # Find latest images
        file_paths, name_match = find_latest_set_data(FILES_DIRECTORY)

        # Load matched filter template
        template = fh.load_match_template(match_template_path)
        
        # Process each image
        list_of_energies = []
        positions = []
        for file_path in file_paths:
            print("Processing M-mode image: ", file_path)

            # Perform imaging
            depth, energies, xpos = md.m_mode_imaging(file_path, template)
            list_of_energies.append(energies)
            positions.append(xpos)
            plt.savefig(file_path.replace('.csv', '.png').replace('//dat', '//pic'), format='png', bbox_inches='tight', pad_inches=0)
            plt.close()

        # Plot energy distribution across scan area
        title = f"Full M-mode scan at XPOS:"
        energy_map = np.array(list_of_energies).T
        proc.display_full_scan(energy_map, positions, title)
        png_file_path = f"{name_match}_set.png"
        print("New scan image: ", png_file_path)
        plt.savefig(png_file_path.replace('//dat', '//pic'), format='png', bbox_inches='tight', pad_inches=0)
        plt.close()

    except Exception as e:
        print(f"Error while processing image: {e}")
    
def run_cpp_program(current_values, is_configured):
    # Convert boolean AUTOGAIN to string 'true' or 'false'
    A_MODE_autogain_str = 'true' if current_values['A_MODE_AUTOGAIN'] else 'false'
    DOPPLER_autogain_str = 'true' if current_values['DOPPLER_AUTOGAIN'] else 'false'

    # Prepare the command with arguments
    command = ['sudo', executable_path] + [str(value) for value in current_values.values()] + [str(is_configured)]

    try:
        # Run the command
        subprocess.run(command, check=True)
    except subprocess.CalledProcessError as e:
        ui.notify(f'Error running program: {e}')
        
def release_port(port):
    for proc in psutil.process_iter(['pid', 'name', 'connections']):
        connections = proc.info.get('connections', [])
        if connections:  # Only proceed if there are connections to check
            for conn in connections:
                if conn.laddr.port == port:
                    proc.terminate()  # Terminate the process holding the port
                    return
                