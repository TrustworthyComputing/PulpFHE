# The Dawn
* `juliet_interpreter.py`:
    
    * Canclled sending `exit` command to exit the server.

* `encalu_p.cpp`:

    * Added the new instructions.
    * The addition instruction uses the `p_adder` function to map the execution to the Kogge-Stone adder. If the adder doesn't support a specific word size, it uses the normal adder function. 
    * The serial normal adder function itself is modified to be parallelizable.
    * The `listen_for_inst` funciton is refactored to fit in the cpp file.

    
