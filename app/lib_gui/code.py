import cv2
import numpy as np
import matplotlib.pyplot as plt
from scipy.ndimage import median_filter

import pandas as pd
from scipy.signal import hilbert, butter, filtfilt, correlate, detrend
BMH = 250
BMW = 250
NCIRCLES = 10
# Bandpass filter function
def bandpass_filter(_data, _lowcut, _highcut, _fs, _order=4):
    nyquist = 0.5 * _fs
    low = _lowcut / nyquist
    high = _highcut / nyquist
    b, a = butter(_order, [low, high], btype='band')
    return filtfilt(b, a, _data, axis=1)

# Function to estimate pulse from strong reflections
def estimate_pulse(signal_segment):
    return signal_segment[:, 24:64].mean(axis=0)

def apply_matched_filter(_signals, _pulse_estimate):
    matched_filter = _pulse_estimate[::-1]
    return np.array([correlate(scan, matched_filter, mode='same') for scan in _signals])

# Linear intensity remapping function
def linear_remap(_image, _input_min, _input_max):
    # Clip the image to the input range
    clipped = np.clip(_image, _input_min, _input_max)
    # Remap the intensity values to the range [0, 255]
    remapped = 255 * (clipped - _input_min) / (_input_max - _input_min)
    return remapped

# Define the filter parameters
fs = 60e6  # Sampling frequency: 60 MHz

#ok filter

pulse_estimate = [ 191.88044306, 101.13246905,  -31.99125607, -125.13920687, -120.15156195, \
                   -23.58481095,   98.66428005,  166.59694768,  136.93185179,   26.74466111, \
                   -99.5099067,  -166.94983779, -130.9845268,    -4.09250074,  144.78855588, \
                   223.96668827,  174.30050933,   14.01003687, -163.07774791, -245.16313775, \
                  -180.9932315,   -19.51497124,  125.44213571,  161.78157986,   87.6488038,  \
                   -15.19937815,  -56.41633166,  -14.28623576,   52.00505903,   63.34806533, \
                    -2.29865019,  -87.60648695, -108.7888121,   -35.77163949,   73.57575599, \
                   117.88614223,   38.61021823, -125.59411997, -263.43376617, -275.38307592]
# Load the test file
test_file = './data/log0582.csv'
data = pd.read_csv(test_file)
xpos = data['XPOS'].values
signals = data.iloc[:, 5:].values
detrended_signals = detrend(signals, axis=1)

'''
#pretty but not good for finding stuff
lowcut = 1.5e6  # Lower bound of the bandpass filter
highcut = 9.5e6  # Upper bound of the bandpass filter
filtered_signals = bandpass_filter(detrended_signals, lowcut, highcut, fs)
enhanced_signals = apply_matched_filter(filtered_signals, pulse_estimate)
envelope_signals = np.abs(hilbert(enhanced_signals, axis=1))
input_min, input_max = 1200000, 10000000
remapped_image = linear_remap(envelope_signals, input_min, input_max)
filtered_image = median_filter(remapped_image, size=3)  # Adjust size for more or less filtering
'''
#not pretty (smeared) but good for finding stuff
lowcut = 4.4e6  # Lower bound of the bandpass filter
highcut = 5.5e6  # Upper bound of the bandpass filter
filtered_signals = bandpass_filter(detrended_signals, lowcut, highcut, fs)
enhanced_signals = apply_matched_filter(filtered_signals, pulse_estimate)
envelope_signals = np.abs(hilbert(enhanced_signals, axis=1))
input_min, input_max = 70000, 700000
remapped_image = linear_remap(envelope_signals, input_min, input_max)
filtered_image = median_filter(remapped_image, size = 10)  # Adjust size for more or less filtering



depth = np.linspace(0, 51.385, remapped_image.shape[1])

resized_image = cv2.resize(filtered_image, (BMH, BMW))
rotated_image = cv2.rotate(resized_image, cv2.ROTATE_90_CLOCKWISE)
transformed_image = cv2.flip(rotated_image, 1)  # Flip code 1 indicates mirroring along the y-axis
image_8bit = cv2.convertScaleAbs(transformed_image)
blurred_image = cv2.medianBlur(image_8bit, 5)
circles = cv2.HoughCircles(
    blurred_image,
    cv2.HOUGH_GRADIENT,
    dp        = 1.0,
    minDist   = 1,
    param1    = 60,  # Adjust for edge detection sensitivity
    param2    = 9,   # Lower this value to make detection more sensitive
    minRadius = 7,   # Smaller circles included
    maxRadius = 20   # Adjust based on your image size
)

if circles is not None:
    circles = np.round(circles[0, :]).astype("int")
    circle_candidates = []

    # Evaluate each circle and store the candidates based on darkness
    for (x, y, r) in circles:
        mask = np.zeros_like(image_8bit)
        cv2.circle(mask, (x, y), r, 255, thickness = -1)
        masked_image = cv2.bitwise_and(image_8bit, image_8bit, mask=mask)
        mean_intensity = cv2.mean(image_8bit, mask=mask)[0]

        # Store the circle along with its darkness score
        circle_candidates.append((x, y, r, r * r * mean_intensity))
    # Sort the circles by darkness (lower intensity is darker)
    circle_candidates.sort(key=lambda x: x[3])

    # Select the top 3 (or more) candidates
    top_candidates = circle_candidates[:NCIRCLES]  # Adjust this value if you need more candidates

    # Plot the transformed image and overlay the top candidate circles
    fig, ax = plt.subplots(figsize=(6, 6))  # Keep the figure square for 300x300 display
    
    resized_image = cv2.resize(filtered_image, (BMH, BMW))
    rotated_image = cv2.rotate(resized_image, cv2.ROTATE_90_CLOCKWISE)
    transformed_image = cv2.flip(rotated_image, 1)  # Flip code 1 indicates mirroring along the y-axis
    ax.imshow(transformed_image, cmap='gray')
    i = 1
    for candidate in top_candidates:
        x, y, r, _ = candidate
        circle = plt.Circle((x, y), r, color='red', fill=False, lw=2)
        ax.add_patch(circle)
        label = "{:1d}".format(i)
        #ax.plot(x, y, 'ro')  # Cross at the center
        ax.annotate(label, (x,y), color='red', textcoords="offset points", xytext=(0,-4), ha='center')
        i = i + 1
    # Restore axis labels, scale, and color bar
    ax.set_xlabel('X Position (Scaled)')
    ax.set_ylabel('Depth (Scaled)')
    ax.set_title('Transformed B-Mode Ultrasound Image with Detections')
    plt.show()
else:
    print("No circles detected.")