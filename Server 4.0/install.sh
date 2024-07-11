git clone https://github.com/Firewolf304/web-server.git && cd web-server && cd Server\ 4.0
cd src && mkdir build && cmake .. --build -G Ninja
ninja -j $(nproc)
cp Server4_0  ~/web-server/
