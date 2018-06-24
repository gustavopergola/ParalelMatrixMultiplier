# Multiplicação Matrizes MPI

## Como utilizar
  
  Use o parâmetro **'g'** para gerar uma nova matriz.<br/>
  Use o parâmetro **'g+'** para gerar novas matrizes com conteúdo randômico.<br/>
  Use o parâmetro **s** para utilizar o método sequencial. *Utilize -np 1* </br>
  Use o parâmetro **m1** para utilizar o método paralelo 1 (MPI).

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

## Para rodar o programa

    mpicc main.c -o main.o
    mpirun -np 2 -mca plm_rsh_no_tree_spawn 1 main.o
    
    OBS: -np X , onde X representa o número de processos a serem criados
    
