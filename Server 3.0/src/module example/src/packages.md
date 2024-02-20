### Install

<b align="center">Fedora</b>
    
    cd ~
    dnf install boost-devel boost-iostrams cmake g++ gcc git json-devel libcurl-devel libpqxx-devel libpq libpq-devel openssl-devel python3-devel
    git clone https://github.com/Firewolf304/web-server.git
    cd web-server/Server\ 2.0/src/
    cmake .
    cmake --build ./
    mv Server_2_0 ~/web-server/Server\ 2.0/files/Server_2_0
    ./Server_2_0

Needing versions:

    boost-devel >1.76
    boost-iostreams > 1.76
    cmake >3.24
    g++ 13.2.1
    json-devel >3.11.2
    libcurl-devel 7.79.1
    libpqxx-devel 7.6.1-7.7.5
    openssl-devel >1.1.1q.1fc35
    mono-devel 6.12.0
    glibc >2.3.4

<b align="center">Arch (not ready)</b>

    pacman -S nlohmann-json boost curl gcc boost-libs mono-tools git cmake openssl
    git clone https://github.com/Firewolf304/web-server.git
    cd web-server/Server\ 2.0/src/
    cmake .
    cmake --build ./
    mv Server_2_0 ~/web-server/Server\ 2.0/files/Server_2_0
    ./Server_2_0

Needing versions:
    
    boost-devel >1.76
    boost-iostreams > 1.76
    cmake >3.24
    g++ 13.2.1
    json-devel >3.11.2
    libpqxx 7.7.5 (better to build https://github.com/jtv/libpqxx/releases/tag/7.7.5)
    openssl >3.1.3-1x
    mono-tools 6.12.0
    glibc >2.3.4


