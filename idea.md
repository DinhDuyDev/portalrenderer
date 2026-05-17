##### Ideas for implementation
1. z-buffer: the distance from the wall is stored in a 640x360 array, then, z-depth is checked instead of just sorting the whole list.
2. use up-trees and union-finds to store wall data.
    + Union find is consisted of wall vertices.
    + If a wall vertex is found to be in the same set as another when adding.
        + Creates a sector.

3. OR: create a list of vertices that has commands connecting them, like a graph.
    + Directed graph?

    (a)           (b)
    *------------*
   /
  /
 * (c)
 |
 |
 * (d)
  \
   *-----------*
   (e)          (f)

    --> This example, to make this shape: 
        1. a-b is made, and union(a, b) called.
        2. a-c is made, and union(a, c) called.
        3. c-d is made, and union(c, d),
        4. d-e is made, and union(d, e),
        5. e-f is made, and union(e, f)
    
    
    (a)           (b)
    *------------* ------------ * (h)
   /             | (g)           \
  /              |                \
 * (c)           |                 \
 |               |                  * (i)
 |               |                  |
 * (d)           |                  |
  \              |                  |
   *-------------*------------------* (j)
   (e)          (f)

    --> doing this will lead to f union-ed with b, but they're already in the same union.
        1. Special condition will be done to make the whole set a "sector".
        2. Connecting sectors will be done through a point set in the same position as another (i.e. (g) and (b))
        3. This will be checked and accounted for.