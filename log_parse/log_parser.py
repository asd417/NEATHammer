import numpy as np
from PIL import Image, ImageTk
import matplotlib.pyplot as plt
from IPython.display import display
import tkinter as tk
from tkinter import ttk

import subprocess
import glob
import os



def parse_grid(file_path):
    """
    Parse a grid of float values from a file and return as a 2D numpy array.
    """
    grid = []
    with open(file_path, 'r') as file:
        subgrid = []
        for line in file:
            # Split the line into values and convert to float
            row = list(map(float, line.split()))
            #print(len(row))
            if(len(row) == 0 and len(subgrid) != 0):
                grid.append(subgrid)
                subgrid = []
            else:
                if(len(row) != 0): 
                    subgrid.append(row)

    # Ensure all rows have the same length
    if len(set(len(row) for row in grid[0])) != 1:
        #print(set(len(row) for row in grid[0]))
        raise ValueError("Inconsistent row lengths found in the grid data.")

    return np.array(grid)

def normalize_grid(grid):
    """
    Normalize the grid values to the range 0-255.
    """
    min_val = np.min(grid)
    max_val = np.max(grid)
    norm_grid = 255 * (grid - min_val) / (max_val - min_val)
    return norm_grid.astype(np.uint8)


def create_image_from_array(array, size=(512, 512)):
    """
    Create a PIL image from a 2D numpy array.
    """
    image = Image.fromarray(array)
    return image.resize(size, resample=Image.Resampling.NEAREST)


class ImageSliderApp:
    def __init__(self, master, images):
        self.master = master
        self.images = images
        self.current_index = 0
        
        self.image_label = tk.Label(master)
        self.image_label.pack()

        self.slider_frame = ttk.Frame(master)
        self.slider_frame.pack(pady=20)

        self.slider = ttk.Scale(master, from_=0, to=len(images) - 1, orient="horizontal", command=self.update_image)
        self.slider.pack(fill=tk.X, padx=20)
        self.slider.pack()

        self.index_label = ttk.Label(self.slider_frame, text=f"Image Index: {self.current_index}")
        self.index_label.pack()
        
        self.button_frame = ttk.Frame(master)
        self.button_frame.pack(pady=10)

        self.prev_button = ttk.Button(self.button_frame, text="Previous", command=self.decrement_index)
        self.prev_button.pack(side=tk.LEFT, padx=5)

        self.next_button = ttk.Button(self.button_frame, text="Next", command=self.increment_index)
        self.next_button.pack(side=tk.LEFT, padx=5)

        self.update_image(0)

    def update_image(self, index):
        index = int(float(index))
        self.current_index = index
        img = self.images[self.current_index]
        imgtk = ImageTk.PhotoImage(image=img, size=32)
        self.image_label.imgtk = imgtk
        self.image_label.configure(image=imgtk)
        self.index_label.config(text=f"Image Index: {self.current_index}")

    def increment_index(self):
        if self.current_index < len(self.images) - 1:
            self.current_index += 1
            self.update_image(self.current_index)

    def decrement_index(self):
        if self.current_index > 0:
            self.current_index -= 1
            self.update_image(self.current_index)

GENERATEFRAMES = True

def main():
    path = os.path.abspath("").replace("\\", "/")
    framepath = '/near_winning_example/Terran_win/outputs/'
    frame_save_path = path + framepath
    if(GENERATEFRAMES):
        input_file = 'near_winning_example/Terran_win/Log_NEAT_Output.txt-1'  # Path to the input file containing the grid
        #input_file = 'near_winning_example/Terran_win/Log_NEAT_Input.txt-1'  # Path to the input file containing the grid

        # Parse the grid from the file
        grid = parse_grid(input_file)

        normalized_images = np.array([normalize_grid(image) for image in grid])

        # Convert to PIL images
        pil_images = [create_image_from_array(image) for image in normalized_images]
        c = 0
        for i in pil_images:
            i.save(frame_save_path + "{:06d}".format(c) + '.png')
            c += 1
        # Create the main window
        root = tk.Tk()
        root.title("Image Slider")

        print(f"Grayscale image saved as {frame_save_path}")


    
    filenames = glob.glob(path + framepath + '*.png')
    print(f"Creating Video file from frames created at path: {filenames[0]}")
    fps = 17.89/6
    duration = 1/fps

    with open(path + framepath + "ffmpeg_input.txt", "wb") as outfile:
        for filename in filenames:
            fixed_path = filename.replace("\\", "/")
            outfile.write(f"file '{fixed_path}'\n".encode())
            outfile.write(f"duration {duration}\n".encode())

    command_line = f"ffmpeg -f concat -safe 0 -i {frame_save_path}ffmpeg_input.txt -c:v libx265 -pix_fmt yuv420p {path}\\out.mp4"
    print(command_line)

    pipe = subprocess.Popen(command_line, shell=True, stdout=subprocess.PIPE).stdout
    output = pipe.read().decode()
    pipe.close()
    if(GENERATEFRAMES):
        # Create and run the application
        app = ImageSliderApp(root, pil_images)
        root.mainloop()

if __name__ == "__main__":
    main()
