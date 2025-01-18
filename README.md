<h1 align="center">PulpFHE </h1>
<h3 align="center">Complex Instruction Set Extensions for FHE Processors</h3>

### What is PulpFHE
PulpFHE is an extension unit with optimized non-linear functions for FHE processors. This project is a deployment of the new instructions set on its sister project, [Juliet](https://github.com/TrustworthyComputing/Juliet).

This project introduces several optimizations and extensions to the existing Juliet architecutre.


### Prerequisites 
1. Install [TFHE](https://github.com/tfhe/tfhe) 
2. Update library path if necessary:
```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:<TFHE_INSTALL_DIR>/lib
export LIBRARY_PATH=$LIBRARY_PATH:<CUFHE_DIR>/cufhe/bin
export CPLUS_INCLUDE_PATH=$CPLUS_INCLUDE_PATH:<CUFHE_DIR>/cufhe/include
```
### How to Run
1. Navigate to the ``client`` directory, open the ``ppscript.sh`` file, and edit the ``wordsize`` variable to specify the bit size of the data.
     * PulpFHE supports 8, 16, and 32-bit data.
2. Edit the ``preAux.txt`` file to input the plaintext data to be encrypted and processed.
3. From the ``PulpFHE`` directory, launch the server by running ``bash runme.sh``.
4. From another terminal, navigate to the ``cloud_enc`` directory and run the processor by running the interperter using the following command: 
``python3 pulpfhe_interpreter.py <program-name> [wordsize] [number-of-registers]``.
   * The ``<program-name>`` is the name of the assembly file in the ``Benchmarks`` directory.
   * The ``[wordsize]`` is the size of the plaintext data in bits. The default is 8.
   * The ``[number-of-registers]`` is the number of registers that processor will use. The defualt is 128.
   * Please note that when passing ``<program-name>``, just pass the filename and the interperter will automatically fetch it from the ``Benchmarks`` directory.
5. To print the output, go to the ``client`` directory, and run ``./decrypt ../cloud_enc/output.data <wordsize>``.

The companion upgraded HE-Java compiler can be found at [HE-Java-Compiler](https://github.com/OmarAlmighty/HEJava-compiler/tree/master)


<p align="center">
    <img src="./logos/twc.png" height="20%" width="20%">
</p>
<h4 align="center">Trustworthy Computing Group</h4>
