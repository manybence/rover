import glob
import os
import subprocess
import pandas as pd
import numpy as np
import scipy.signal as signal
import matplotlib.pyplot as plt
from nicegui import app, ui
import json

# Define the directory containing the files
FILES_DIRECTORY = '/home/rapid/projects/app/data/'
executable_path = '/home/rapid/projects/app/main'
default_path = '/home/rapid/projects/app/lib_gui/default_parameters.json'

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
    latest_file = max(files, key=os.path.getctime)
    return latest_file


def find_latest_data(directory):
    files = glob.glob(os.path.join(directory, '*.csv'))
    if not files:
        return None
    latest_file = max(files, key=os.path.getctime)
    return latest_file

        
def process_image():
    data_range = 4003
    file_path = find_latest_data(FILES_DIRECTORY)
    print(file_path)
    df = pd.read_csv(file_path)
    data_columns = [f'D[{i}]' for i in range(data_range)]
    data = df[data_columns].values
    envelopes = np.abs(signal.hilbert(data, axis=1))
    log_envelopes = np.log1p(envelopes)  # Use log1p to avoid log(0)
    normalized_envelopes = 255 * (log_envelopes - np.min(log_envelopes)) / (np.max(log_envelopes) - np.min(log_envelopes))
    bitmap = normalized_envelopes.astype(np.uint8)
    plt.imshow(bitmap.T, cmap='gray', aspect='auto')
    plt.axis('off') 
    plt.savefig(file_path.replace('.csv', '.png').replace('//dat', '//pic'), format='png', bbox_inches='tight', pad_inches=0)
    
    
def run_cpp_program(current_values):
    # Convert boolean AUTOGAIN to string 'true' or 'false'
    A_MODE_autogain_str = 'true' if current_values['A_MODE_AUTOGAIN'] else 'false'
    DOPPLER_autogain_str = 'true' if current_values['DOPPLER_AUTOGAIN'] else 'false'

    # Prepare the command with arguments
    command = ['sudo', executable_path] + [str(value) for value in current_values.values()]

    try:
        # Run the command
        subprocess.run(command, check=True)
        process_image()
    except subprocess.CalledProcessError as e:
        ui.notify(f'Error running program: {e}')
        
    