from nicegui import app, ui
from lib_gui import process_handlers as ph
import os
from fastapi.responses import FileResponse

def create_header():
    with ui.header().style('background-color: #C30010; color: white; text-align: center; padding: 1em;'):
        ui.label('RAPID ULTRASOUND UNIT 3 CONTROL PANEL').style('font-size: 2em; font-weight: bold;')
        

def create_status_label():
    with ui.card().style('width: 100%; max-width: 600px; margin: auto;'):
        return ui.label("STATUS: Standby").style('font-size: 20px; font-weight: bold; color: black; text-align: center; width: 100%;')

def create_display():
    with ui.card().style('width: 100%; max-width: 600px; margin: auto;'):
        ui.label('LATEST SCAN').style('font-weight: bold;')
        try:
            latest_bitmap = ph.find_latest_bitmap(ph.FILES_DIRECTORY)
            ui.label(latest_bitmap)
            if latest_bitmap:
                ui.image(latest_bitmap).style('width: 100%;')
            else:
                ui.notify('No bitmap found after scan.')
        except:
            ui.label("Error loading image.")
            
def available_files():
    with ui.card().style('width: 100%; max-width: 600px; margin: auto;'):
        with ui.expansion('FILES', icon='download').classes('w-full').style('font-weight: bold;'):
            ui.label('File Downloader')
            ui.separator()
            # Get a list of files in the directory
            files = os.listdir(ph.FILES_DIRECTORY)

            # Display the files with download links
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
