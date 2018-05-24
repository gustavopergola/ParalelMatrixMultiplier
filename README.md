# Multiplicação Matrizes MPI

## Como utilizar
  Use o parametro 'g' para gerar uma nova matriz.

## Como instalar MPI
    mkdir openmpi
    cd openmpi
    wget https://download.open-mpi.org/release/open-mpi/v3.1/openmpi-3.1.0.tar.gz
    tar -xzvf openmpi-3.1.0.tar.gz
    cd openmpi-3.1.0
    ./configure --prefix=$HOME/openmpi
    make all
    make install
    
    Após isso adicione essa linha ao seu .bashrc e reinicie seu terminal:
    
    export PATH=$PATH:$HOME/openmpi/bin
