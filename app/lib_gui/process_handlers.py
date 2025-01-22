import os
import subprocess
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from nicegui import app, ui
from lib_gui import file_handling as fh
from lib_gui import m_mode_detection as md
from lib_gui import b_mode_detection as bd
import psutil

projectfolder = '/home/rapid/rover'

# Define the directory containing the files
FILES_DIRECTORY = os.path.join(projectfolder,'log/')
executable_path = os.path.join(projectfolder,'app/main')


def process_b_mode_image():
    try:
        file_path = fh.find_latest_file(FILES_DIRECTORY, "csv")
        if file_path:
            print("Processing B-mode image: ", file_path)

            # Load matched filter template
            template = fh.load_match_template()

            bd.b_mode_imaging(file_path, template)
            plt.savefig(file_path.replace('.csv', '.png').replace('//dat', '//pic'), format='png', bbox_inches='tight', pad_inches=0)
            plt.close()
        else:
            print("B-mode data file not found.")
    except Exception as e:
        print(f"Error while processing image: {e}")
    
def process_m_mode_image():

    # Find latest image
    try:
        file_path = fh.find_latest_file(FILES_DIRECTORY, "csv")
        if file_path:
            print("Processing M-mode image: ", file_path)
                
            # Load matched filter template
            template = fh.load_match_template()

            # Perform imaging
            md.m_mode_imaging(file_path, template)
            plt.savefig(file_path.replace('.csv', '.png').replace('//dat', '//pic'), format='png', bbox_inches='tight', pad_inches=0)
            plt.close()
        else:
            print("M-mode data file not found.")
    except Exception as e:
        print(f"Error while processing image: {e}")

def process_m_mode_scan():

    try:
        # Find latest images
        file_paths, name_match = fh.find_latest_set_data(FILES_DIRECTORY)

        # Load matched filter template
        template = fh.load_match_template()
        
        # Process each image
        list_of_energies = []
        positions = []
        for file_path in file_paths:
            print("Processing M-mode image: ", file_path)

            # Perform imaging
            energies, freq, xpos = md.m_mode_imaging(file_path, template)
            list_of_energies.append(energies*10e6)
            positions.append(xpos)
            plt.savefig(file_path.replace('.csv', '.png').replace('//dat', '//pic'), format='png', bbox_inches='tight', pad_inches=0)
            plt.close()

        # Plot energy distribution across scan area
        plt.figure()
        
        plt.plot(positions, list_of_energies)
        plt.title("Full M-mode scan")
        plt.ylabel("Energy")
        plt.xlabel("XPOS (mm)")
        png_file_path = f"{name_match}_set.png"
        plt.savefig(png_file_path.replace('//dat', '//pic'), format='png', bbox_inches='tight', pad_inches=0)
        plt.close()
        print("New scan image: ", png_file_path)

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
                