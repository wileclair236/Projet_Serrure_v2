import tkinter as tk

SCREEN_WIDTH = 128
SCREEN_HEIGHT = 64
PIXEL_SIZE = 7

# ----------------- BUFFER -----------------
canvas_buffer = [[0 for _ in range(SCREEN_HEIGHT)] for _ in range(SCREEN_WIDTH)]

# visited utilisé uniquement pour generate
visited = [[False for _ in range(SCREEN_HEIGHT)] for _ in range(SCREEN_WIDTH)]

# ----------------- HISTORY -----------------
history = []

line_start = None
mode = "line"

# ----------------- WINDOW -----------------
root = tk.Tk()
root.title("Display Vector Generator")

canvas_widget = tk.Canvas(
    root,
    width=SCREEN_WIDTH * PIXEL_SIZE + 50,
    height=SCREEN_HEIGHT * PIXEL_SIZE + 20,
    bg="black"
)
canvas_widget.pack()

hud_label = tk.Label(root, text="Mode: line", fg="white", bg="black")
hud_label.pack()

# ----------------- COORDS -----------------
def screen_y(y):
    return (SCREEN_HEIGHT - 1 - y) * PIXEL_SIZE + 20

def get_coords(event):
    x = (event.x - 30) // PIXEL_SIZE
    y = (event.y - 20) // PIXEL_SIZE
    y = SCREEN_HEIGHT - 1 - y

    if 0 <= x < SCREEN_WIDTH and 0 <= y < SCREEN_HEIGHT:
        return x, y
    return None, None

# ----------------- DRAW PIXEL -----------------
def draw_pixel(x, y, record=True):
    if 0 <= x < SCREEN_WIDTH and 0 <= y < SCREEN_HEIGHT:

        old = canvas_buffer[x][y]
        canvas_buffer[x][y] = 1

        canvas_widget.create_rectangle(
            x * PIXEL_SIZE + 30,
            screen_y(y),
            (x + 1) * PIXEL_SIZE + 30,
            screen_y(y) + PIXEL_SIZE,
            fill="white",
            outline="white",
            tags="pixel"
        )

        if record:
            history.append([(x, y, old)])

# ----------------- LINE -----------------
def draw_line(x0, y0, x1, y1):
    pixels = []

    dx = abs(x1 - x0)
    dy = abs(y1 - y0)
    sx = 1 if x0 < x1 else -1
    sy = 1 if y0 < y1 else -1
    err = dx - dy

    while True:
        pixels.append((x0, y0, canvas_buffer[x0][y0]))
        canvas_buffer[x0][y0] = 1

        canvas_widget.create_rectangle(
            x0 * PIXEL_SIZE + 30,
            screen_y(y0),
            (x0 + 1) * PIXEL_SIZE + 30,
            screen_y(y0) + PIXEL_SIZE,
            fill="white",
            outline="white",
            tags="pixel"
        )

        if x0 == x1 and y0 == y1:
            break

        e2 = 2 * err
        if e2 > -dy:
            err -= dy
            x0 += sx
        if e2 < dx:
            err += dx
            y0 += sy

    history.append(pixels)

# ----------------- RECT -----------------
def draw_rect(x1, y1, x2, y2, fill=False):
    pixels = []

    min_x, max_x = sorted([x1, x2])
    min_y, max_y = sorted([y1, y2])

    for x in range(min_x, max_x + 1):
        for y in range(min_y, max_y + 1):

            if fill or x in (min_x, max_x) or y in (min_y, max_y):

                old = canvas_buffer[x][y]
                canvas_buffer[x][y] = 1

                canvas_widget.create_rectangle(
                    x * PIXEL_SIZE + 30,
                    screen_y(y),
                    (x + 1) * PIXEL_SIZE + 30,
                    screen_y(y) + PIXEL_SIZE,
                    fill="white",
                    outline="white",
                    tags="pixel"
                )

                pixels.append((x, y, old))

    history.append(pixels)

# ----------------- INPUT -----------------
def on_click(event):
    global line_start, mode

    x, y = get_coords(event)
    if x is None:
        return

    if mode == "line":
        if line_start is None:
            line_start = (x, y)
        else:
            draw_line(*line_start, x, y)
            line_start = None

    elif mode == "rect":
        if line_start is None:
            line_start = (x, y)
        else:
            draw_rect(line_start[0], line_start[1], x, y, fill=False)
            line_start = None

    elif mode == "rectfill":
        if line_start is None:
            line_start = (x, y)
        else:
            draw_rect(line_start[0], line_start[1], x, y, fill=True)
            line_start = None

    else:
        draw_pixel(x, y)

canvas_widget.bind("<Button-1>", on_click)

# ----------------- HUD -----------------
def update_hud(event):
    x, y = get_coords(event)
    hud_label.config(text=f"X: {x} Y: {y}" if x is not None else "Out")

canvas_widget.bind("<Motion>", update_hud)

# ----------------- CLEAR -----------------
def clear_canvas():
    global canvas_buffer, visited, history, line_start

    canvas_buffer = [[0 for _ in range(SCREEN_HEIGHT)] for _ in range(SCREEN_WIDTH)]
    visited = [[False for _ in range(SCREEN_HEIGHT)] for _ in range(SCREEN_WIDTH)]
    history = []
    line_start = None

    canvas_widget.delete("pixel")
    draw_reference_line()
    draw_grid()

# ----------------- UNDO -----------------
def undo(event=None):
    if not history:
        return

    last = history.pop()

    for x, y, old in last:
        canvas_buffer[x][y] = old

    redraw()

def redraw():
    canvas_widget.delete("pixel")

    for x in range(SCREEN_WIDTH):
        for y in range(SCREEN_HEIGHT):
            if canvas_buffer[x][y]:
                canvas_widget.create_rectangle(
                    x * PIXEL_SIZE + 30,
                    screen_y(y),
                    (x + 1) * PIXEL_SIZE + 30,
                    screen_y(y) + PIXEL_SIZE,
                    fill="white",
                    outline="white",
                    tags="pixel"
                )

root.bind("<Control-z>", undo)

# ----------------- GRID -----------------
def draw_grid():
    for x in range(0, SCREEN_WIDTH, 8):
        canvas_widget.create_text(
            x * PIXEL_SIZE + 30 + PIXEL_SIZE / 2,
            10,
            text=str(x),
            fill="white",
            font=("Arial", 8),
            tags="ui"
        )

    for y in range(0, SCREEN_HEIGHT, 8):
        canvas_widget.create_text(
            15,
            screen_y(y) + PIXEL_SIZE / 2,
            text=str(y),
            fill="white",
            font=("Arial", 8),
            tags="ui"
        )

# ----------------- REFERENCE LINE -----------------
def draw_reference_line():
    y = 45
    canvas_widget.create_line(
        30,
        screen_y(y) + PIXEL_SIZE / 2,
        SCREEN_WIDTH * PIXEL_SIZE + 30,
        screen_y(y) + PIXEL_SIZE / 2,
        fill="purple",
        width=2,
        tags="ui"
    )

# ----------------- MODE -----------------
def set_mode(m):
    global mode, line_start
    mode = m
    line_start = None
    hud_label.config(text=f"Mode: {mode}")

# ----------------- GENERATE -----------------
def generate_code():
    global visited
    visited = [[False for _ in range(SCREEN_HEIGHT)] for _ in range(SCREEN_WIDTH)]

    code = ["if (!display.isReady())","{","    return;","}","display.clear();"]

    # RECTFILL
    for y in range(SCREEN_HEIGHT):
        for x in range(SCREEN_WIDTH):

            if canvas_buffer[x][y] and not visited[x][y]:

                w = 0
                while x + w < SCREEN_WIDTH and canvas_buffer[x + w][y] and not visited[x + w][y]:
                    w += 1

                h = 0
                valid = True
                while y + h < SCREEN_HEIGHT and valid:
                    for xi in range(x, x + w):
                        if not canvas_buffer[xi][y + h] or visited[xi][y + h]:
                            valid = False
                            break
                    if valid:
                        h += 1

                if w > 1 and h > 1:
                    code.append(f"display.rectFill({x}, {y}, {w}, {h}, true);")
                    for xi in range(x, x + w):
                        for yi in range(y, y + h):
                            visited[xi][yi] = True

    # RECT
    visited = [[False for _ in range(SCREEN_HEIGHT)] for _ in range(SCREEN_WIDTH)]

    for y in range(SCREEN_HEIGHT):
        for x in range(SCREEN_WIDTH):

            if canvas_buffer[x][y] and not visited[x][y]:

                w = 0
                while x + w < SCREEN_WIDTH and canvas_buffer[x + w][y]:
                    w += 1

                h = 0
                while y + h < SCREEN_HEIGHT:
                    ok = True
                    for xi in range(x, x + w):
                        if not canvas_buffer[xi][y + h]:
                            ok = False
                            break
                    if not ok:
                        break
                    h += 1

                if w > 1 and h > 1:

                    top = all(canvas_buffer[xi][y] for xi in range(x, x+w))
                    bottom = all(canvas_buffer[xi][y+h-1] for xi in range(x, x+w))
                    left = all(canvas_buffer[x][yi] for yi in range(y, y+h))
                    right = all(canvas_buffer[x+w-1][yi] for yi in range(y, y+h))

                    if top and bottom and left and right:
                        code.append(f"display.rect({x}, {y}, {w}, {h}, true);")

                        for xi in range(x, x + w):
                            for yi in range(y, y + h):
                                visited[xi][yi] = True

    # PIXELS
    for x in range(SCREEN_WIDTH):
        for y in range(SCREEN_HEIGHT):
            if canvas_buffer[x][y] and not visited[x][y]:
                code.append(f"display.pixel({x}, {y}, true);")
                visited[x][y] = True

    code.append("display.flush();")

    win = tk.Toplevel(root)
    win.title("Generated Code")

    txt = tk.Text(win, width=100, height=35)
    txt.pack()
    txt.insert(tk.END, "\n".join(code))

# ----------------- BUTTONS -----------------
frame = tk.Frame(root)
frame.pack()

tk.Button(frame, text="Line", command=lambda: set_mode("line")).pack(side=tk.LEFT)
tk.Button(frame, text="Rect", command=lambda: set_mode("rect")).pack(side=tk.LEFT)
tk.Button(frame, text="RectFill", command=lambda: set_mode("rectfill")).pack(side=tk.LEFT)
tk.Button(frame, text="Pixel", command=lambda: set_mode("pixel")).pack(side=tk.LEFT)

tk.Button(frame, text="Generate", command=generate_code).pack(side=tk.LEFT)
tk.Button(frame, text="Clear", command=clear_canvas).pack(side=tk.LEFT)

# ----------------- INIT -----------------
draw_grid()
draw_reference_line()

root.mainloop()