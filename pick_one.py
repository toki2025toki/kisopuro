#!/usr/bin/env python3
import matplotlib.pyplot as plt
import matplotlib.image as mpimg

img = mpimg.imread("map.png")
fig, ax = plt.subplots()
ax.imshow(img)
ax.set_title("Click to print (x,y). Close to exit.")

def on_click(e):
    if e.inaxes != ax:
        return
    x = int(round(e.xdata))
    y = int(round(e.ydata))
    print(f"({x}, {y})")

fig.canvas.mpl_connect("button_press_event", on_click)
plt.show()
