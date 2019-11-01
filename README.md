# Serial plotter

based on terminal Qt example

### Features

- "Real time" visualization data from serial port with QCustomPlot 
- For plotting 6 channels used ASC II data format
- Hiding and showing channels in plotting time

## Input data format

Now hardcoded like "A012B312C213D241E001F\n", this is bad way, I know :) will be dynamicaly in the feature.

Just send any 3 asc II numbers for each not used channels and hide them in the app, if you want to use less than 6 channels.

## OS
Tested for Windows 10 only

### TODO

- implement architecture for app, put in order this shit

Add options for: 
- choosing number of channels
- choosing color for each channel, and other customize parameters for plot
- choosing path for export data
- plotting graph by early exported data
- changing format for mapping data from serial port 
