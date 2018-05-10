#include <pthread.h> //for pthread functions
#include <semaphore.h>// for semaphore
#include <iostream> // for cout
#include <unistd.h> // for sleep function
#include <sched.h> //for sched_param
#include <ctime> //for clock()
#include <cstring>//for strcmp()
#include <chrono>// for time_point, system_clock
#include <thread>//for sleep_until()
#define unit 5 //in milliseconds
#define periods 10 //# of times a frame is run
#define numThreads 5 //scheduler + 4 workers
#define core 3 //core # for processor affinity

using namespace std;

sem_t sema[numThreads];//1 for scheduler and 4 for threads
pthread_mutex_t locks[4];//locks for when incrementing counters and decrementing counters

int runs[] = {0,0,0,0};//# of tasks started
int miss[] = {0,0,0,0};//# of tasks missing a deadline
int scheduled[] = {0,0,0,0};//# of tasks scheduled to run
int finished[]= {0,0,0,0};//# of tasks successfully finished before simulation end
int times[] = {1,2,4,16};//# of time a thread has per period
int times_m[] = {1,2,4,16};//# of doWork() each thread will run per period
int num[] = {0,1,2,3};//just a placeholder to pass thread number
chrono::system_clock::time_point endTime;//the end time of the simulation

void doWork(int n);//will doWork() an n amount of times
void *scheduler(void* a);//scheduler with null argument
void *threadT(void* a);//a thread taking thread number as an argument

int main(int argc, char** argw){

  cout<<"Initializing variables..."<<endl;

  //setting a cpu mask to use for affinity
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core,&cpuset);

  struct sched_param params;//used for setting priority

  pthread_t* threads=new pthread_t[numThreads];//holds PID for all threads

  //setting semaphores and mutexes
  for(int x=0; x<numThreads; ++x)
    sem_init(&sema[x], 0, 0);
  for(int x=0; x<4; ++x)
    pthread_mutex_init(&locks[x],NULL);

  //using commandline argument for case selection
  if(strcmp(argw[1],"1")==0){
    cout<<"Normal case:"<<endl;
  }
  else if(strcmp(argw[1],"2")==0){
    cout<<"T2 overrun case:"<<endl;
    times_m[1]=50000;
  }
  else if(strcmp(argw[1],"3")==0){
    cout<<"T3 overrun case:"<<endl;
    times_m[2]=50000;
  }
  else{
    cout<<"Normal case:"<<endl;
  }

  cout<<"\tEach period has "<<unit<<" milliseconds."<<endl;
  for(int x=0; x<4; ++x){
    cout<<"\tThread "<<x+1<<" has a period of "<<times[x]<<" and runs doWork() "<<times_m[x]<<" times."<<endl;
  }

  //starting with max priority and multi-feedback FIFO
  params.sched_priority = sched_get_priority_max(SCHED_FIFO);
  cout<<"Scheduler created. ";
  pthread_create(&threads[0],NULL,scheduler,NULL);
  cout<<"Affinity set to core "<<core<<". ";
  pthread_setaffinity_np(threads[0], sizeof(cpu_set_t), &cpuset);
  cout<<"Priority set to "<<params.sched_priority<<"."<<endl;
  pthread_setschedparam(threads[0], SCHED_FIFO, &params);

  for(int x=1; x<numThreads; ++x){
    params.sched_priority--;//decrementing priority for each subsequent thread
    cout<<"Worker "<<x<<"  created. ";
    pthread_create(&threads[x],NULL,threadT,(void *) &num[x-1]);
    cout<<"Affinity set to core "<<core<<". ";
    pthread_setaffinity_np(threads[x], sizeof(cpu_set_t), &cpuset);
    cout<<"Priority set to "<<params.sched_priority<<"."<<endl;
    pthread_setschedparam(threads[x], SCHED_FIFO, &params);
  }
  cout<<"Simulation started."<<endl;
  sem_post(&sema[0]);//release the scheduler thread

  //cleaning up afterward
  for(int x=numThreads; x>0; --x){
    pthread_join(threads[x-1],NULL);
  }
  cout<<"Ending program..."<<endl;
  for(int x=0; x<numThreads; ++x)
    sem_destroy(&sema[x]);
  for(int x=0; x<4; ++x)
    pthread_mutex_destroy(&locks[x]);
  delete threads;
  return 0;
}
void doWork(int n){
  for(int x=0; x<n; ++x){//running n amount of times
    double matrix[10][10];
    for(int y=0; y<10; ++y){
      for(int x=0; x<10; ++x){
        matrix[x][y]=1.0;
      }
    }
    for(int y=0; y<5; ++y){
      for(int x=0; x<10; ++x){
        matrix[x][y]=matrix[x][y]*matrix[(x+1)%10][(y+5)%10];
        matrix[x][(y+5)%10]=matrix[x][(y+5)%10]*matrix[(x+1)%10][y];
      }
    }
  }
}
void *scheduler(void* a){
  const chrono::milliseconds intervalPeriodMilli{unit};//milliseconds per period

  sem_wait(&sema[0]);
  chrono::system_clock::time_point currentStartTime=chrono::system_clock::now();//marking start point
  endTime=currentStartTime+intervalPeriodMilli*16*periods;//setting end point
  for(int count=0; count<=16*periods; ++count){//run 16 * # of frames
    currentStartTime+=intervalPeriodMilli;//set next time point
    pthread_mutex_lock(&locks[0]);//check for deadline misses and schedule tasks
    if(scheduled[0]>0)
      miss[0]++;
    scheduled[0]++;
    pthread_mutex_unlock(&locks[0]);
    sem_post(&sema[1]);
    if(count%times[1]==0){//same thing for for thread2 for each 2 periods
      pthread_mutex_lock(&locks[1]);
      if(scheduled[1]>0)
        miss[1]++;
      scheduled[1]++;
      pthread_mutex_unlock(&locks[1]);
      sem_post(&sema[2]);
    }
    if(count%times[2]==0){//for each 4 periods
      pthread_mutex_lock(&locks[2]);
      if(scheduled[2]>0)
        miss[2]++;
      scheduled[2]++;
      pthread_mutex_unlock(&locks[2]);
      sem_post(&sema[3]);
    }
    if(count%times[3]==0){//for each 16 periods
      pthread_mutex_lock(&locks[3]);
      if(scheduled[3]>0)
        miss[3]++;
      scheduled[3]++;
      pthread_mutex_unlock(&locks[3]);
      sem_post(&sema[4]);
    }
    this_thread::sleep_until(currentStartTime);//sleep until next time point
  }
  //print out results
  cout<<"Simulation finished."<<endl;
  cout<<"########################################################"<<endl;
  cout<<"\t\t\tT1\tT2\tT3\tT4"<<endl;
  cout<<"########################################################"<<endl;
  cout<<"Tasks Started:  \t"<<runs[0]<<"\t"<<runs[1]<<"\t"<<runs[2]<<"\t"<<runs[3]<<endl;
  cout<<"Tasks Finished: \t"<<finished[0]<<"\t"<<finished[1]<<"\t"<<finished[2]<<"\t"<<finished[3]<<endl;
  cout<<"Deadlines Missed:\t"<<miss[0]<<"\t"<<miss[1]<<"\t"<<miss[2]<<"\t"<<miss[3]<<endl;
  cout<<"########################################################"<<endl;
  pthread_exit(NULL);
}
void *threadT(void* a){
  int *t=(int *)a;//get thread #
  for(int x=0; x<16*periods/times[*t]; ++x){//setting max time to run, may end early if simulation ends
    sem_wait(&sema[(*t)+1]);
    runs[*t]++;
    doWork(times_m[*t]);
    if(endTime<chrono::system_clock::now())//check if simulation has ended
      break;
    finished[*t]++;
    pthread_mutex_lock(&locks[*t]);
    scheduled[*t]--;
    pthread_mutex_unlock(&locks[*t]);
  }
  pthread_exit(NULL);
}
