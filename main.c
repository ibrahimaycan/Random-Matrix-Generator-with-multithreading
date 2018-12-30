#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

struct Queue
{
    int front, rear, size;
    unsigned capacity;
    int* id;
};


int globali=0;//holds the id of matrix created by generate threads
int globalj=0;//holds the id of matrix created bymod threads
int globaladdi=0;//holds the id of local sum
int globallog=0;
int **bigmatrix;
int log_matrix_size;
int matrix[500][5][5];
int modmatrix[500][5][5];
int globalsum;




pthread_mutex_t generate_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t inserting_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mode_thread_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t inserting_queue2_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mode_readqueue1_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t log_readqueue1_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t add_readqueue2_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue1_initiliazer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue2_initiliazer_mutex = PTHREAD_MUTEX_INITIALIZER;

struct Queue* createQueue(unsigned capacity);
void enqueue(struct Queue* queue, int item);
void printMatrix(int mat[5][5]);
void * generateThread(void* unused);
void * modThread(void* unused);
void *logThread(void* threadid);
void *addThread(void* unused);
void errorCheck(int argc, char*argv[]);

int dequeue(struct Queue* queue);
struct Queue* queue1=NULL;
struct Queue* queue2=NULL;


int main(int argc, char *argv[]){

    errorCheck(argc,argv);//error checks

    globalsum=0;
    int matrix_size,gen_thread_size,mod_thread_size,add_thread_size,log_thread_size;
    matrix_size=atoi(argv[2]);
    log_matrix_size=matrix_size/5;
    gen_thread_size=atoi(argv[4]);
    log_thread_size=atoi(argv[5]);
    mod_thread_size=atoi(argv[6]);
    add_thread_size=atoi(argv[7]);





    //

    int upperCount=0;


    bigmatrix = (int **)malloc(matrix_size * sizeof(int*));
    for(int i = 0; i < matrix_size; i++)
        bigmatrix[i] = (int *)malloc(matrix_size * sizeof(int));




    pthread_t generate_thread_id[gen_thread_size];
    pthread_t mod_thread_id[mod_thread_size];
    pthread_t add_thread_id[add_thread_size];
    pthread_t log_thread_id[log_thread_size];
    while(upperCount<((matrix_size/5)*(matrix_size/5))) {
        for (int i = 0; i < gen_thread_size; i++) {
            pthread_create(&generate_thread_id[i], NULL, generateThread, (void*)i);
            upperCount++;

            if(upperCount==((matrix_size/5)*(matrix_size/5)))
                break;
        }

    }

    upperCount=0;
    while (upperCount<((matrix_size/5)*(matrix_size/5))){//mod threads created
        for (int i = 0; i <mod_thread_size ;i++) {
            pthread_create(&mod_thread_id[i],NULL, modThread, (void*)i);
            upperCount++;

            if(upperCount==((matrix_size/5)*(matrix_size/5)))
                break;
        }
    }
    upperCount=0;
    while(upperCount<((matrix_size/5)*(matrix_size/5))){////add thread created
        for (int i = 0; i <add_thread_size ; i++) {
            pthread_create(&add_thread_id[i],NULL,addThread,(void*)i);
            upperCount++;
            if(upperCount==((matrix_size/5)*(matrix_size/5)))
                break;
        }
    }
    upperCount=0;

    while(upperCount<((matrix_size/5)*(matrix_size/5))) {
        for (int i = 0; i <log_thread_size ; i++) {
            pthread_create(&log_thread_id[i],NULL,logThread,(void*)i);
            upperCount++;
            if(upperCount==((matrix_size/5)*(matrix_size/5)))
                break;
        }
    }

    int t,rc;


    ////waits all threads to join
    if(log_matrix_size>gen_thread_size){
        for(t=0; t<gen_thread_size; t++)
            pthread_join(generate_thread_id[t],NULL);
    }
    else{
        for(t=0; t<log_matrix_size; t++)
            pthread_join(generate_thread_id[t],NULL);
    }
    if(log_matrix_size>mod_thread_size) {
        for (t = 0; t < mod_thread_size; t++)
            pthread_join(mod_thread_id[t], NULL);
    }
    else{
        for (t = 0; t < log_matrix_size; t++)
            pthread_join(mod_thread_id[t], NULL);

    }

    if(log_matrix_size>add_thread_size) {
        for(t=0; t<add_thread_size; t++)
            pthread_join(add_thread_id[t],NULL);
    }
    else{
        for(t=0; t<log_matrix_size; t++)
            pthread_join(add_thread_id[t],NULL);

    }

    if(log_matrix_size>add_thread_size) {
        for (t = 0; t < log_thread_size; t++)
            pthread_join(log_thread_id[t], NULL);
    }
    else{
        for (t = 0; t < log_matrix_size; t++)
            pthread_join(log_thread_id[t], NULL);
    }


    FILE *f;
    f = fopen("output.txt", "w");       ////writes to the file

    fprintf(f,"The matrix is\n[");
    int m,i;
    for (m = 0; m <matrix_size ; m++) {
        for (i = 0; i <matrix_size ; i++) {
            fprintf(f,"%d",bigmatrix[m][i]);
            if((i!=matrix_size-1)||(m!=matrix_size-1))
                fprintf(f,",");

        }
        if((m==matrix_size-1)&&(i==matrix_size))
           fprintf(f,"]");

        fprintf(f,"\n ");

    }
    fprintf(f,"The global sum is : %d.",globalsum);
    fclose(f);
    pthread_exit(NULL);
    return ;
}

void *generateThread(void* threadid){
    pthread_mutex_lock(&queue1_initiliazer_mutex);

    if(queue1==NULL) {          ///if queue has not created than creates
        queue1 = createQueue(1000);
        for (int j = 0; j < (log_matrix_size)*(log_matrix_size) ; j++) { //initilizing all elemets id of queue with -1
            queue1->id[(queue1->front+j)]=-1;
        }
    }

    pthread_mutex_unlock(&queue1_initiliazer_mutex);


    long tid;
    tid=(long)threadid;
    int i,j,id;
    pthread_mutex_lock(&generate_mutex);    //inserts globali
    id=globali;                 //because of changing the global value  used mutex
    globali++;
    pthread_mutex_unlock(&generate_mutex);
    //printf("%d\n",k);
    for (int i = 0; i < 5 ; i++) {      //creates random namber of matrix
        for (j = 0; j <5 ; j++) {
            matrix[id][i][j]=1+(rand()%99);
            //printf("%d ",matrix[k][i][j]);
        }
        //printf("\n");

    }


    pthread_mutex_lock(&print_mutex);//prints the generated matrix
    printf("\nGenerator_%ld generated following matrix:",threadid);
    printMatrix(matrix[id]);
    printf("This matrix is [%d,%d] submatrix\n",(id/log_matrix_size),(id%log_matrix_size));
    pthread_mutex_unlock(&print_mutex);


    pthread_mutex_lock(&inserting_queue_mutex);///inserts the queue
    enqueue(queue1,id);
    pthread_mutex_unlock(&inserting_queue_mutex);
    return NULL;
}
void * modThread(void* threadid){


    pthread_mutex_lock(&queue2_initiliazer_mutex);///creates queue 2 if does not exist

    if(queue2==NULL){
        queue2 = createQueue(1000);
        for (int j = 0; j < (log_matrix_size)*(log_matrix_size) ; j++) { //initilizing all elemets id of queue with -1
            queue2->id[(queue2->front+j)]=-1;

        }
    }
    pthread_mutex_unlock(&queue2_initiliazer_mutex);


    int i,j,id;
    long tid;
    tid=(long)threadid;
    pthread_mutex_lock(&mode_readqueue1_mutex); //reads queue1
    while (1){
        if(queue1->id[(queue1->front+globalj)]!=-1){
            break;
        }
    }
    id=queue1->id[(queue1->front)+globalj];
    globalj++;
    pthread_mutex_unlock(&mode_readqueue1_mutex);
    for (int i = 0; i <5 ; i++) {
        for (int j = 0; j <5 ;j++) {
            modmatrix[id][i][j]=matrix[id][i][j]%matrix[id][0][0];
        }
    }
    pthread_mutex_lock(&print_mutex);////prints mode of matrix
    printf("\nMOD_%ld generated following matrix:",threadid);
    printMatrix(modmatrix[id]);
    printf("This matrix is generated by [%d,%d] submatrix\n",(id/log_matrix_size),(id%log_matrix_size));
    pthread_mutex_unlock(&print_mutex);

    pthread_mutex_lock(&inserting_queue2_mutex);        //inserts queue2
    enqueue(queue2,id);
    pthread_mutex_unlock(&inserting_queue2_mutex);
}
void *addThread(void* threadid){
    while (queue2==NULL){
        if(queue2!=NULL)
            break;

    }
    int i,j,sum,id,k;
    long tid;
    tid=(long)threadid;
    pthread_mutex_lock(&add_readqueue2_mutex); //reads queue 2
    while (1){
        if(queue2->id[(queue2->front+globaladdi)]!=-1){
            break;
        }
    }
    id=queue2->id[(queue2->front)+globaladdi];
    globaladdi++;
    pthread_mutex_unlock(&add_readqueue2_mutex);
    for (int i = 0; i < 5 ; i++) {
        for (j = 0; j <5 ; j++) {
            sum+=modmatrix[id][i][j];
        }
    }
    pthread_mutex_lock(&print_mutex);////prints global sum
    printf("\nAdd_%d has local sum: %d by [%d,%d] submatrix,\nglobal sum"
           "before/after update: %d/%d\n",tid,sum,(id/log_matrix_size),(id%log_matrix_size),globalsum,(globalsum+sum));
    globalsum+=sum;//adds sum to global sum
    pthread_mutex_unlock(&print_mutex);
    pthread_exit(NULL);
    return NULL;

}
void *logThread(void* threadid){
    while (queue1==NULL){
        if(queue1!=NULL)
            break;
    }
    int id;
    long tid;
    tid=(long)threadid;
    pthread_mutex_lock(&log_readqueue1_mutex);///reads queue1
    while (1){
        if(queue1->id[(queue1->front+globallog)]!=-1){
            break;
        }
    }
    id=queue1->id[(queue1->front)+globallog];
    globallog++;
    pthread_mutex_unlock(&log_readqueue1_mutex);
    int k=0;
    for (int i = 5*(id/log_matrix_size); i <5*(id/log_matrix_size)+5 ; i++) {
        int l=0;
        for (int j =5*(id%log_matrix_size); j <5*(id%log_matrix_size)+5 ; j++) {

            bigmatrix[i][j]=matrix[id][k][l];
            l++;
        }
        k++;
    }
    pthread_exit(NULL);
    return;

}
void printMatrix(int mat[5][5]){
    printf("\n");
    int i,j;
    for (int i = 0; i <5 ; i++) {
        for (int j = 0; j <5 ; j++) {
            printf("%d ",mat[i][j]);
        }
        printf("\n");
    }
}
void errorCheck(int argc, char*argv[]){

    if(argc!=8){
        perror("Wrong format");
        exit(1) ;
    }
    if(strcmp(argv[1],"-d")){
        perror("Wrong argumant");
        exit(1) ;
    }
    if(strcmp(argv[3],"-n")){
        perror("Wrong argumant");
        exit(1);
    }


}






///---------------------------------------QUEUE---------------------------------------


// function to create a queue of given capacity.
// It initializes size of queue as 0
struct Queue* createQueue(unsigned capacity)
{
    struct Queue* queue = (struct Queue*) malloc(sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;  // This is important, see the enqueue
    queue->id = (int*) malloc(queue->capacity * sizeof(int));
    return queue;
}

// Queue is full when size becomes equal to the capaci

// Function to add an item to the queue.
// It changes rear and size
void enqueue(struct Queue* queue, int item)
{

    queue->rear = (queue->rear + 1)%queue->capacity;
    queue->id[queue->rear] = item;
    queue->size = queue->size + 1;
   // printf("%d enqueued to queue\n", item);
}

int dequeue(struct Queue* queue)
{

    int item = queue->id[queue->front];
    queue->front = (queue->front + 1)%queue->capacity;
    queue->size = queue->size - 1;
    return item;
}




