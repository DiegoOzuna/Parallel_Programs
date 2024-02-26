#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char** argv) {
    int rank, size, n, cols_per_p;
    MPI_Init(&argc, &argv); //initialize mpi process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); //returns the unique identifier for a process in a group
    MPI_Comm_size(MPI_COMM_WORLD, &size); //returns the total num of processes in group

    /////////////////////////////////////////////////////////////////////////
    //   NOTE: This program makes the following assumptions...             //
    //      1. The matrix is square of order n                             //
    //      2. n is evenly divisible by comm_sz                            //
    //      3. We are multiplying the matrix by a vector n                 //
    //      4. Each column of the matrix is being passed to the processes  //
    //      5. Each process will also have piece of the vector             //
    /////////////////////////////////////////////////////////////////////////


    //Ask for n ...
    if (rank == 0) {
        printf("Input a number n that is evenly divisible by the number of processes going...: \n"); 
        scanf("%d", &n);
        cols_per_p = n/size; //used to make sure we distribute correct column-blocks to each process
    }
   
    MPI_Bcast(&n,1,MPI_INT,0,MPI_COMM_WORLD); //share n with every process...
    MPI_Bcast(&cols_per_p, 1, MPI_INT, 0, MPI_COMM_WORLD); //share cols_per_p with every process...

    int recv_data[n][cols_per_p];    //this is a buffer for each process.
    int recv_vec[cols_per_p];      //this is buffer to hold a segment of the vector
    int vector[n];
    

    //Rank 0 will make the nxn matrix and the nx1 vector
    if(rank==0){
        int two_d[n][n];

        //generate random matrix and vector for the program...
        for(int i=0; i<n; i++){
            vector[i] = rand() % 10 + 1;                              //NOTE: you can change the upper and lower limit to whatever is desired. for demonstration, its set to small numbers.
            for(int j=0; j<n; j++){
                two_d[i][j] = rand() % 10 + 1;
            }
        }

        MPI_Scatter(vector, cols_per_p, MPI_INT, recv_vec, cols_per_p, MPI_INT,0,MPI_COMM_WORLD);



        //print the matrix...
        printf("The Matrix Generated: \n");
        for(int i=0; i<n; i++){
            for(int j=0; j<n; j++){
                printf("  %d  ", two_d[i][j]);
            }
            printf("\n");
        }

        // Print the vector...
        printf("The Vector Generated:\n");
        for(int i=0; i<n; i++){
            printf("%d \n", vector[i]);
        }
        printf("\n");

        int send_data[n]; //buffer to send columns
        

        //Distribute the matrix among the processes by column...
        for(int i=0; i<size; i++) {
            for(int j=0; j<cols_per_p; j++) {
                int col_index = i*cols_per_p + j;
                for(int k=0; k<n; k++){
                    send_data[k]= two_d[k][col_index]; //note: this gives us the column array
                }

                //NOTE: This is done so that process 0 can be informed of its column blocked piece. 
                //      If we were to use "send" to inform 0 about this info, we would end up in a 
                //      deadlock where 0 passes and immediately wants to recieve

                if(i == 0){
                    for(int l=0;l<n;l++){
                        recv_data[l][j] = send_data[l]; //direct copy for process 0.
                    }
                } else {
                    MPI_Send(send_data, n, MPI_INT, i, 0, MPI_COMM_WORLD); //once column array is obtained, send
                }
            }                                               
        }
        printf("Finished Sending Information... \n\n");
    } else { 
        MPI_Scatter(NULL, cols_per_p, MPI_INT, recv_vec, cols_per_p, MPI_INT,0,MPI_COMM_WORLD);  //each process will communicate with scatter to get their portion of the vector...
    }

    // All processes, except 0, receive their block of the matrix through sent message
    if(rank != 0){
        int temp_recv_data[n]; //temp buffer
        for(int i=0; i<cols_per_p; i++){
            MPI_Recv(&temp_recv_data, n, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // Receive one column at a time
            printf("Process %d just received column %d\n", rank, i);
            for(int j=0; j<n; j++){
                recv_data[j][i] = temp_recv_data[j]; //Assign the recieved data by column
            }
            printf("\n");
        }   

    }

    //Let each process print out their column block recieved....
    printf("Process %d's recv_data contents: \n", rank);
    for(int i=0; i<n; i++){
        for(int j=0; j<cols_per_p; j++){
            printf("%d ", recv_data[i][j]);
        }
        printf("\n");
    }
    printf("\n");

    //Let each process print out their portion of the recieved vec...
    printf("Process %d's recv_vec contents: \n", rank);
    for(int i=0; i<cols_per_p; i++){
        printf("%d ", recv_vec[i]);
    }
    printf("\n");


    /*NOW WE CAN ACTUALLY DO THE MATH. Idea is simple enough, let each process do matrix multiplication and then add all at the end to get the matrix-vec product*/

    int presult[n];  //this will hold each processes' result at the end of their local calculation

    for(int i=0;i<n;i++){     //this is basically each row x vector (column)
        presult[i] = 0;
        for(int j=0; j<cols_per_p;j++){
            presult[i] += recv_data[i][j] * recv_vec[j];
        }
    }

    // Print the partial result of each process
    printf("Process %d's partial result: \n", rank);
    for(int i=0; i<n; i++){
        printf("%d ", presult[i]);
    }
    printf("\n");

    // So each process now holds their partial results. We can combine all these partial results to get the final result...
    if(rank == 0){
        int total_result[n];     //again because of assumptions, nxn * nx1 will be n result
        MPI_Reduce(presult, total_result, n, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        //print the final result
        printf("~~~~~~The final result is: ~~~~~~\n");
        for(int i=0; i<n; i++){
            printf("%d ", total_result[i]);
        }
        printf("\n");
    } else {
        MPI_Reduce(presult, NULL, n, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD); //each process sends their partial result to 0 where the sum is done
    }


    

    MPI_Finalize(); //close opened resources from MPI
    return 0;
}
