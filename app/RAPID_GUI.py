from nicegui import app, ui
import os
import io
from lib_gui import process_handlers as ph
from lib_gui import components
import sys
import time

# Store one-time configuration flag
configured = False

# Close previous session if exists
ph.release_port(8080)

favicon_path = '/home/rapid/projects/rover/app/lib_gui/neurescue_logo_heart.png'
latest_image_path = ""

if not os.path.exists(ph.FILES_DIRECTORY):
    os.makedirs(ph.FILES_DIRECTORY)  
        
def start_scan():
    print("\n\n\n------------------------------------------------------------------------------\nScanning started")
    
    global configured
    
    # Update parameters
    for key in current_values.keys():
        component_name = f'{key}_input'
        try:
            component = eval(component_name)
            if component and hasattr(component, 'value'):
                current_values[key] = component.value
        except NameError:
            pass

    app.storage.general['current_values'] = current_values
    status_label.set_status("Scanning")

    # Run scanning
    ph.run_cpp_program(current_values, configured)
    configured = True

    # Process image
    if (MODE_input.value == 'A-MODE'): ph.process_b_mode_image()
    elif (MODE_input.value == 'M-MODE'): ph.process_m_mode_image()
    elif (MODE_input.value == 'M-MODE FULL SCAN'): ph.process_m_mode_scan()

    # Update GUI
    print("Scanning finished")
    status_label.set_status("Scanning finished")
    display.update_image()

def on_mode_change(value):
    a_mode_exp.visible = (value == 'A-MODE') 
    doppler_exp.visible = (value == 'DOPPLER') 
    m_mode_exp.visible = (value in ['M-MODE', 'M-MODE FULL SCAN']) 
    XPOSMIN_input.visible = (value in ['A-MODE', 'DOPPLER', 'M-MODE FULL SCAN']) 
    XSTEP_input.visible = (value == 'M-MODE FULL SCAN')

# Ensure stored values include all keys from defaults
defaults = ph.read_default_values()
stored_values = app.storage.general.get('current_values', {})
current_values = defaults.copy()
current_values.update(stored_values)
    
# GUI setup
ui.page_title("RAPID GUI")

# Header
components.create_header()

with ui.row().style('width: 100%; display: flex; flex-wrap: wrap;'):

    # Left-side container for the first two components
    with ui.column().style('flex: 1; padding: 10px;'):
        # Status label
        status_label = components.Status()

        # Scanning
        with ui.card().style('width: 100%; max-width: 600px; margin: auto;'):
            ui.label('ACTION').style('font-weight: bold;').style('width: 100%;')
            MODE_input = ui.select(
                ['A-MODE', 'DOPPLER', 'M-MODE', "M-MODE FULL SCAN", 'NEEDLE'], 
                label='MODE OF OPERATION', 
                value=current_values['MODE'],
                on_change = lambda e: on_mode_change(e.value)).style('width: 33%;')
            COMMENT_input = ui.textarea(label='Comments', value=current_values['COMMENT']).style('width: 100%;')
            ui.button('START', on_click=start_scan).style('width: 100%;')
            
        # Settings
        with ui.card().style('width: 100%; max-width: 600px; margin: auto;'):

            # Movement settings
            with ui.expansion('MOVEMENT', icon='settings').classes('w-full').style('font-weight: bold;'):
                ui.label('X-MOVEMENT (CARRIAGE RANGE)').style('font-weight: bold;')
                with ui.row().style('width: 100%;'):
                    XPOSMIN_input = ui.number(label='START (mm)', value=current_values['XPOSMIN'], min=2.0, max=60.0, step=0.1, format='%.1f').style('width: 25%;')
                    XPOSMAX_input = ui.number(label='TARGET (mm)', value=current_values['XPOSMAX'], min=2.0, max=60.0, step=0.1, format='%.1f').style('width: 25%;')
                    SPEED_input = ui.number(label='SPEED (mm/s)', value=current_values['SPEED'], min=0.5, max=50.0, step=0.1, format='%.1f').style('width: 25%;')
                    XSTEP_input = ui.number(label='XSTEP (mm)', value=current_values['XSTEP'], min=0.1, max=5.0, step=0.1, format='%.1f').style('width: 25%;')

            # A-mode settings
            a_mode_exp = ui.expansion('A-MODE', icon='settings').classes('w-full').style('font-weight: bold;')
            with a_mode_exp:
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
            
            
            # M-mode settings
            m_mode_exp = ui.expansion('M-MODE', icon='settings').classes('w-full').style('font-weight: bold;')
            m_mode_exp.visible = False
            with m_mode_exp:
                with ui.row():
                    M_MODE_SCANTIME_input = ui.number('SCAN TIME (ms)', value=current_values['M_MODE_SCANTIME']).style('width: 100%;')
        
            # Doppler settings
            doppler_exp = ui.expansion('DOPPLER', icon='settings').classes('w-full').style('font-weight: bold;')
            doppler_exp.visible = False
            with doppler_exp:
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
        

    # Right-side container for the next two components
    with ui.column().style('flex: 1; padding: 10px;'):
        
        # Display latest scan image
        display = components.Display()

        # Display available files
        components.available_files()
 
ui.run(show=False, favicon=favicon_path)