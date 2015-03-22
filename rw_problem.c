#include <stdio.h>
	// for printf()
#include <pthread.h>
	// for pthread~
#include <stdlib.h>
	// for srand(), rand(), malloc(), free()
#include <time.h>
	// for clock_gettime(), sleep(), struct timespec

#define NUM_OF_WRITER	3	
#define NUM_OF_READER	10	

#define TYPE_WRITER 0
#define TYPE_READER 1

pthread_mutex_t mutex;
pthread_mutex_t wrt;

int readcount = 0;

int shared_val;

void *pthread_reader(void *thread_num);
void *pthread_writer(void *thread_num);

void init();
void sleeping();

void msg_waiting			(int thread_type, int thread_num);
void msg_num_of_readers	(int thread_num, int num_of_readers);
void msg_complete			(int thread_type, int thread_num, int written_data);

int main(void) {
	pthread_t writer[NUM_OF_WRITER], reader[NUM_OF_READER];
	void *writer_ret[NUM_OF_WRITER], *reader_ret[NUM_OF_READER];

	int i;

	init();

	// Writter thread creation	
	for(i=0;i<NUM_OF_WRITER;i++) {
		int *num = (int *)malloc(1 * sizeof(int));
		*num = i;

		pthread_create(&writer[i], NULL, pthread_writer, num);
	}
	
	// Reader thread creation
	for(i=0;i<NUM_OF_READER;i++) {
		int *num = (int *)malloc(1 * sizeof(int));
		*num = i;

		pthread_create(&reader[i], NULL, pthread_reader, num);
	}

	// Waiting for reader and writer threads to quit.
	for(i=0;i<NUM_OF_WRITER;i++)	pthread_join(writer[i], &writer_ret[i]);
	for(i=0;i<NUM_OF_READER;i++)	pthread_join(reader[i], &reader_ret[i]);

	// Mutex destruction
	pthread_mutex_destroy(&mutex);
	pthread_mutex_destroy(&wrt);

}

void *pthread_reader(void *thread_num) {
	int reader_num = *(int *)thread_num;
	free((int *)thread_num);

	sleeping();

	msg_waiting(TYPE_READER, reader_num);	

	pthread_mutex_lock(&mutex);	//wait about reader
	readcount++;	//increase reader count 

	msg_num_of_readers(reader_num, readcount);

	if(readcount ==1)	pthread_mutex_lock(&wrt);	//wait about writer
	pthread_mutex_unlock(&mutex);// unlock(signal)


	msg_complete(TYPE_READER, reader_num, shared_val);

	pthread_mutex_lock(&mutex);//wait about reader
	readcount--;//decrease readercount
	msg_num_of_readers(reader_num, readcount);
	if(readcount == 0)	pthread_mutex_unlock(&wrt);//signal about writer
	pthread_mutex_unlock(&mutex);//signal about reader
}

void *pthread_writer(void *thread_num) {
	int writer_num = *(int *)thread_num;
	free((int *)thread_num);

	sleeping();

	msg_waiting(TYPE_WRITER, writer_num);
	
	pthread_mutex_lock(&wrt);	//wait about writer

	shared_val= rand();	//data write

	msg_complete(TYPE_WRITER, writer_num, shared_val);

	pthread_mutex_unlock(&wrt);	//signal about writer
}

void init() {
	struct timespec current_time;
	
	// Mutex initializaion
	if(pthread_mutex_init(&mutex, NULL)) {
		printf("mutex init error!\n");
		exit(1);
	}
	if(pthread_mutex_init(&wrt, NULL)) {
		printf("mutex init error!\n");
		exit(1);
	}

	// Random seed initializaion
	clock_gettime(CLOCK_REALTIME, &current_time);
	srand(current_time.tv_nsec);

	// Shared value initializaion
	shared_val = 0;
}

// Being slept during random for randomly selected time (< 1s)
void sleeping() {
	sleep(1 + rand() % 2);
}

void msg_waiting(int thread_type, int thread_num) {
	if(thread_type == TYPE_READER) 
		printf("[Reader %02d] --------------------- Waiting\n", thread_num);

	else
		printf("[Writer %02d] --------------------- Waiting\n", thread_num);
}

void msg_num_of_readers(int thread_num, int num_of_readers) {
	printf("[Reader %02d] --------------------- # of readers : %d\n",
			thread_num, num_of_readers);
}

void msg_complete(int thread_type, int thread_num, int written_data) {
	if(thread_type == TYPE_READER)
		printf("[Reader %02d] Read    : %d\n", thread_num, written_data);

	else
		printf("[Writer %02d] Written : %d\n", thread_num, written_data);
}

