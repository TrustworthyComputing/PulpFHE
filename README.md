<h1 align="center">PulpFHE </h1>
<h3 align="center">Complex Instruction Set Extensions for FHE Processors</h3>

### What is PulpFHE
PulpFHE is an extension unit with optimized non-linear functions for FHE processors. This project is a deployment of the new instructions set on its sister project, [Juliet](https://github.com/TrustworthyComputing/Juliet).

*The following instructions are the same of which are found in the Juliet repository*


### Prerequisites 
1. Install [TFHE](https://github.com/tfhe/tfhe) 
2. Update library path if necessary:
```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:<TFHE_INSTALL_DIR>/lib
export LIBRARY_PATH=$LIBRARY_PATH:<CUFHE_DIR>/cufhe/bin
export CPLUS_INCLUDE_PATH=$CPLUS_INCLUDE_PATH:<CUFHE_DIR>/cufhe/include
```

### Client Setup
1. Run ``make client_scripts_cpu`` for the CPU-based TFHE backend.
2. Navigate to the ``client`` directory and run ``./keygen.out`` to generate an
   FHE keypair. The evaluation key needed for FHE operations will be placed in
   the ``cloud_enc`` directory.
3. Create a file named ``preAux.txt`` and load this with cleartext integer inputs (one
   per line) that represent sensitive data and will serve as sensitive program inputs
   for Juliet. 
4. Run ``./ppscript.sh`` to automatically generate a ciphertext memory
   directory, which will be placed in the ``cloud_enc`` directory. 
5. Upload the entire ``cloud_enc`` directory to the cloud server.

### How to cite this work



<p align="center">
    <img src="./logos/twc.png" height="20%" width="20%">
</p>
<h4 align="center">Trustworthy Computing Group</h4>
