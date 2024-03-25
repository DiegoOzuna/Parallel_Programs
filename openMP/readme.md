I utilized brew to install openMP. 

I ran brew install libomp to get the libraries for openMP and also used brew install llvm.

To run the programs in vscode. I do the following steps within the terminal at the openMP directory...

export CC=$(brew --prefix llvm)/bin/clang
and then I follow with...
$CC -fopenmp (name of the file).c -o (name of the file)

This will produce the executable to run afterwards. ( ./(name of file) )