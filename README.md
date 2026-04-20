# Cellular Automata Engine


A fast **Cellular Automata Engine** built in **C++** with **SFML**, currently running **Conway’s Game of Life**.

This project is designed around a clear separation between a **logical grid** and a **physical grid**, where the physical grid contains extra padding cells to reduce branching in hot loops and make neighbourhood processing more efficient.

The main goal of me building this was to benchmark and stress test my ECS Library. It passed the test and only stopped at a fundamental limit of the ECS library, being thread safe, which my library is currently not.
The engine is being built with performance, clarity, and experimentation in mind.

---

## Features

- Conway’s Game of Life implementation
- Logical grid to physical grid mapping
- Physical padding around the grid to simplify neighbour traversal
- ECS-based entity/component design
- SFML rendering using `sf::VertexArray`
- Interactive cell drawing and erasing
- Simple profiling utilities for timing hot sections

---

## Core Idea

This engine uses two grid concepts:

### Physical Grid
The **physical grid** is the actual grid that exists in memory.

It includes extra padding cells around the playable area. These padded cells help simplify neighbour calculations because edge cases become regular cases.

### Logical Grid
The **logical grid** is the visible, meaningful simulation area.

It does **not** physically exist as a separate allocation. It is a logical view mapped onto the physical grid.

### Why this exists

Neighbour-based cellular automata often suffer from boundary checks in hot loops.

Instead of checking:

- is this cell on the edge?
- does this neighbour exist?
- do I need to branch here?

this engine adds padding around the actual simulation region so neighbour scans can be performed more uniformly.

That means:

- cleaner neighbour iteration
- fewer edge-condition branches
- better cache-friendly traversal patterns
- simpler hot-loop logic

---

## Current Simulation

At the moment, the engine runs:

- **Conway’s Game of Life**

Rules:

1. Any live cell with fewer than two live neighbours dies
2. Any live cell with two or three live neighbours lives on
3. Any live cell with more than three live neighbours dies
4. Any dead cell with exactly three live neighbours becomes alive
 
<br> 
<img width="841" height="863" alt="2d CAE pic0" src="https://github.com/user-attachments/assets/97d202bc-559c-4b00-b442-6926534d1591" />

### Note: This engine can run any other cellular automata simualtion just by changing its rulebook and neighbour counting range

---

## Tech Stack

- **C++**
- **SFML**
- Custom ECS : [My Entity Component System](https://github.com/TerminalBoy/My_Entity_Component_System)
- Custom PRNG (pseudo random number generator, seed based) (`xorshift32`)

---

## Controls

- **Left Mouse Button** → draw live cells
- **Right Mouse Button** → erase cells
- Hold **Left Ctrl** → pause simulation

---

## Rendering

Rendering is done with `sf::VertexArray` quads instead of drawing each cell as a separate shape object.

This keeps rendering more efficient and gives tighter control over visual updates.

The engine currently renders:

- cell bodies
- horizontal borders
- vertical borders

---

## <span style="color: skyblue;"> Multithreading Model: Coming soon!!

The engine divides the logical grid into thread segments.

Each participating thread receives a segment using this integer partitioning formula:

```cpp
start_index = t * N / T
end_index   = (t + 1) * N / T
// end index is exclusive
