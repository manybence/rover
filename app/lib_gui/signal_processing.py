# -*- coding: utf-8 -*-
"""
Created on Wed Jun 14 11:46:04 2023

@author: Bence Mány

Algorithm development for ultrasound-based robotic artery detection 
(RAPID project)

Library for common processing functions for B-mode and M-mode detection
"""

import numpy as np
from scipy import signal
from scipy.signal import hilbert, correlate
import matplotlib.pyplot as plt

sampling_freq = 60e6
wave_velocity = 1480
max_depth_mm = 40  # maximum depth in mm
max_depth_sample = int(max_depth_mm * sampling_freq / wave_velocity / 1000)

def segmentate_image(image, distance_range):
    
    #Erasing the upper part of the image by replacing with mean value
    mean = np.mean(image)
    image_top = np.zeros(image[:min(distance_range)].shape)
    for row in range(len(image_top)):
        for i in range(len(image_top[row])):
            image_top[row, i] = mean

    #Cutting off the bottom region
    image_bottom = image[min(distance_range):max(distance_range)]
    
    #Assembling the image
    segmented_image = np.concatenate((image_top, image_bottom), axis = 0)
    
    return segmented_image

def bandpass_filtering(series, cutoff_low, cutoff_high, order=5, fs=60e6):   
    
    #Defining the filter values
    nyq = 0.5 * fs
    normal_cutoff_low = cutoff_low / nyq
    normal_cutoff_high = cutoff_high / nyq
    
    #Applying the band-pass filter
    b, a = signal.butter(order, [normal_cutoff_low, normal_cutoff_high], btype='bandpass', analog=False)
    filtered_signal = signal.filtfilt(b, a, series)
    
    return filtered_signal

def log_mapping(image):
    c = 255 / np.log(1 + np.max(image))
    log_image = c * (np.log(image + 1))
    return log_image

def apply_matched_filter(_signals, _pulse_estimate):
    matched_filter = _pulse_estimate[::-1]
    return np.array([correlate(scan, matched_filter, mode='same') for scan in _signals])

def preprocess_image(image, template):
    
    #Applying signal processing steps
    signals = []
    for index, row in image.iterrows():
        signal_raw = np.array(row.values)[:max_depth_sample]
        filtered_signal = bandpass_filtering(signal_raw, cutoff_low = 4.7e6, cutoff_high = 5.8e6)  #Applying band-pass filter
        matched = signal.lfilter(template, 1, filtered_signal)  #Apply match filter
        envelopes = np.abs(hilbert(matched))   #Hilbert-transformation
        signals.append(envelopes)

    normalized = normalize_image(signals)
    # log_envelopes = np.log1p(signals)
    # normalized_envelopes = 255 * (log_envelopes - np.min(log_envelopes)) / (np.max(log_envelopes) - np.min(log_envelopes))
    # bitmap = normalized_envelopes
    bitmap = np.array(normalized)
    
    return bitmap.T

def normalize_image(image):
    all_values = [value for sublist in image for value in sublist]
    global_min = min(all_values)
    global_max = max(all_values)
    
    # Normalize each value using the global min and max
    normalized_data = [[(value - global_min) / (global_max - global_min) for value in sublist] for sublist in image]
    
    return normalized_data

def display(image, time, title):

    fig = plt.figure()
    time_in_seconds = [t / 1000000.0 for t in time]
    plt.imshow(image, extent=(min(time_in_seconds), max(time_in_seconds), image.shape[0] * 1000 * wave_velocity / sampling_freq, 0), aspect='auto', cmap='gray')    
    plt.title(title)
    plt.xlabel("Time (s)")
    plt.ylabel("Depth (mm)")

def display_full_scan(image, positions, title):
    intensity_max = 0.02
    fig = plt.figure()
    plt.imshow(image, 
               extent=(min(positions), max(positions), image.shape[0] * 1000 * wave_velocity / sampling_freq, 0), 
               aspect='auto', 
               cmap='gray',
               vmin=0,
               vmax=intensity_max)    
    plt.title(title)
    plt.xlabel("XPOS (mm)")
    plt.ylabel("Depth (mm)")
    
def plot_signal_analysis(signal):
    
    plt.figure(figsize=(12, 6))
    plt.subplot(1, 2, 1)
    plt.plot(signal)
    plt.title("Signal")
    plt.xlabel('Sample Index')
    plt.ylabel('Amplitude')
     
    # Compute FFT of the signal
    signal_fft = np.fft.fft(signal)
    freqs = np.fft.fftfreq(len(signal), d=1/sampling_freq)
     
    # Take only the positive half of the FFT (real signal)
    positive_freqs = freqs[:len(freqs)//2]
    positive_fft = np.abs(signal_fft[:len(signal_fft)//2])
     
    # Plot the FFT of the 10th row signal
    plt.subplot(1, 2, 2)
    plt.plot(positive_freqs, positive_fft)
    plt.title("FFT of signal")
    plt.xlabel('Frequency (Hz)')
    plt.ylabel('Magnitude')
     
    # Show both plots
    plt.tight_layout()
    plt.show()
    
def measure_energy(signal):
    
    # Define frequency range of artery pulsation (Hz)
    lower_bound = 0.5
    upper_bound = 1.8
    
    # Compute FFT of the signal
    signal_fft = np.fft.fft(signal)
    freqs = np.fft.fftfreq(len(signal), d=1/50)
     
    # Take only the positive half of the FFT (real signal)
    positive_freqs = freqs[:len(freqs)//2]
    positive_fft = np.abs(signal_fft[:len(signal_fft)//2])
    indices = np.where((positive_freqs >= lower_bound) & (positive_freqs <= upper_bound))
    
    # Measure energy component
    extracted_fft = positive_fft[indices]
    extracted_fft_norm = extracted_fft / len(signal)        # Normalize amplitude to make energy independent of signal length
    energy = sum(value ** 2 for value in extracted_fft_norm)
    
    return energy

def detect_noise(image, plot_histogram=False):
    
    threshold = 1e-4
    
    # Calculate the histogram for pixel values in the range [0, 1] with 256 bins
    hist, bin_edges = np.histogram(image, bins=256, range=(0.0, 1.0))

    # Normalize the histogram
    hist = hist / hist.sum()

    # Plot the histogram if requested
    if plot_histogram:
        plt.figure()
        plt.title("Histogram (Pixel Range: 0-1)")
        plt.xlabel("Pixel Intensity (0-1)")
        plt.ylabel("Frequency")
        plt.plot(bin_edges[:-1], hist)
        plt.xlim([0, 1])
        plt.show()
        
    # Measure histogram flatness by calculating the variance
    variance = np.var(hist)
    
    # A low variance suggests a flat (noisy) image
    return variance < threshold

def linear_remap(_image, _input_min, _input_max):
    clipped = np.clip(_image, _input_min, _input_max)   # Clip the image to the input range
    remapped = 255 * (clipped - _input_min) / (_input_max - _input_min) # Remap the intensity values to the range [0, 255]
    return remapped