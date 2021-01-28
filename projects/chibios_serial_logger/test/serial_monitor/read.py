import serial
import sys
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib import style
import numpy as np

from struct import unpack

NO_ANALOG_CHANNELS = 4
ANALOG_SAMPLING_FREQUENCY = 200.0
DATA_DISPLAYED = 4000
channel_titles = {0 : "Battery Voltage", 1 : "APPS 1", 2 : "BSE", 3 : "APPS 2"}

if len(sys.argv) == 1:
    baudrate = 115200
else:
    baudrate = int(sys.argv[1])

data = [[0 for i in range(DATA_DISPLAYED)] for i in range(NO_ANALOG_CHANNELS)]
time_lbl = [1]
title = [""]

fig, axs = plt.subplots(nrows=2, ncols=2)

# This function is called periodically from FuncAnimation
def animate(i):
    for x in range(10):
        packet = str(ser.readline())
        if packet != "b\'\'":
            packet_name, packet_data = packet.split(':')
            packet_name = str(packet_name)
            packet_name = packet_name.strip('b\'')
            buffer = list(map(int, packet_data.split(',')[:-1]))
            
            if "can" in packet_name:
                time_lbl[0] = list(buffer)[0]
                fig.suptitle(f'{packet_name}: {time_lbl[0]} seconds', ha='left')
            elif "analog" in packet_name:
                title[0] = packet_name
                for i in range(NO_ANALOG_CHANNELS):
                    data[i].extend(buffer[i::NO_ANALOG_CHANNELS])
                    if len(data[i]) > DATA_DISPLAYED:
                        data[i] = data[i][-1*DATA_DISPLAYED::]
    # Draw x and y lists
    
    curr_time = float(time_lbl[0])
    xs = np.linspace(start=(curr_time - float(DATA_DISPLAYED)/ANALOG_SAMPLING_FREQUENCY), stop = curr_time, num=DATA_DISPLAYED)
    # print(xs)
    for i, ax in enumerate(axs.flatten()):
        ys = data[i][-1*DATA_DISPLAYED::]
        ax.clear()
        ax.plot(xs, ys)
        ax.title.set_text(f"{channel_titles[i]}")
        # Format plot
        # plt.xticks(rotation=45, ha='right')
        ax.axis([xs[0], xs[-1], -20, 5000]) #Use for arbitrary number of trials
    fig.subplots_adjust(top=.8, bottom=0.1, hspace=0.4, wspace=.5)

with serial.Serial("/dev/tty.usbmodem14103", baudrate, timeout=.05) as ser:
    # Create figure for plotting

    # Set up plot to call animate() function periodically
    ani = animation.FuncAnimation(fig, animate, interval=500, cache_frame_data=False)
    plt.show()
