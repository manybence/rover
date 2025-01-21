from nicegui import app, ui
from lib_gui import process_handlers as ph
import os
from fastapi.responses import FileResponse
import socket
import re

image_element = None  # Placeholder for the image element
current_image_path = None  # Track the current image to detect changes

class Display:
    def __init__(self):

        image_path = ph.find_latest_bitmap(ph.FILES_DIRECTORY)

        with ui.card().style('width: 100%; max-width: 600px; margin: auto;'):
            if image_path:
                self.label = ui.label('LATEST SCAN: ' + image_path).style('font-weight: bold;')
                self.image = ui.image(image_path).style('width: 100%;')
                self.path = image_path
            else:
                self.label = ui.label("No image found").style('font-weight: bold;')
                self.image = ui.image()
                self.path = ""

    def update_image(self):
        
        # Find latest image file
        new_image_path = ph.find_latest_bitmap(ph.FILES_DIRECTORY)

        # Update image variables
        if not (new_image_path == self.path):
            print(f"New image: {new_image_path}")
            self.label.set_text(new_image_path)
            self.image.source = new_image_path
            self.image.force_reload()
            self.path = new_image_path

class Status:
    def __init__(self):

        with ui.card().style('width: 100%; max-width: 600px; margin: auto;'):
            self.label = ui.label("STATUS: Standby").style('font-size: 20px; font-weight: bold; color: black; text-align: center; width: 100%;')
        
    def set_status(self, new_status):

        self.label.set_text("STATUS: " + new_status)

        if new_status == "Scanning":
            self.label.style("color:yellow")
            ui.notify("Scanning started")
        elif new_status == "Scanning finished":
            self.label.style("color:green")
            ui.notify("Scanning finished")
        elif new_status == "Standby":
            self.label.style("color:black")
        
def create_header():
    # Get the system hostname
    hostname = socket.gethostname()
    # Try to find numeric part(s) in the hostname
    match = re.search(r'(\d+)', hostname)
    
    # If a numeric portion is found, use it; otherwise default to 'X'
    if match:
        unit_number = match.group(1)
    else:
        unit_number = 'X'  # Fallback if no digits are found

    with ui.header().style('background-color: #C30010; color: white; text-align: center; padding: 1em;'):
        ui.label(f'RAPID ULTRASOUND UNIT {unit_number} CONTROL PANEL') \
          .style('font-size: 2em; font-weight: bold;')
           
def available_files():
    with ui.card().style('width: 100%; max-width: 600px; margin: auto;'):
        with ui.expansion('FILES', icon='download').classes('w-full').style('font-weight: bold;'):
            ui.label('File Downloader')
            ui.separator()
            # Get a list of files in the directory
            files = os.listdir(ph.FILES_DIRECTORY)

            # Display the files with download links
            files.sort()
            for file in files:
                file_path = os.path.join(ph.FILES_DIRECTORY, file)
                if os.path.isfile(file_path):
                    ui.link(file, f'/download/{file}')

                @ui.page('/download/{file_name}')
                async def download(file_name: str):
                    file_path = os.path.join(ph.FILES_DIRECTORY, file_name)
                    if os.path.isfile(file_path):
                        return FileResponse(file_path, filename=file_name)
                    else:
                        return ui.label('File not found'), 404

