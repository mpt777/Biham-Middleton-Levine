# How to build and run it on Linux

* Compile and install OpenMPI from source. For more information about how to do this see, 
https://www.open-mpi.org/faq/?category=building.

* Install SFML. This can be done on Linux with the following command.

`sudo apt-get install libsfml-dev`

* Clone project from Github.

`git clone git@github.com:mpt777/Biham-Middleton-Levine.git`

* Compile project

`mpic++ Source.cpp -o sfml-app -lsfml-graphics -lsfml-window -lsfml-system`

* Run project

`mpirun -np 4 sfml-app`

or

`mpirun -np 4 sfml-app [X_SIZE] [Y_SIZE] [FILL_PERCENTAGE]`
