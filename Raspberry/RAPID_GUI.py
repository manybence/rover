from nicegui import app, ui
import os
from fastapi.responses import FileResponse
from lib_gui import process_handlers as ph
from lib_gui import components


if not os.path.exists(ph.FILES_DIRECTORY):
    os.makedirs(ph.FILES_DIRECTORY)

      
        
# Function to update the scanning status
def set_status(new_status):
    status_label.set_text("STATUS: " + new_status)
    if new_status == "Scanning":
        status_label.style("color:yellow")
        ui.notify("Scanning started")
    elif new_status == "Scanning finished":
        status_label.style("color:green")
        ui.notify("Scanning finished")
    elif new_status == "Standby":
        status_label.style("color:black")
        
def update_values():
    print("Scanning started")
    for key in current_values.keys():
        current_values[key] = eval(f'{key}_input').value
    app.storage.general['current_values'] = current_values
    set_status("Scanning")
    ph.run_cpp_program(current_values)
    print("Scanning finished")
    set_status("Scanning finished")        
        

# Ensure stored values include all keys from defaults
defaults = ph.read_default_values()
stored_values = app.storage.general.get('current_values', {})
current_values = {**defaults, **stored_values}
current_values = {k: current_values[k] for k in defaults.keys()}
    
# GUI setup
ui.page_title("RAPID GUI")

# Header
components.create_header()
    
# Status label
status_label = components.create_status_label()

# Scanning
with ui.card().style('width: 100%; max-width: 600px; margin: auto;'):
    ui.label('ACTION').style('font-weight: bold;').style('width: 100%;')
    MODE_input = ui.select(['A-MODE', 'DOPPLER', 'M-MODE', 'NEEDLE'], label='MODE OF OPERATION', value=current_values['MODE']).style('width: 33%;')
    NEEDLEPOS_input = ui.number(label='NEEDLE DEPTH (mm)', value=current_values['NEEDLEPOS'], min=current_values['ZPOSMIN'], max=current_values['ZPOSMAX'], step=0.1, format='%.1f').style('width: 33%;')
    COMMENT_input = ui.textarea(label='Comments', value=current_values['COMMENT']).style('width: 100%;')
    ui.button('START', on_click=update_values).style('width: 100%;')
    
# Settings
with ui.card().style('width: 100%; max-width: 600px; margin: auto;'):
    with ui.expansion('SETTINGS', icon='settings').classes('w-full').style('font-weight: bold;'):
        
        # Movement settings
        with ui.expansion('MOVEMENT', icon='settings').classes('w-full').style('font-weight: bold;'):
            ui.label('X-MOVEMENT (CARRIAGE RANGE)').style('font-weight: bold;')
            with ui.row().style('width: 100%;'):
                XPOSMIN_input = ui.number(label='MIN (mm)', value=current_values['XPOSMIN'], min=0.0, max=100.0, step=0.1, format='%.1f').style('width: 25%;')
                XPOSMAX_input = ui.number(label='MAX (mm)', value=current_values['XPOSMAX'], min=0.0, max=100.0, step=0.1, format='%.1f').style('width: 25%;')
                XSTEP_input = ui.number(label='STEP (mm)', value=current_values['XSTEP'], min=0.1, max=5.0, step=0.1, format='%.1f').style('width: 25%;')
                SPEED_input = ui.number(label='SPEED (mm/s)', value=current_values['SPEED'], min=0.5, max=50.0, step=0.1, format='%.1f').style('width: 25%;')

            ui.label('Z-MOVEMENT (NEEDLE RANGE)').style('font-weight: bold;')
            with ui.row().style('width: 100%;'):
                ZPOSMIN_input = ui.number(label='MIN (mm)', value=current_values['ZPOSMIN'], min=0.0, max=30.0, step=0.1, format='%.1f').style('width: 25%;')
                ZPOSMAX_input = ui.number(label='MAX (mm)', value=current_values['ZPOSMAX'], min=0.0, max=30.0, step=0.1, format='%.1f').style('width: 25%;')
        
        # A-mode settings
        with ui.expansion('A-MODE', icon='settings').classes('w-full').style('font-weight: bold;'):
            with ui.row():
                A_MODE_AUTOGAIN_input = ui.checkbox('AUTO GAIN', value=current_values['A_MODE_AUTOGAIN']).style('width: 40%;')
                A_MODE_MANUALGAIN_input = ui.number(label='MANUAL GAIN', value=current_values['A_MODE_MANUALGAIN']).style('width: 40%;')
                A_MODE_OFFSETMIN_input = ui.number(label='OFFSET MIN', value=current_values['A_MODE_OFFSETMIN']).style('width: 40%;')
                A_MODE_OFFSETMAX_input = ui.number(label='OFFSET MAX', value=current_values['A_MODE_OFFSETMAX']).style('width: 40%;')
                A_MODE_PORT_input = ui.select(['X1', 'X2', 'X3', 'X4'], label='PORT', value=current_values['A_MODE_PORT']).style('width: 40%;')
                A_MODE_TXPAT_input = ui.select(['10 MHz 4 Pulses','5 MHz 3 Pulses','4 MHz 2 Pulses','4 MHz 3 Pulses','2.5 MHz 5 Pulses','2 MHz 4 Pulses','Barker 7','Test Pulse'], label='TX PATTERN', value=current_values['A_MODE_TXPAT']).style('width: 40%;')
                A_MODE_GAINRATE_input = ui.number(label='GAINRATE', value=current_values['A_MODE_GAINRATE']).style('width: 40%;')
                A_MODE_FILTERTYPE_input = ui.select(['BPF', 'HPF'], label='FILTERTYPE', value=current_values['A_MODE_FILTERTYPE']).style('width: 40%;')
                A_MODE_SCANLINES_input = ui.number('SCANLINES', value=current_values['A_MODE_SCANLINES'], min=1, max=1000).style('width: 40%;')
    
        # Doppler settings
        with ui.expansion('DOPPLER', icon='settings').classes('w-full').style('font-weight: bold;'):
            with ui.row():
                DOPPLER_AUTOGAIN_input = ui.checkbox('AUTO GAIN', value=current_values['DOPPLER_AUTOGAIN']).style('width: 40%;')
                DOPPLER_MANUALGAIN_input = ui.number(label='MANUAL GAIN', value=current_values['DOPPLER_MANUALGAIN']).style('width: 40%;')
                DOPPLER_OFFSETMIN_input = ui.number(label='OFFSET MIN', value=current_values['DOPPLER_OFFSETMIN']).style('width: 40%;')
                DOPPLER_OFFSETMAX_input = ui.number(label='OFFSET MAX', value=current_values['DOPPLER_OFFSETMAX']).style('width: 40%;')
                DOPPLER_PORT_input = ui.select(['X1', 'X2', 'X3', 'X4'], label='PORT', value=current_values['DOPPLER_PORT']).style('width: 40%;').style('width: 40%;')
                DOPPLER_TXPAT_input = ui.select(['10 MHz 4 Pulses','5 MHz 3 Pulses','4 MHz 8 Pulses','3.3 MHz 7 Pulses','2.5 MHz 5 Pulses','2 MHz 4 Pulses','Barker 13','Test Pulse'], label='TX PATTERN', value=current_values['DOPPLER_TXPAT']).style('width: 40%;')
                DOPPLER_ANGLE_input = ui.number(label='ANGLE (DEG)', value=current_values['DOPPLER_ANGLE']).style('width: 40%;')
                DOPPLER_FILTERTYPE_input = ui.select(['BPF', 'HPF'], label='FILTERTYPE', value=current_values['DOPPLER_FILTERTYPE']).style('width: 40%;')
                DOPPLER_SCANLINES_input = ui.number('SCANLINES', value=current_values['DOPPLER_SCANLINES'], min=1, max=1000).style('width: 40%;')
    
# Display available files
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
    
# Display latest scan image
components.create_display()
    
ui.run(show=False)
