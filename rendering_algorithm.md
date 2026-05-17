## REQUIREMENTS:
1. visible sectors list.
2. all walls list.
3. index for visible sectors.
4. index for all walls.
5. size for all walls.
6. size for visible sectors.
7. zbuffer

1. Check for the current sector. Load in all its walls
2. While there are still walls to draw:
    + Draw the wall. If the distance to the wall is smaller than the zbuffer (all set to 9999), draw it.
    + If there is more than 64 pixels visible of the wall, then it's marked as visible.
    + For each wall, if a wall is marked as a portal.
        -> Draw a "window" into the wall.
        -> Mark the window's zbuffer as -1.
    + If a portal is visible (i.e. more than 64 pixels of it is visible, then the sector it is connected to is loaded in).