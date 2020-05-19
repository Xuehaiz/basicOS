
/* -----------------------------------Project Description--------------------------------
 *Author: Linyi Fu
 *Description: This is project first part and use circle ring buffer to alloc the data enqueue and dequeue.
 *Date: Nov/28.2019
 */


/*-----------------------------------Head File ----------------------------------------*/
#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>

#define URLSIZE 32
#define CAPSIZE 32
#define MAXQUEUE 3
#define NUMPROXIES 3
/*-----------------------------------Main Function --------------------------------*/

pthread_mutex_t mutex[MAXQUEUE]= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_cond_sub  = PTHREAD_COND_INITIALIZER;
pthread_cond_t  condition_cond_pub  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t condition_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_clean= PTHREAD_MUTEX_INITIALIZER;

int delta_time;

int counter[MAXQUEUE];

struct topicEntry{
    int entryNum;
    struct timeval timeStamp;
    int pubID;
    char photoURL[URLSIZE];
    char photoCaption[CAPSIZE];
};


struct CRB{
    char *name[32];
    int topicID;
    struct topicEntry *entry;
    int head;
    int tail;
    int length;
};

struct combineArg{
    int order;
    char *filename[64];
};

struct thread_pool{
    pthread_t pool;
    int flag;
    int thread_id;
    char *thread_read_file[64];
};
struct CRB crbArray[MAXQUEUE];

int enqueue(struct CRB *crb, struct topicEntry *entry){
    if((crb -> head) == (crb -> tail )){
        printf(" queue is FULL\n");
        return 0;
    }
    else{
        if (crb -> tail == -1){
            (crb -> tail) ++;
        }
        gettimeofday(&(entry -> timeStamp), NULL);
//        entry -> entryNum = head +1;
        crb -> entry[crb -> head].entryNum = entry->entryNum;
        crb -> entry[crb -> head].timeStamp.tv_sec = entry->timeStamp.tv_sec;
        crb -> entry[crb -> head].pubID = entry->pubID;
        strcpy(crb -> entry[crb -> head].photoURL , entry->photoURL);
        strcpy(crb -> entry[crb -> head].photoCaption , entry->photoCaption);


        (crb -> head )++;
        
        if((crb -> head) == (crb -> length)) {
            crb -> head = 0;
        }
        return 1;
    }
}

int getEntry(int lastEntry, struct CRB *crb, struct topicEntry *empty){
    int k,a;
    if((crb -> tail == -1)&&(crb -> head == 0)){
        printf("The Queue is empty\n");
        return 0;
    }
    else{
        int check = 1;
        for (k = 0; k< (crb -> length); k ++){
            if (lastEntry + 1  == crb->entry[k].entryNum){
                empty ->entryNum = crb->entry[k].entryNum;
                empty ->timeStamp = crb->entry[k].timeStamp;
                empty ->pubID = crb->entry[k].pubID;
                strcpy(empty ->photoURL, crb->entry[k].photoURL);
                strcpy(empty ->photoCaption, crb->entry[k].photoCaption);
                return 1;
            }
            if ((lastEntry + 1  > crb->entry[k].entryNum)&&(crb->entry[k].entryNum != -1)){
                check  = check && 1;
            }
            else {check = check&&0;}
        }
        if(check){
            printf("All enries in the queue are less than input.\n");
            return 0;
        }
        for ( a = 0; a< (crb -> length); a ++){
            if (lastEntry + 1  < crb->entry[a].entryNum){
                empty ->entryNum = crb->entry[a].entryNum;
                empty ->timeStamp = crb->entry[a].timeStamp;
                empty ->pubID = crb->entry[a].pubID;
                strcpy(empty ->photoURL, crb->entry[a].photoURL);
                strcpy(empty ->photoCaption, crb->entry[a].photoCaption);
                return crb -> entry[a].entryNum;
            }
        }
        
    }
    return 0;
}

int dequeue (struct timeval delta, struct CRB *crb ){
    int k,t;
    if ((crb -> head == 0)&&(crb -> tail == -1)){
        printf("The Queue is empty\n");
        return 0;
    }
    else{
        for (k = 0; k< (crb -> length);){
//            printf("%ld\n", delta.tv_sec);
//            printf("%ld\n", crb->entry[k].timeStamp.tv_sec);


            if( delta.tv_sec - crb->entry[k].timeStamp.tv_sec > delta_time){
                for ( t = k; t <(crb -> length) -1 ; t ++){
                    crb -> entry[t] = (crb -> entry[t+1]);
                }
                crb -> entry[crb->length -1].entryNum = -1;
                crb -> entry[crb->length -1].timeStamp.tv_sec = delta.tv_sec;
                crb -> head --;
                if (crb -> head == -1){crb -> head = (crb -> length -1);}
                if (crb -> entry[0].entryNum == -1){
                    crb -> head = 0;
                    crb -> tail = -1;
                }
            }
            else{
                k++;
            }
        }
        return 1;
    }
}
void initQueue(struct CRB *crb){

    strcpy(crb->name,"");
    crb->topicID = -1;
    crb->head = 0;
    crb->tail = -1;

    crb->length = -1;
//    crb->entry = malloc (crb->length * sizeof(struct topicEntry));
//    for (int k = 0; k < maxentries;k++){
//        crb->entry[k].entryNum = -1;
//        gettimeofday(&(crb->entry[k].timeStamp), NULL);
//        crb->entry[k].pubID = -1;
//        strcpy(crb->entry[k].photoURL, "");
//        strcpy(crb->entry[k].photoCaption, "");
//    }
}
void initEntry(struct topicEntry *entry){
    entry->entryNum = -1;
    gettimeofday(&(entry->timeStamp), NULL);
    entry->pubID = -1;
    strcpy(entry->photoCaption, "");
    strcpy(entry->photoURL, "");
}

void initComb(struct combineArg *combine){
    strcpy(combine->filename, "");
    combine -> order = -1;

}

void initPool(struct thread_pool *a){
    a->flag = 0;
    a->thread_id= -1;
    strcpy(a->thread_read_file , "");
}

char* replace_char(char* str, char find, char replace){//this method search from the internet.
    char *current_pos = strchr(str,find);
    while (current_pos){
        *current_pos = replace;
        current_pos = strchr(current_pos,find);
    }
    return str;
}


void *publisher(void *args){
    
    pthread_mutex_lock(&condition_mutex);
    pthread_cond_wait(&condition_cond_pub, &condition_mutex);
    pthread_mutex_unlock(&condition_mutex);
    
    
    
    sleep(1);
    char *file_read;
    struct combineArg *combine;
    struct topicEntry entry;
    initComb(&combine);
    combine = ((struct combineArg *) args);
    initEntry(&entry);
    int order;
    order = combine->order;
    file_read = combine->filename;
    pthread_mutex_lock (&mutex[order]);

    char *pubBuffer;
    char *pubchart[4];
    size_t input_size,buffer_size = 32;
    FILE *pubPtr = fopen(file_read, "r");
    char *pub_middle;
    pubBuffer = (char*)malloc(buffer_size * sizeof(char));
    int index,j,a;

    while ((input_size = getline(&pubBuffer,&buffer_size,pubPtr)) != -1){
        pubBuffer = replace_char(pubBuffer,'\n','\0');
        
        for( a = 0; a < 4; a++){
            pubchart[a] = "";
        }
        printf("Proxy thread <P%d> - type: Publisher - Executed command: %s\n", order+1, pubBuffer);
        index = 0;
        while((pub_middle = strtok_r(pubBuffer, " ", &pubBuffer))){
            pubchart[index] = pub_middle;
            index+=1;
        }
        
        if (strcmp(pubchart[0],"put") == 0){
            int i ;
            for(int i = 0; i < MAXQUEUE; i++){
                if (crbArray[i].topicID == atoi(pubchart[1])){
                    entry.entryNum = order;
                    entry.pubID = order;
                    gettimeofday(&entry.timeStamp, NULL);
                    strcpy(entry.photoURL, pubchart[2]);
                    strcpy(entry.photoCaption, pubchart[3]);
                    enqueue(&crbArray[i],&entry);
                }
            }
        }
        if (strcmp(pubchart[0],"sleep") == 0){
            sleep(atoi(pubchart[1])/1000);
        }
        if (strcmp(pubchart[0],"stop") == 0){
            break;
        }
    }

    fclose(pubPtr);
//    free(pubBuffer);

    pthread_mutex_unlock(&mutex[order]);
    return NULL;
};

void *subscriber(void *args){
    
    pthread_mutex_lock(&condition_mutex);
    pthread_cond_wait(&condition_cond_sub, &condition_mutex);
    pthread_mutex_unlock(&condition_mutex);
    sleep(1);
    
    char *sub_file_read;
    struct combineArg *combine;
    struct topicEntry entry;
    int lastEntry;
    int order;
    initComb(&combine);

    combine = ((struct combineArg *) args);
    
    initEntry(&entry);
    order = combine->order;
    sub_file_read = combine -> filename;
    char *subBuffer;
    char *subchart[2];
    
    size_t input_size, buffer_size = 32;
    FILE *subPtr = fopen(sub_file_read, "r");
    char *sub_middle;
//    printf("aaa\n");
    subBuffer = (char*)malloc(buffer_size * sizeof(char));
    int index,j ,a;
    while((input_size = getline(&subBuffer, &buffer_size, subPtr))!= -1){
        subBuffer = replace_char(subBuffer,'\n','\0');

        for( a = 0; a < 2; a++){
            subchart[a] = "";
        }
        printf("Proxy thread <S%d> - type: Subscriber - Executed command: %s\n", order+1, subBuffer);
        index = 0;
        while((sub_middle = strtok_r(subBuffer, " ", &subBuffer))){
            subchart[index] = sub_middle;
            index+=1;
        }
        if(strcmp(subchart[0], "get") ==0){
            pthread_mutex_lock (&mutex[order]);
            getEntry(order -1,&crbArray[order],&entry);
            pthread_mutex_unlock(&mutex[order]);

        }
        if (strcmp(subchart[0],"sleep") == 0){
            usleep(atoi(subchart[1]));
        }
        if (strcmp(subchart[0],"stop") == 0){
            break;
        }
        
    }
    printf("get answer: %s\n",entry.photoURL);
    
    
    return NULL;
};

void *cleanup(void *args){
    struct combineArg *combine;

    struct timeval delta;
    gettimeofday(&delta,NULL);
    combine =((struct combineArg *) args);
    int order;
    order = combine -> order;
    pthread_mutex_lock (&mutex[order]);


    dequeue(delta,&crbArray[order]);
    pthread_mutex_unlock(&mutex[order]);

    return NULL;

};


int main (int argc, char *argv[]){
    
    setbuf(stdout,NULL);

    struct thread_pool subPool[NUMPROXIES], pubPool[NUMPROXIES];
    struct combineArg pub_combine[NUMPROXIES];
    struct combineArg sub_combine[NUMPROXIES];
    struct combineArg clean[MAXQUEUE];

    int num;
    for (num = 0; num< NUMPROXIES;num ++){
        initPool(&subPool[num]);
        initPool(&pubPool[num]);
    }


    struct timeval delta;
    int pubPool_index = -1;
    int subPool_index = -1;
    int i,j,a,k;


    
    for( i = 0; i< MAXQUEUE; i++){
        initQueue(&crbArray[i]);
    }
    int QueueIndex = -1;
    char *cBuffer, *command_with_arg;
    size_t bufferSize = 32, inputSize;
    int sub_index, pub_index;
    char *command_array[5];
    char *middle;
    
    pthread_t cleanThread[MAXQUEUE];
    FILE *fp = fopen(argv[1],"r");
    
    cBuffer = (char *)malloc(bufferSize * sizeof(char));
    int index;
    while (((inputSize = getline(&cBuffer,&bufferSize,fp)) != -1)) {

        cBuffer = replace_char(cBuffer,'\n','\0');
        index = 0;

        while((middle = strtok_r(cBuffer, " ", &cBuffer))){
//            printf("%s\n", middle);
            command_array[index] =middle;
            index+=1;
            
        }

        if (strcmp(command_array[0],"create") == 0){

            if (strcmp(command_array[4], "") == 0){
                printf("Not enough arguments\n");
                break;
            }
            else{
                if (QueueIndex < MAXQUEUE ){

                    QueueIndex ++;
                    crbArray[QueueIndex].length = atoi(command_array[4]);
                    crbArray[QueueIndex].topicID = atoi(command_array[2]);
                    strcpy(crbArray[QueueIndex].name, command_array[3]);
                    crbArray[QueueIndex].entry = malloc (crbArray[QueueIndex].length * sizeof(struct topicEntry));
                    for ( k = 0; k < crbArray[QueueIndex].length;k++){
                        crbArray[QueueIndex].entry[k].entryNum = -1;
                        gettimeofday(&(crbArray[QueueIndex].entry[k].timeStamp), NULL);
                        crbArray[QueueIndex].entry[k].pubID = -1;
                        strcpy(crbArray[QueueIndex].entry[k].photoURL, "");
                        strcpy(crbArray[QueueIndex].entry[k].photoCaption, "");
                    }
                }
                else{
                    printf("Create Queue reach Max Queue number which is: %d\n", MAXQUEUE);
                    break;
                }
            }
            
//            create_topic(command_array[2],command_array[3],command_array[4]);
            
        }
        
        else if (strcmp(command_array[0],"query") == 0){
            
            if(strcmp (command_array[1], "topics")==0){
                int aaa;
                for(aaa = 0;aaa < QueueIndex;aaa ++){
                    printf("Topic id: %d , Length: %d\n",crbArray[aaa].topicID,crbArray[aaa].length);
                }
            }
            else if(strcmp(command_array[1],"publishers")==0){
                printf("current publishers: %d    It's command file: %s\n", pubPool_index, pubPool[pubPool_index].thread_read_file);
            }
            else if(strcmp(command_array[1],"subscribers")==0){
                printf("current subscribers: %d    It's command file: %s\n", subPool_index, subPool[subPool_index].thread_read_file);
            }
        }
        else if (strcmp(command_array[0],"add") == 0){
            if(strcmp(command_array[1],"subscriber") == 0){
                subPool_index ++;
                subPool[subPool_index].thread_id = subPool_index;
                subPool[subPool_index].flag= 1;
                strcpy(subPool[subPool_index].thread_read_file , command_array[2]);

                initComb(&sub_combine[subPool_index]);
                strcpy(sub_combine[subPool_index].filename,command_array[2]);
                sub_combine[subPool_index].order = subPool_index;
            pthread_create(&subPool[subPool_index].pool,NULL,subscriber,&sub_combine[subPool_index]);
                
            }
            else if(strcmp(command_array[1],"publisher")==0){
                pubPool_index ++;
                pubPool[pubPool_index].thread_id = pubPool_index;
                pubPool[pubPool_index].flag= 1;
                strcpy(pubPool[pubPool_index].thread_read_file , command_array[2]);
                
                initComb(&pub_combine[pubPool_index]);
                strcpy(pub_combine[pubPool_index].filename, command_array[2]);
                pub_combine[pubPool_index].order = pubPool_index;
                
                pthread_create(&pubPool[pubPool_index].pool,NULL,publisher, &pub_combine[pubPool_index]);
            }
            
        }
        else if (strcmp(command_array[0],"delta") == 0){
            delta_time = atoi(command_array[1]);
        }
        
        else if ((strcmp(command_array[0],"start") == 0)){
            int start_index;
            struct combineArg clean[QueueIndex+1];

            for (start_index = 0; start_index < QueueIndex+1;start_index ++){
                initComb(&clean[start_index]);
                clean[start_index].order = start_index;
            }
            for (start_index = 0; start_index < QueueIndex+1;start_index ++){
//                printf("%d , %d\n", clean[start_index].crb->topicID,clean[start_index].order);
                pthread_create(&cleanThread[start_index], NULL,cleanup,&clean[start_index]);
            }


            for(start_index = 0 ; start_index< pubPool_index+1; start_index++){

                sleep(1);
                pthread_cond_signal(&condition_cond_pub);
            }
            sleep(3);

            for(start_index = 0 ; start_index< subPool_index+1; start_index++){
                sleep(1);
                pthread_cond_signal(&condition_cond_sub);
            }
        }
        else{
            printf("Error, Unsupport argumnets\n");
        }
        sleep(1);

    }
    int qwe;
    for (qwe = 0; qwe < QueueIndex  ; qwe ++){
       pthread_join(cleanThread[qwe] ,NULL);
    }
    for (qwe = 0;qwe < pubPool_index+1; qwe++){
        pthread_join(pubPool[qwe].pool, NULL);
    }
    for (qwe = 0;qwe < pubPool_index+1; qwe++){
        pthread_join(subPool[qwe].pool, NULL);
    }
    
    printf("delta is : %d\n", delta_time);
//    printf("%s %s\n\n",crbArray[0].entry[0].photoURL,crbArray[0].entry[1].photoURL);
//    fclose(fp);
//    free(cBuffer);
//    int asd;
//    for( asd = 0; asd< QueueIndex; asd++ ){
//        free(crbArray[asd].entry);
//    }
    return 1;
    
}

