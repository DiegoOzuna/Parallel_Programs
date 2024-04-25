#include <mpi.h>
#include <iostream>
#include <vector>
#include <math.h>

int main(int argc, char* argv[]){

    int commsize;
    int rank;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &commsize);

    // Check if the command-line argument is provided
    if(argc < 2) {
        if(rank == 0) {
            std::cout << "Please provide the search max as a command-line argument." << std::endl;
        }
        MPI_Finalize();
        return 0;
    }
    //create a vector with howerver many 0's are needed
    // Convert the command-line argument to integer
    int search_max = std::stoi(argv[1]);

    
    int sqrt_search_max = (int) sqrt(search_max);
    //All porcesses will have this vector to work with
    std::vector<int> numbers(search_max, 0);

    if(rank == 0){
        //For rank 0, fill the vector with all the numbers from 1 to search_max
        for(int i = 0; i < search_max; i++){
            numbers[i] = i+1;
        }

        //For the sieve, we need to mark 1 as nonprime
        numbers[0] = 0;

        //Iterate through the array of numbers and then send that to all other processes
        for(int i = 0; i<sqrt_search_max;i++){
            // store the current number in a temp variable
            int current_number = numbers[i];

            //then send the number
            for(int j=1; j<commsize;j++){
                MPI_Send(&current_number, 1, MPI_INT, j, 0, MPI_COMM_WORLD);
            }

            //If the current number is prime, then this process recieves nonprime numbers
            //from all other processes and updates its vector of numbers by corssing out those 
            //nonprimes.

            if(current_number != 0){
                //send the array to other processes
                for(int i =1;i<commsize;i++){
                    MPI_Send(&numbers[0], numbers.size(), MPI_INT, i, 0, MPI_COMM_WORLD);
                }

                //All other processes are sending back the array with changes
                //Use a temporary vector to store those changes

                std::vector<int> temp_vector(search_max, 0);

                //Recieve the arrays from all other processes

                for(int i =1; i<commsize; i++){
                    MPI_Recv(&temp_vector[0], temp_vector.size(), MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                

                    //find out what values rank 1 had changed. this uses the same calculation as used in the else

                    int current_prime = current_number;
                    int starting_index = current_number -1 + i * current_prime; //use i instead of rank
                    int stride = current_prime * (commsize-1);

                    //get any updated values stored in the temp.vector
                    for(int j = starting_index; j<temp_vector.size();j+=stride){
                        if(temp_vector[j] == 0)
                            numbers[j] = temp_vector[j];
                    }
                }
            }
        }
        for(int i =0;i<search_max;i++){
            if (numbers[i]!=0) printf("%d ", numbers[i]);
        }
    } else {
        //Other process will be receiving a message from rank 0,
        // one for each element in the array of numbers
        for(int i=0;i<sqrt_search_max;i++){
            int recieved_number;
            MPI_Recv(&recieved_number, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            //If the current number is prime then this process find and sends
            //nonprime numbers back to rank 0

            if(recieved_number != 0){
                //for other processes, it will simply wait for rank 0 to send numbers,
                //so it will use the vector to recieve a message
                MPI_Recv(&numbers[0], numbers.size(), MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                //lets suppose we are to cross out every even number (except 2) in the array by replacing with zeros
                int current_prime = recieved_number;
                int starting_index = current_prime - 1 + rank * current_prime;
                int stride = current_prime * (commsize - 1);
                
                for(int i = starting_index; i<numbers.size(); i+= stride){
                    numbers[i]=0;
                }
                
                MPI_Send(&numbers[0], numbers.size(), MPI_INT, 0, 0, MPI_COMM_WORLD);
            }  
        }
    }

    MPI_Finalize();
    return 0;

}