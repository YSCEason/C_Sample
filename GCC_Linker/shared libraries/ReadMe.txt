gcc -c -fPIC -o sum.o sum.c

gcc -shared -Wl,-soname,libsum.so.1 -o libsum.so.1.0.0 sum.o


ln -s libsum.so.1.0.0 libsum.so

ln -s libsum.so.1.0.0 libsum.so.1

gcc main.c libsum.so -o main_dynamic