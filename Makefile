client_scripts_cpu: client/keygen.c client/preprocessor.c client/decrypt.c client/filenames.c 
	gcc -o client/keygen client/keygen.c -ltfhe-spqlios-fma
	gcc -o client/preprocessor client/preprocessor.c client/filenames.c -ltfhe-spqlios-fma
	gcc -o client/decrypt client/decrypt.c -ltfhe-spqlios-fma

encalu_cpu: cloud_enc/encalu_p.cpp cloud_enc/filenames_p.cpp
	g++ -o cloud_enc/encalu_cpu_p cloud_enc/encalu_p.cpp cloud_enc/filenames_p.cpp -ltfhe-spqlios-fma -fopenmp

