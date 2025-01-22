# -*- coding: utf-8 -*-
"""
Created on Tue Feb 13 18:05:01 2024

@author: Bence Many

File handling functions 
"""
from tkinter import Tk
from tkinter.filedialog import askopenfilename
from datetime import datetime
import csv
import numpy as np
import pandas as pd
import os
import json
import glob
import re

projectfolder = '/home/rapid/rover'
default_path = os.path.join(projectfolder,'app/lib_gui/default_parameters.json')
match_template_path = os.path.join(projectfolder,'app/lib_gui/match_filter/filter_5_MHz_1_pulse.csv')

def find_file():
        
    #Open file dialog to pick the desired BoM
    root = Tk()
    root.attributes('-topmost', True)  # Display the dialog in the foreground.
    root.iconify()  # Hide the little window.
    file_path = askopenfilename(title='Select M-mode data', parent=root, filetypes=[("CSV files", "*.csv")])
    print("File selected: ", file_path)
    root.destroy()  # Destroy the root window when folder selected.
    return file_path

def save_results(path, results):

    #Current time for naming the file
    now = datetime.now()
    current_time = now.strftime("%H_%M_%S")
    
    #Opening new file and saving result: 1 file / row
    f = open(path + "\\" + current_time + ".csv", 'w')
    writer = csv.writer(f, delimiter = ';', lineterminator = '\n')
    writer.writerow(["file name", "label", "classification", "amplitude", "period", "confidence"])
    for line in results:
        writer.writerow(line)
    f.close()

def load_b_image(path):

    #Load M-mode ultrasound scan picture from csv file
    df = pd.read_csv(path)

    # Extract the XPOS column
    xpos = df.iloc[:, 0]

    # Extract the data columns
    signals = df.iloc[:, 5:] 
    
    return signals, xpos

def load_m_image(path):
    
    #Load M-mode ultrasound scan picture from csv file
    df = pd.read_csv(path)

    # Extract the XPOS
    xpos = df.iloc[:, 0][0]
    
    # Extract the time column 
    time_range = df.iloc[:, 2]

    # Extract the data columns
    signals = df.iloc[:, 5:] 
    
    return time_range, signals, xpos
    
def load_match_template():
    
    try:
        with open(match_template_path, mode='r') as file:
            reader = csv.reader(file)
            for row in reader:
                float_list = [float(value) for value in row]
            return float_list
    except:
        print("Opening match filter template has failed.")
        return []
    
def read_default_values():
    
    # Read dictionary of default values from JSON file
    try:
        with open(default_path, 'r') as f:
            defaults = json.load(f)
        return defaults
    except Exception as e:
        print(f"An error occurred: {e}")
        return {} 
    
def find_latest_file(directory, extension="csv"):
    path = os.path.join(directory, f'*.{extension}')
    files = glob.glob(path)
    if not files:
        print(f"File does not exist: {path}")
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
        raise FileNotFoundError(f"Unrecognized naming convention for file: {latest_file}")

    base_filename = base_filename_match.group(1)

    # Numerical sorting finction
    def sort_key(file_path):
        match = re.search(r"log\d+_(\d+)", file_path)
        return int(match.group(1)) if match else 0

    # Find all files in the same set, then sort numerically
    set_files = sorted(
        [f for f in files if re.match(rf'{re.escape(base_filename)}(?:_\d+)?\.csv$', os.path.basename(f))],
        key=sort_key)
    
    return set_files, os.path.join(directory, base_filename)