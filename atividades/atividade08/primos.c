// Nota: 1,5
#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include<malloc.h>
#include <stdlib.h>


int primo (int n) {
    int i;
    if (n <= 1) return 0;
    if (n == 2) return 1;
    if (!(n % 2)) return 0;
    for (i = 3; i < (sqrt(n)+1); i+=2) {
        if (n%i == 0) return 0;
    }
    return 1;
}
      
////// O numero N precisa ser divisível pelo valor do chunk

int main(int argc, char *argv[]) {


    int source = 0, destination = 0, n, elem;
    double * a, *b, *c;
    int chunk;

    int tag = 0, root=0;
    int rank, size, p, number_chunks;

   


    double starttime, endtime;
    starttime = MPI_Wtime();

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);



   
    if(rank ==root){
        n = atoi(argv[1]);
        chunk = atoi(argv[2]);
        number_chunks =  n/chunk;



    }

    // Certo. É uma chamada bloqueante, mas ainda não é o grosso da computação.
    MPI_Bcast(&chunk,1, MPI_INT,root, MPI_COMM_WORLD);

    if(rank == root){
        int number_chunks_used = 0;
        MPI_Request send_request[size - 1];
        MPI_Request recv_request[size - 1];
        MPI_Status status[size - 1];

        //Será retorado no máximo chunk primos por execucao
	// Isso aqui sem malloc é propenso a erro.
        int primos[size ][chunk];
        int total_num_received[size - 1];

        int number_process_used;
        int number_send ;
        int primos_total[n];
        int primos_total_numero = 0;
        int local_work = 0;
        //Caso o valor n seja menor que o chunk é executado local
        if(number_chunks == 0){
            for(int i = 2; i < n; i++){
                if(primo(i)){
                    primos_total[primos_total_numero] = i;
                    primos_total_numero++;


                }
            }
        }
        while(number_chunks_used < number_chunks){
            number_process_used = 0;
            for(int i = 0; i <size && number_chunks_used < number_chunks; i++){
            //Informa a posição inicial e cada processo sabe a quantidade em um chunk

               number_send = number_chunks_used*chunk + 1;

                if(i != 0){
                    MPI_Isend(&number_send,1, MPI_INT, i, tag, MPI_COMM_WORLD, &send_request[i-1]);
                    MPI_Irecv(primos[i], chunk, MPI_INT, i, tag, MPI_COMM_WORLD, &recv_request[i-1]);

                }
                else{

                    local_work = number_send;

                }
                printf("%d send \n",number_send );

                number_chunks_used++;
                number_process_used++;
            }

            int primos_encontrados = 0;
            printf("%d local\n",local_work );
            for(int i = local_work; i < local_work + chunk; i++){
                if(primo(i)){
                    primos[0][primos_encontrados] = i;
                    primos_encontrados++;
                }
            }



            MPI_Waitall(number_process_used-1, send_request, status);

            MPI_Waitall(number_process_used-1, recv_request, status);
            printf("Teste\n");

            for(int i =0; i < number_process_used ; i++){
                if(i ==0){
                    for(int j = 0; j < primos_encontrados; j++){
                        primos_total[primos_total_numero] = primos[i][j];

                        primos_total_numero++;
                    }
                }
                else{


                    MPI_Get_count(&status[i-1], MPI_INT, &total_num_received[i-1]);
                    for(int j = 0; j < total_num_received[i-1]; j++){
                        primos_total[primos_total_numero] = primos[i][j];

                        primos_total_numero++;
                    }
                }   


            }

        }

        int send_exit = -1;
        for(int i =1; i < size; i++){
            MPI_Send(&send_exit, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
        }

        for(int i =0; i <primos_total_numero;i++){
            printf("%d ",primos_total[i] );
        }
        printf("\n");
    }

    else{
        while(1){

            int init_pos;
            MPI_Request send_request;
            MPI_Request recv_request;
            MPI_Status status;
            int primos[chunk];
            int primos_encontrados = 0;
            MPI_Irecv(&init_pos, 1, MPI_INT, root, tag, MPI_COMM_WORLD, &recv_request);
            primos[0] = 3;
            MPI_Wait(&recv_request, &status);

            if(init_pos != -1){
                for(int i = init_pos; i < init_pos + chunk; i++){
                    if(primo(i)){
                        primos[primos_encontrados] = i;
                        primos_encontrados++;
                    }
                }

                MPI_Isend(primos, primos_encontrados, MPI_INT, root, tag, MPI_COMM_WORLD, &send_request);
                
                MPI_Wait(&send_request, &status);
            }
            else break;
        }

    }  
    MPI_Finalize();

    return 0;
}
