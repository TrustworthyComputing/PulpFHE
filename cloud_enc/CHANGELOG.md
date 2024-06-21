# The Dawn
* `juliet_interpreter.py`:
    
    * Cancelled sending `exit` command to exit the server.

* `encalu_p.cpp`:

    * Added the new instructions.
    * The addition instruction uses the `p_adder` function to map the execution to the Kogge-Stone adder. If the adder doesn't support a specific word size, it uses the normal adder function. 
    * The serial normal adder function itself is modified to be parallelizable.
    * The `listen_for_inst` function is refactored to fit in the cpp file.

# Adding div and mod

* `juliet_interpreter.py`:
    
    * Enabled sending `exit` command to exit the server to avoid socket binding failure.

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
    * Added the `esqrt` instruction.

* `encalu_p.cpp`:
    * The `esqrt` instruction has been integrated and assigned ID 114.
    
# Adding variance

*  `juliet_interpreter.py`:
    * Added the `evar` instruction.
    * Added the `secread_l` for reading a list of ciphertexts.
    * Added the `elist` for creating a list of ciphertexts object.

*  `encalu_p.cpp`
    * The `evar` instruction has been integrated and assigned ID 115.
    * Added `split` function to split list of ciphertexts received from the interpreter.
    
# Adding mean

*  `juliet_interpreter.py`:
    * Added the `emean` instruction.

*  `encalu_p.cpp`
    * The `emean` instruction has been integrated and assigned ID 116.

# Adding std, relu

*  `juliet_interpreter.py`:
    * Added the `estd` and `erelu` instructions.

*  `encalu_p.cpp`
    * The `estd` instruction has been integrated and assigned ID 117.
    * The `erelu` instruction has been integrated and assigned ID 118.

# Fixed relu

* `relu` had an issue with comparing negative numbers.
* `client\decrypt.c` didn't print negative number, I fixed this by converting the number 
into the 2's complement representation.

