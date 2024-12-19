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

def load_match_template2(path):

    #Load match filter template file
    try:
        us_file = open(path)
        template = np.loadtxt(us_file, delimiter=",")
        print("Reading match filter template done")
        return template
    except:
        print("Opening match filter template has failed.")
        return []
    
def load_match_template(file_path):
    
    try:
        with open(file_path, mode='r') as file:
            reader = csv.reader(file)
            for row in reader:
                float_list = [float(value) for value in row]
            return float_list
    except:
        print("Opening match filter template has failed.")
        return []