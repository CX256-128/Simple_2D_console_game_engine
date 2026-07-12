## Brief
  This engine is a lightweight, data-driven 2D console game engine written in C++. It adopts an MVC-like architecture, completely separating the game state (variable table), rendering (display table), and business logic (event-tree scripts).  
  
  By using this engine, you can create RPG or puzzle games with rich storylines, triggers, dynamic generation, and combat calculations—without modifying any C++ source code. You only need to write map files and custom assembly-style scripts.  
  
## Quick Start
  You can use the default set for the engine.In your own .cpp files, just do like
  ```C++
#include "engine.h"

int main() {
    // Advised if in linux
    // - > printf("\033c");

    engine my_game;
    
    // initialize：provide variable file、display file、script file
    game.set_up("variable_table.txt","display_table.txt","event_script.txt");
    
    // Run the main loop of the game，this parameter is the gap millionsecs between frames（16ms $\implies$ 60 FPS）
    my_game.run(16);
    
    return 0;
}
  ```

## Support file
### variable file
Player initial state and registers. Values separated by spaces or newlines.  
**Format:**
```text
[version code: 1.0]\n
<current blood> <current power> <current shield>\n
<max blood> <max power> <max shield>\n
<initial x coordination> <initial y coordination> <initial direction> <sight range>
```

### display file
Map layout, obstacle chars, and player appearance definitions.  
**Format:**
```text
[version code: 1.0]\n
<Map width X> <Map height Y>\n
<symbol for the player like &> [Obstacle character set: tiles that cannot be passed through, e.g., #, @, *]\n
[Below is the raw map character array. Characters are contiguous (no spaces), and each line maps directly to a row of the game map ...]...
...
```

### script file
The logic which is triggered when the player steps on specific coordinates, including the trigger table and code snippets.  
**Format**
```text
<Total number of triggers: N>
<Trigger1_Label> <X-coordinate> <Y-coordinate> ... (N in total)
: <Label1>
[Script code...]
endbr
: <Non-trigger label>
[Script code...]
endbr
: <Label2>
[Script code...]
endbr
```

**Tips：The name of the label can't be endbr**


## Manual for the script file
This is the core feature of the engine: a minimal interpreter designed around a in-built-register-based architecture.

### Registers and Keywords
You can use the following keywords as the source or destination of an operation:
- **Base Attributes:**  
  `blood`, `power`, `shield`, `max_blood`, `max_power`, `max_shield`
- **Coordinates & Status:**  
  `x`, `y`, `direction`, `sight`
- **General-Purpose Registers:**  
  `eax`, `ebx` (integer), `rax`, `rbx` (long integer), `xmm0`, `xmm1` (floating-point)
- **Generator Queue (Write-Only):**  
  `gen_x`, `gen_y` (spawn coordinates), `gen_sym` (spawn character)

### Data Prefix Symbols
Parameters in instructions must be prefixed with a symbol to indicate their type:
|Prefix|Type|example|
|------|----|-------|
|  `$` |Integer immediate|	`$ 10`, `$ -5`|
|  `.` |Floating-point immediate|	`. 0.5` |
|  `*` |Character literal	|`* A` (Note: only one character allowed; commonly used with `% gen_sym`)|
|  `%` |Register / Variable|	`% blood`, `% eax`|

### Instruction Set Reference
**Data Movement & Modification**  
`mov <src> <dest>` – Assigns data to the destination.
+ Example: `mov $ 10 % power` (sets power to 10)
+ Example: `mov * X % gen_sym` (prepares to spawn character 'X')

`add <src1> <src2> <dest>` – Addition (subtraction can be done using negative values). dest = src1 + src2
+ Example: `add % blood $ -5 % blood` (deducts 5 HP)

`multiply <src1> <src2> <dest>` – Multiplication.

`divide <src1> <src2> <dest>` – Division (with built-in division-by-zero protection).  

---
**Control Flow & Jumps**  
`cmp <src1> <src2>` – Compares two values and implicitly sets the internal cmp_flag.

`je <label>` – Jump if Equal.

`jne <label>` – Jump if Not Equal.

`jg <label>` – Jump if Greater.

`jl <label>` – Jump if Less.

`goto <label>` – Unconditional jump to the specified label.

`endbr` – Required. Ends the execution of the current branch and returns to the game.

---
**System Calls**  
`call msg <string... ends with '\n'>` – Prints a line of message at the bottom of the screen (supports full sentences with spaces; reads the entire line without requiring quotation marks).
+ Example: `call msg The monster hits you for 5 damage!\n`

`call rand . 1.0 % <dest>` – Generates an integer random number (0 ~ RAND_MAX , as defined in the C standard library) and stores it in `<dest>`.

`call rand . 0.5 % <dest>` – Generates a floating-point random number (0.0 ~ 1.0) and stores it in <dest>. Typically used with `% xmm0` or `% xmm1`.

---
**Setting a timer**  
`wait <seconds> <label>` - Set a timer, and we execute the code after this sentence before the label only after this period.  
+ Example:
 ```text
wait 2 and     ; set a 2-second timer
mov * a % sym  ; code here will be executed after 2 seconds
mov $ 2 % sym_x
mov $ 2 % sym_y
: and
...            ; And the main loop will do other things here
```
