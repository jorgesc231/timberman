gcc -O3 -o build/timberman Timberman.cpp -I/usr/local/include/SDL2 -I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux -D_REENTRANT -L/usr/local/lib -Wl,-rpath,/usr/local/lib -Wl,--enable-new-dtags -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -DRPI1
