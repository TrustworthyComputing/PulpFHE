# The Dawn
* `juliet_interpreter.py`:
    
    * Canclled sending `exit` command to exit the server.

* `encalu_p.cpp`:

    * Added the new instructions.
    * The addition instruction uses the `p_adder` function to map the execution to the Kogge-Stone adder. If the adder doesn't support a specific word size, it uses the normal adder function. 
    * The serial normal adder function itself is modified to be parallelizable.
    * The `listen_for_inst` funciton is refactored to fit in the cpp file.

# Adding div and mod

* `juliet_interpreter.py`:
    
    * Enabled sending `exit` command to exit the server to avoid socket binding failuer.

* `encalu_p.cpp`:

    * The `div` instruction has been integrated and assigned ID 111
    * The `mod` instruction has been integrated and assigned ID 112

# Adding left and right rotation

* It seems that `juliet_interpreter.py` incorrectly parses the shift amount for the left and right shift instructions.

* `juliet_interpreter.py`:
    * Added the `eror` and `erol` instructions.

* `encalu_p.cpp`:
    * The `eror` instruction has been integrated and assigned ID 105.
    * The `erol` instruction has been integrated and assigned ID 106.
    
# Adding sqrt

* `juliet_interpreter.py`:
    * Added the `esqrt` instructions.

* `encalu_p.cpp`:
    * The `esqrt` instruction has been integrated and assigned ID 114.
    
    
    
    
