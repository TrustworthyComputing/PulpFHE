#!\bin\bash

make client_scripts_cpu

make encalu_cpu

(cd cloud_enc/ && rm *.data)

(cd cloud_enc/encrypted_data && rm *.data)

(cd client/ && ./keygen)

(cd client/ && bash ppscript.sh)

(cd cloud_enc/ && ./encalu_cpu_p) 

#(/bin/python3 /home/circ/Desktop/PulpFHE/Juliet/cloud_enc/juliet_interpreter.py &)


