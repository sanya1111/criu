#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define CONF_FILE_NAME "tree.conf"
#define PS_FILE_NAME "who_ps.txt"
#define TR_FILE_NAME "tree_ps.txt"

#define NUM_THREADS 3

int** get_node_matrix(int num_row, int num_column);
void delete_node_matrix(int **matrix, int num_row);
void fill_node_matrix(FILE *file_conf, int **matrix, int num_edge);
void get_ps_tree(int **matrix, int num_column, FILE *file_ps, FILE *file_tr);
void get_ps_node(int parrent_node, int **matrix, int num_column, FILE *file_ps, FILE *file_tr);
void do_func();

sem_t sem_ps;
sem_t sem_tr;

int main(void)
{
    FILE* file_conf;
    FILE* file_ps;
    FILE* file_tr;

    int num_node, num_edge;
    int **node_matrix;

    pid_t main_pid;

    // get node_matrix
    file_conf = fopen(CONF_FILE_NAME, "r");
    if (file_conf == NULL)
    {
        printf("File %s is not open!\n", CONF_FILE_NAME);
        return 0;
    }

    fscanf(file_conf, "%d\n%d", &num_node, &num_edge);
    node_matrix = get_node_matrix(num_node, num_node);
    fill_node_matrix(file_conf, node_matrix, num_edge);

    fclose(file_conf);

    // get ps_tree
    file_ps = fopen(PS_FILE_NAME, "w");
    if (file_ps == NULL)
    {
        printf("File %s is not open!\n", PS_FILE_NAME);
        return 0;
    }

    file_tr = fopen(TR_FILE_NAME, "w");
    if (file_tr == NULL)
    {
        printf("File %s is not open!\n", TR_FILE_NAME);
        fclose(file_ps);
        return 0;
    }

    fprintf(file_ps, "%d\n", num_node);
    fflush(file_ps);
    fprintf(file_tr, "%d\n%d\n", num_node, num_edge);
    fflush(file_tr);

    main_pid = getpid();
    
    if (sem_init(&sem_ps, 1, 1) == -1 || sem_init(&sem_tr, 1, 1) == -1)
    {
    	printf("Semaphores is not init!\n");
        fclose(file_tr);
        fclose(file_ps);
        return 0;
    }

    // get processes
    get_ps_tree(node_matrix, num_node, file_ps, file_tr);

    // completion
    if (getpid() == main_pid)
    {
    	sem_destroy(&sem_ps);
    	sem_destroy(&sem_tr);
    	
        fclose(file_tr);
        fclose(file_ps);

        delete_node_matrix(node_matrix, num_node);
    }

    return 0;
}

int** get_node_matrix(int num_row, int num_column)
{
    int **matrix;
    int i;

    matrix = (int **)malloc(num_row * sizeof(int *));

    for (i = 0; i < num_row; ++i)
    {
            matrix[i] = (int *)malloc(num_column * sizeof(int));
            memset((void *)matrix[i], -1, num_column * sizeof(int));
    }

    return matrix;
}

void delete_node_matrix(int **matrix, int num_row)
{
    int i;
    for (i = 0; i < num_row; ++i)
    {
        free(matrix[i]);
    }

    free(matrix);
}

void fill_node_matrix(FILE* file_conf, int **matrix, int num_edge)
{
    int parrent, child;
    int i;
    for (i = 0; i < num_edge; ++i)
    {
        fscanf(file_conf, "%d %d", &parrent, &child);
        matrix[parrent][child] = 1;
    }
}

void get_ps_tree(int **matrix, int num_column, FILE *file_ps, FILE *file_tr)
{
    get_ps_node(0, matrix, num_column, file_ps, file_tr);
}

void get_ps_node(int node, int **matrix, int num_column,  FILE *file_ps, FILE *file_tr)
{
    int child_node;
    int num_child;

    pid_t pid;
    pid_t current_pid;

    pthread_t thread[NUM_THREADS];
    int thread_status;
    
    int ps_status;

    int i;

    num_child = 0;
    pid = -1;
    current_pid = getpid();

    // get child node
    for (i = 0; i < num_column; ++i)
    {
        if (matrix[node][i] == 1)
        {
            child_node = i;
            ++num_child;

            pid = fork();
            if (pid == 0)
            {
                break;
            }
            else if (pid == -1)
            {
                printf("Error fork: process #%d\n", child_node);
            }
            else
            {
                sem_wait(&sem_tr);

                fprintf(file_tr, "%d %d\n", current_pid, pid);
                fflush(file_tr);

                sem_post(&sem_tr);
            }
        }
    }

    // work process
    if (pid == 0) //child
    {
        get_ps_node(child_node, matrix, num_column, file_ps, file_tr);
    }
    else //parrent
    {
        sem_wait(&sem_ps);

        fprintf(file_ps, "%d %d\n", node, current_pid);
        fflush(file_ps);

        sem_post(&sem_ps);

        // get threads

        for (i = 0; i < NUM_THREADS; ++i)
        {
            thread_status = pthread_create(thread + i, NULL, (void *)do_func, NULL);
            if (thread_status != 0)
            {
                printf("Error thread create: thread #%d\n", i);
            }
        }

        // wait threads and process
        for (i = 0; i < NUM_THREADS; ++i)
        {
            pthread_join(thread[i], NULL);
        }

        for(i = 0; i < num_child; ++i)
        {
            wait(&ps_status);
        }
    }
}

void do_func()
{
    while (1)
    {
        sleep(1);
    }
}
