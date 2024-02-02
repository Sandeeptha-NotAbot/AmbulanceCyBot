# Author: Phu Nguyen
# Date: 10/30/2023
# Description: Client starter code that combines: 1) Simple GUI, 2) creation of a thread for
#              running the Client socket in parallel with the GUI, and 3) Simple recieven of mock sensor 
#              data for a server/cybot.for collecting data from the cybot.

# General Python tutorials (W3schools):  https://www.w3schools.com/python/

# Serial library:  https://pyserial.readthedocs.io/en/latest/shortintro.html 
from ast import Attribute
from turtle import circle
from typing import Self
import serial
import time # Time library   
# Socket library:  https://realpython.com/python-sockets/  
# See: Background, Socket API Overview, and TCP Sockets  
import socket
from tkinter import *
import tkinter as tk # Tkinter GUI library
from PIL import Image,ImageTk
# Thread library: https://www.geeksforgeeks.org/how-to-use-thread-in-tkinter-python/
import threading
from multiprocessing import Process
import os  # import function for finding absolute path to this python script
from math import *
import re

##### START Define Functions  #########
#Global Variables
xold = 0
yold = 0
lineArray = []
radarLine = 0
left_turn_thread_count = 0
right_turn_thread_count = 0
move_thread_count = 0
bump = None
edge = None
compass = None
compassDeg = 0
startTime = 0
stopTime = 0
timeArray = []

# Main: Mostly used for setting up, and starting the GUI
def main():
        #Canvas Dimensions
        global CANVAS_WIDTH
        CANVAS_WIDTH = 600
        global CANVAS_HEIGHT
        CANVAS_HEIGHT = 300
        global OFFSET
        OFFSET = 21

        global window  # Made global so quit function (send_quit) can access
        window = tk.Tk() # Create a Tk GUI Window

        window.title('Final Project')
        window.geometry('640x640')
        window.resizable(width=False, height=False)

        #Scan screen
        global canvas
        canvas = tk.Canvas(window, width=CANVAS_WIDTH, height=CANVAS_HEIGHT, bg="white")
        canvas.pack(pady=20)

        background = Image.open("background5.png")

        background = background.resize((600,300))
        global backgroundImage
        backgroundImage = ImageTk.PhotoImage(background)

        canvas.create_image(CANVAS_WIDTH / 2,CANVAS_HEIGHT / 2,anchor=tk.CENTER,image=backgroundImage)

        #Textbox
        textFrame = tk.Frame(window, height=280, width=220, bg="white")
        textFrame.place(x = 20, y = 340)
        scrollbar = tk.Scrollbar(textFrame, orient='vertical')
        scrollbar.pack(side=tk.RIGHT, fill='y')
        global textbox
        textbox = tk.Text(textFrame, height=17, width=50, yscrollcommand=scrollbar.set)
        scrollbar.config(command=textbox.yview)
        textbox.pack(side=tk.LEFT)

        buttonFrame = tk.Frame(window, height=280, width=220)
        buttonFrame.pack(side=tk.RIGHT, padx = 20)

        # Last command label  
        global Last_command_Label  # Made global so that Client function (socket_thread) can modify
        Last_command_Label = tk.Label(buttonFrame, text="Last Command Sent:") # Creat a Label
        Last_command_Label.pack() # Pack the label into the window for display

        # Quit command Button
        quit_command_Button = tk.Button(buttonFrame, text ="Press to Quit", command = send_quit)
        quit_command_Button.pack()  # Pack the button into the window for display

        # Cybot Scan command Button
        scan_command_Button = tk.Button(buttonFrame, text ="Press to Scan", command = send_scan)
        scan_command_Button.pack() # Pack the button into the window for display

        scan_command_Button = tk.Button(buttonFrame, text ="Switch Mode", command = send_switch)
        scan_command_Button.pack() # Pack the button into the window for display

        clear_command_Button = tk.Button(buttonFrame,text ="Press to clear data", command = send_clear)
        clear_command_Button.pack()  # Pack the button into the window for display

        clear_command_Button = tk.Button(buttonFrame,text ="Press to reset compass", command = send_compass)
        clear_command_Button.pack()  # Pack the button into the window for display

        window.bind('<Key>',get_pos)

        # Create a Thread that will run a fuction assocated with a user defined "target" function.
        # In this case, the target function is the Client socket code

        my_thread = threading.Thread(target=socket_thread) # Creat the thread
        my_thread.start() # Start the thread

        #Fills line array with 180 place holder lines
        global lineArray
        for i in range(181):
                lineArray.append(canvas.create_line(0, 0, 0, 0, fill="red", width=0))

        # Start event loop so the GUI can detect events such as button clicks, key presses, etc.
        window.mainloop()

#When a bump is hit run this function
def update_bump(char):
        global bump
        global gui_send_message

        #Send a stop to the CyBot
        gui_send_message = " "

        radius = 7

        #Draw which bump was hit on scan based on character recieved
        if char == "b":
                canvas.delete(bump)
                return
        elif char == "l":
                x = CANVAS_WIDTH / 2 - 35
                y = CANVAS_HEIGHT - 35
        elif char == "r":
                x = CANVAS_WIDTH / 2 + 35
                y = CANVAS_HEIGHT - 35
        elif char == "f":
                x = CANVAS_WIDTH / 2
                y = CANVAS_HEIGHT - 40

        canvas.delete(bump)
        bump = canvas.create_oval(x - radius, y - radius, x + radius, y + radius, fill="orange")   

#When an edge is detected run this function
def update_edge(string):
        global edge
        global gui_send_message

        #Send a stop to the Cybot
        gui_send_message = " "

        #Convert ascii symbol into number
        currentEdge = ord(string[1])

        #Detect which edge was hit and change the line displayed on the cybot or if "k" was sent then remove line
        if string[0] == "k":
                canvas.delete(edge)
        elif currentEdge == 8:
                canvas.delete(edge)
                edge = canvas.create_line(250, CANVAS_HEIGHT, CANVAS_WIDTH - 300, CANVAS_HEIGHT - 70, fill="blue", width=3)
        elif currentEdge == 12:
                canvas.delete(edge)
                edge = canvas.create_line(250, CANVAS_HEIGHT - 10, CANVAS_WIDTH - 275, CANVAS_HEIGHT - 60, fill="blue", width=3)
        elif currentEdge == 4: 
                canvas.delete(edge)
                edge = canvas.create_line(250, CANVAS_HEIGHT - 20, CANVAS_WIDTH - 250, CANVAS_HEIGHT - 50, fill="blue", width=3)  
        elif currentEdge == 6:
                canvas.delete(edge)
                edge = canvas.create_line(250, CANVAS_HEIGHT - 35, CANVAS_WIDTH - 250, CANVAS_HEIGHT - 35, fill="blue", width=3) 
        elif currentEdge == 2:
                canvas.delete(edge)
                edge = canvas.create_line(250, CANVAS_HEIGHT - 50, CANVAS_WIDTH - 250, CANVAS_HEIGHT - 20, fill="blue", width=3)   
        elif currentEdge == 3:
                canvas.delete(edge)
                edge = canvas.create_line(250, CANVAS_HEIGHT - 60, CANVAS_WIDTH - 250, CANVAS_HEIGHT - 10, fill="blue", width=3)
        elif currentEdge == 1:
                canvas.delete(edge)
                edge = canvas.create_line(300, CANVAS_HEIGHT - 70, CANVAS_WIDTH - 250, CANVAS_HEIGHT, fill="blue", width=3) 

#Updates textbox in GUI with provided string
def update_textbox(data):
        textbox.config(state=tk.NORMAL) # unlock the textbox

        textbox.insert(tk.END, "\n"+str(data)) # add newline and append the data

        textbox.config(state=tk.DISABLED) # lock back the textbox to readonly
        textbox.see(tk.END) # scroll to the bottom to see the last line

#Updates the scan with provded degrees and data
def update_scan(degrees, radius):
        global lineArray
        radius = radius * 5
        global xold
        global yold
        radians = degrees * (pi / 180)

        #Stores the location of the old x and y then calculates the value of the new x and y and connects those 2 with a line to make a radar scan

        if degrees == 0:
                xold = CANVAS_WIDTH / 2 + radius*cos(radians)
                yold = CANVAS_HEIGHT - radius*sin(radians) - OFFSET
        
        xnew = CANVAS_WIDTH / 2 + radius*cos(radians)
        ynew = CANVAS_HEIGHT - radius*sin(radians) - OFFSET
        canvas.delete(lineArray[degrees])
        lineArray[degrees] = canvas.create_line(xold, yold, xnew, ynew, fill="red", width=3)
        global radarLine
        canvas.delete(radarLine)
        radarLine = canvas.create_line(CANVAS_WIDTH / 2, CANVAS_HEIGHT - 20, CANVAS_WIDTH / 2 + 250*cos(radians), CANVAS_HEIGHT - 250*sin(radians) - OFFSET, fill="lime", width=3)
        xold = xnew
        yold = ynew

#Updates compass based on the amount of degrees rotated
def update_compass(degree):
        global compass
        global compassDeg
        compassDeg += degree

        radians = compassDeg * (pi / 180)

        canvas.delete(compass)
        compass = canvas.create_line(50, 50, 50 + 50*sin(radians), 50 - 50*cos(radians), fill="red", width=2)

#Gets the current key pressed and sends it ot the cybot
def get_pos(event):
        global gui_send_message

        #print(f'{event.char} key is press')
        if event.char == 'w' or event.char == 'a' or event.char == 'd' or event.char == 's':
                movement_thread = threading.Thread(target=move_thread, args="%c" % (event.char))
                movement_thread.start()
                return
        
        if event.char == '/':
                totalNum = 0
                for num in timeArray:
                        totalNum += num

                print("Total degrees turned: " + str(totalNum))

                timeArray.clear()
                return
                
        gui_send_message = "%c" % (event.char)

# Quit Button action.  Tells the client to send a Quit request to the Cybot, and exit the GUI
def send_quit():
        global gui_send_message # Command that the GUI has requested be sent to the Cybot
        global window  # Main GUI window
        
        gui_send_message = "quit\n"   # Update the message for the Client to send
        time.sleep(1)
        window.destroy() # Exit GUI

#Resets the compass
def send_compass():
        global compassDeg
        compassDeg = 0
        update_compass(0)

# Scan Button action.  Tells the client to send a scan request to the Cybot
def send_scan():
        global gui_send_message # Command that the GUI has requested sent to the Cybot

        send_clear()
        
        # for i in range(0, 182, 2):
        #         if i > 90 and i < 135:
        #                 update_scan(i, 100)  
        #         else:
        #                 update_scan(i, 50)
        #         update_textbox(i)
        #         textbox.after(100)
        #         textbox.update()

        gui_send_message = "m"   # Update the message for the Client to send

def send_switch():
        global gui_send_message # Command that the GUI has requested sent to the Cybot
        
        gui_send_message = "n"   # Update the message for the Client to send

#Clears the scan canvas
def send_clear():
        textbox.config(state=tk.NORMAL) # unlock the textbox
        textbox.delete('1.0', tk.END)
        canvas.delete('all')
        canvas.create_image(CANVAS_WIDTH / 2,CANVAS_HEIGHT / 2,anchor=tk.CENTER,image=backgroundImage)
        textbox.config(state=tk.DISABLED) # lock back the textbox to readonly

#Parses a string for all integers
def parse_string(string):
        return re.findall(r'\d+', string)

#Thread that sends a movement to the Cybot then waits then sends a stop to the Cybot (allows the cybot to move and scan at the same time)
def move_thread(string):
        global gui_send_message
        global move_thread_count
        global right_turn_thread_count
        global left_turn_thread_count
        global startTime
        global stopTime

        if string == "w":
                move_thread_count += 1
                gui_send_message = "w"
                time.sleep(0.2)

                if move_thread_count == 1:
                        gui_send_message = " "

                move_thread_count -= 1
        elif string == "a":
                left_turn_thread_count += 1
                gui_send_message = "a"

                
                if left_turn_thread_count == 1:
                        startTime = time.time()

                time.sleep(0.2)

                if left_turn_thread_count == 1:
                        gui_send_message = " "
                        #Record time it took to stop rotating and get the amount of degrees turned based off of that
                        stopTime = time.time()
                        timeElapsed = stopTime - startTime

                        factor = min(22 + (3.05 * (timeElapsed - 0.2)), 27.5) * -1
                        update_compass((stopTime - startTime) * factor)
                        print("Time Turning: " + str(stopTime - startTime) + ", Degrees Turned: " + str((stopTime - startTime) * factor))
                        timeArray.append((stopTime - startTime) * factor)
                        

                left_turn_thread_count -= 1
        elif string == "d":
                right_turn_thread_count += 1
                gui_send_message = "d"

                if right_turn_thread_count == 1:
                        startTime = time.time()
                
                time.sleep(0.2)

                if right_turn_thread_count == 1:
                        gui_send_message = " "

                        #Record time it took to stop rotating and get the amount of degrees turned based off of that
                        stopTime = time.time()
                        timeElapsed = stopTime - startTime

                        factor = min(22 + (3.05 * (timeElapsed - 0.2)), 27.5)
                        update_compass((stopTime - startTime) * factor)
                        print("Time Turning: " + str(stopTime - startTime) + ", Degrees Turned: " + str((stopTime - startTime) * factor))
                        timeArray.append((stopTime - startTime) * factor)

                right_turn_thread_count -= 1
        elif string == "s":
                gui_send_message = "s"
                time.sleep(0.4)
                gui_send_message = " "

# Client socket code (Run by a thread created in main)
def socket_thread():
        # Define Globals
        global Last_command_Label # GUI label for displaying the last command sent to the Cybot
        global gui_send_message   # Command that the GUI has requested be sent to the Cybot

        # A little python magic to make it more convient for you to adjust where you want the data file to live
        # Link for more info: https://towardsthecloud.com/get-relative-path-python 
        absolute_path = os.path.dirname(__file__) # Absoult path to this python script
        relative_path = "./"   # Path to sensor data file relative to this python script (./ means data file is in the same directory as this python script)
        full_path = os.path.join(absolute_path, relative_path) # Full path to sensor data file
        filename = 'sensor-scan.txt' # Name of file you want to store sensor data from your sensor scan command

        # Choose to create either a UART or TCP port socket to communicate with Cybot (Not both!!)
        # UART BEGIN
        #cybot = serial.Serial('COM100', 115200)  # UART (Make sure you are using the correct COM port and Baud rate!!)
        # UART END

        # TCP Socket BEGIN (See Echo Client example): https://realpython.com/python-sockets/#echo-client-and-server
        HOST = "192.168.1.1"  # The server's hostname or IP address
        PORT = 288        # The port used by the server
        cybot_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  # Create a socket object
        cybot_socket.connect((HOST, PORT))   # Connect to the socket  (Note: Server must first be running)
                      
        cybot = cybot_socket.makefile("rbw", buffering=0)  # makefile creates a file object out of a socket:  https://pythontic.com/modules/socket/makefile
        # TCP Socket END

        # Send some text: Either 1) Choose "Hello" or 2) have the user enter text to send
        send_message = "Hello\n"                            # 1) Hard code message to "Hello", or
        # send_message = input("Enter a message:") + '\n'   # 2) Have user enter text
        gui_send_message = "wait\n"  # Initialize GUI command message to wait                                

        cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server

        print("Sent to server: " + send_message) 

        # Send messges to server until user sends "quit"
        while send_message != 'quit\n':
                
                # Update the GUI to display command being sent to the CyBot
                command_display = "Last Command Sent: " + send_message
                Last_command_Label.config(text = command_display)  
        
                # Check if a sensor scan command has been sent
                if (send_message == "M\n") or (send_message == "m\n"):
                        count = 0
                        print("Requested Sensor scan from Cybot:\n")
                        rx_message = bytearray(1) # Initialize a byte array

                        # Create or overwrite existing sensor scan data file
                        file_object = open(full_path + filename,'w') # Open the file: file_object is just a variable for the file "handler" returned by open()

                        while (rx_message.decode() != "END\n"): # Collect sensor data until "END" recieved
                                rx_message = cybot.readline()   # Wait for sensor response, readline expects message to end with "\n"
                                file_object.write(rx_message.decode())  # Write a line ofaaaaaaaaaa sensor data to the file
                                print(rx_message.decode()) # Convert message from bytes to String (i.e., decode), then print
                                update_textbox(rx_message.decode())

                                if any(chr.isdigit() for chr in rx_message.decode()) and count <= 180:
                                        int_string = parse_string(rx_message.decode())
                                        count += 1
                                        update_scan(int(int_string[0]), int(int_string[2]))
                                

                        file_object.close() # Important to close file once you are done with it!!


                # Choose either: 1) Idle wait, or 2) Request a periodic status update from the Cybot
                # 1) Idle wait: for gui_send_message to be updated by the GUI

                #Waiting to recieve from the cybot
                while gui_send_message == "wait\n":
                        rx_message = bytearray(1)
                        rx_message = cybot.readline()

                        #Get ping distance
                        if rx_message.decode()[0] == "o":
                                update_textbox("Ping Distance: " + str(rx_message.decode()[1]))
                                continue
                        
                        #if any number is recieved update the scan and textbox
                        if any(chr.isdigit() for chr in rx_message.decode()):
                                int_string = parse_string(rx_message.decode())
                                update_textbox(rx_message.decode())
                                update_scan(int(int_string[0]), int(int_string[1]))
                                #print(int_string)
                        
                        #Recieves bump from the cybot
                        if rx_message.decode() == "l\n" or rx_message.decode() == "r\n" or rx_message.decode() == "f\n" or rx_message.decode() == "b\n":
                                print("Got this bump: " + rx_message.decode())
                                update_bump(rx_message.decode()[0])
                        
                        #Recieves edge detection from the cybot
                        if rx_message.decode()[0] == "e" or rx_message.decode()[0] == "k":
                                print("Got this edge: " + rx_message.decode()[0] + "\n")
                                update_edge(rx_message.decode())

                send_message = gui_send_message
                # 2) Request a periodic Status update from the Cybot:
                # every .1 seconds if GUI has not requested to send a new command
                #time.sleep(.1)
                #if(gui_send_message == "wait\n"):   # GUI has not requested a new command
                #        send_message = "status\n"   # Request a status update from the Cybot
                #else:
                #        send_message = gui_send_message  # GUI has requested a new command

                gui_send_message = "wait\n"  # Reset gui command message request to wait                        
                #print(send_message.encode())
                cybot.write(send_message.encode()) # Convert String to bytes (i.e., encode), and send data to the server
                
        print("Client exiting, and closing file descriptor, and/or network socket\n")
        time.sleep(2) # Sleep for 2 seconds
        cybot.close() # Close file object associated with the socket or UARTwwww
        cybot_socket.close()  # Close the socket (NOTE: comment out if using UART interface, only use for network socket option)

##### END Define Functions  #########


### Run main ###
main()
