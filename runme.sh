#!\bin\bash

make client_scripts_cpu

make encalu_cpu

(cd /home/circ/Desktop/PulpFHE/Juliet/cloud_enc && rm *.data)

(cd /home/circ/Desktop/PulpFHE/Juliet/client && ./keygen)

(cd /home/circ/Desktop/PulpFHE/Juliet/client/ && bash ppscript.sh)

(cd cloud_enc/ && ./encalu_cpu_p) 

#(/bin/python3 /home/circ/Desktop/PulpFHE/Juliet/cloud_enc/juliet_interpreter.py &)


