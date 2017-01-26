#include <stdio.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct pidWrapper {
  pid_t pid;
  int index;
}pidWrapper;



int openFile(char *opt,int flags);
bool isComValid(int *fdArr, char *args[],int numFiles, int *ioe);
void exeCom(char *args[], int* ioe, int *fdArr, int numFiles);
int numArg(char *args[]);
void printOpt(bool isVerbose, char *args[], int index);
void closeFds(int *fdArr, int numFiles);
int * fAlloc(int *fdArr, int numFiles, int *nFd, pidWrapper *pidArr);
int ctoi(char* numChar);
pidWrapper *pAlloc(pidWrapper *pidArr, int numProc, int *nPid, int *fdArr, int numFiles);
bool makePipe(int *fdArr, int numFiles);
void sigHandler(int sig);



int main(int argc, char *argv[]){

  int *fdArr = NULL;
  int numFiles = 0;
  int nFd = 0;

  pidWrapper *pidArr = NULL;
  int numProc = 0;
  int nPid = 0;
  fdArr = fAlloc(fdArr,numFiles, &nFd, pidArr);
  pidArr = pAlloc(pidArr, numProc, &nPid, fdArr, numFiles);

  struct option longopts[] = { //all options
    {"append", no_argument, NULL, '/'},
    {"cloexec", no_argument, NULL, '0'},
    {"creat", no_argument, NULL, '1',},
    {"directory", no_argument, NULL, '2'},
    {"dsync", no_argument, NULL, '3'},
    {"excl", no_argument, NULL, '4'},
    {"nofollow", no_argument, NULL, '5'},
    {"nonblock", no_argument, NULL, '6'},
    {"rsync", no_argument, NULL, '7'},
    {"sync", no_argument, NULL, '8'},
    {"trunc", no_argument, NULL, '9'},

    {"rdonly", required_argument, NULL, 'r' },
    {"wronly", required_argument, NULL, 's' },
    {"rdwr", required_argument, NULL, 't' },
    {"pipe", no_argument, NULL, 'u'},

    {"command", no_argument, NULL, 'c' },
    {"wait", no_argument, NULL, 'w'},

    {"close", required_argument, NULL, 'l'},
    {"verbose", no_argument, NULL, 'v' },
    {"abort", no_argument, NULL, 'a' },
    {"catch", required_argument, NULL, 'h'},
    {"ignore", required_argument, NULL, 'i'},
    {"default", required_argument, NULL, 'd'},
    {"pause", no_argument, NULL, 'p'},
    {0, 0, 0, 0 },
  };

  int fileOpt[11] = {O_APPEND, O_CLOEXEC, O_CREAT, 
		     O_DIRECTORY, O_DSYNC, O_EXCL,
		     O_NOFOLLOW, O_NONBLOCK, O_RSYNC,
		     O_SYNC, O_TRUNC }; //array to hold file options

  int perOpt[3] = {O_RDONLY, O_WRONLY, O_RDWR}; //array for file permission

  //getOpt related
  int opt;
  int longindex;
  int prevInd = 1;

  //option indicators
  bool isVerbose = false;
  bool doWait = false;

  //file option related
  int flags = 0; //accumulator for flags of file opening options
  int fd;


  //command option temporary variables
  int ioe[3];
  bool isValid;
  pid_t p_id;

  //wait option temporary variables
  int status;
  int eStatus;
  //parent related variables
  int retCode = 0;

  //other temporary variables
  int *setSegt = NULL;

  while ((opt = getopt_long(argc, argv, "/0123456789r:s:t:ucwl:vah:i:d:p", longopts, &longindex)) != -1){

    printOpt(isVerbose, argv, prevInd);
    switch(opt) {

      //FILE FLAG OPTIONS
    case '/':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      flags = flags | (fileOpt[ opt - 47]); //add the corresponding file option to flags
      break;


    //FILE-OPENING OPTIONS
    case 'r':
    case 's':
    case 't':
      //check that array is not full
      if(numFiles + 1 >= nFd){
	fdArr = fAlloc(fdArr, numFiles, &nFd, pidArr);
      }
      fd = openFile(argv[optind - 2],flags | perOpt[opt - 114]);
      //if failed to open, return code should be set to 1
      if(fd == -1){
	retCode = 1;
      }
      //Number of files should be increased
      fdArr[numFiles] = fd;
      numFiles++;
      flags = 0; //reset the flag
      break;

    case 'u': //pipe
      if(numFiles + 2 >= nFd){
	fdArr = fAlloc(fdArr, numFiles, &nFd, pidArr);
      }
      if(!makePipe(fdArr, numFiles)){
	closeFds(fdArr, numFiles);
	free(pidArr);
	_exit(1);
      }
      else{
	numFiles += 2;
      }


      break;

    //SUBCOMMAND OPTIONS
    case 'c':
      isValid = isComValid(fdArr, argv + optind, numFiles, ioe);

      //if command is not valid, set return code to 1 and ignore the command
      if (!isValid){ 
	fprintf(stderr, "Skipping this command. \n");
	retCode = 1;
	break;
      }

      //command is valid, so fork
      else{

	if (numProc + 1 >= nPid){
	  pidArr = pAlloc(pidArr, numProc, &nPid, fdArr, numFiles);
	}

      p_id = fork();
      if(p_id == 0){ //child

	exeCom(argv + optind, ioe, fdArr, numFiles);
	//if program is here, that means execvp failed
	closeFds(fdArr, numFiles);
	free(pidArr);
	return -1;
      }
      else{ //parent
	pidArr[numProc].pid = p_id;
	pidArr[numProc].index = optind + 3;
	optind += numArg(argv + optind); 
	numProc++;
      }
      

      }
      break;
    case 'w': //wait
      doWait = true;
      break;

    //MISCELLANEOUS OPTIONS
    case 'l': //close
      fd = ctoi(optarg);
      close(fdArr[fd]);
      fdArr[fd] = -1;
      break;

    case 'v':
      isVerbose = true;
      break;
    case 'a': //abort
      closeFds(fdArr, numFiles);
      free(pidArr);
      *setSegt = 1;
      break;

    case 'h': //catch
      signal(ctoi(optarg), sigHandler);
      break;
    case 'i': //ignore
      signal(ctoi(optarg), SIG_IGN);
      break;
    case 'd': //default
      signal(ctoi(optarg), SIG_DFL);
      break;
    case 'p': //pause
      pause();
      break;
    default:
      printf("not an option\n");

    }

    prevInd = optind;
  } 


  //cleanup
  closeFds(fdArr, numFiles);

  if(doWait == true){

    int i,j;
    for(i = 0; i < numProc; i++){
      p_id = wait(&status);
      if(!WIFEXITED(status)){
	fprintf(stderr, "Child Process %d did not exit correctly. \n", p_id);
	continue;
      }
      eStatus = WEXITSTATUS(status);
      for(j = 0; j < numProc; j++){
	if(p_id == pidArr[j].pid){
	  printf("%d ", eStatus);
	  printOpt(true, argv, pidArr[j].index);
	  break;
	}
      }
      if(eStatus > retCode){
	retCode = eStatus;
      }
    }
  }

  free (pidArr);

  return retCode;


}

int openFile(char *opt, int flags){ //opens the file (if not invalid)

  int oStatus;

  mode_t wrAll = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
  if (optarg[0] == '-' && optarg[1] == '-'){
    fprintf(stderr, "No argument passed for %s. Skipping this option.\n", opt);
    optind--;
    return -1;
  }
  oStatus = open(optarg, flags, wrAll);
  if (oStatus == -1){
    perror(optarg);
    fprintf(stderr, "Failed to open the file. Skipping this option.\n");
    return -1;
  }


  return oStatus;

}


bool isComValid(int *fdArr, char *args[], int numFiles, int *ioe){

  //check argument exists
  int i;
  for(i = 0;i < 3;i++){
    if (args[i] == NULL){
      fprintf(stderr, "Invalid number of arguments. \n");
      return false;
    }

    //check that each argument is a number
    int fd;
    fd = ctoi(args[i]);
    if(fd < 0){
      return false;
    }
    //check that resulting fd is within range
    if (fd >= numFiles || fdArr[i] == -1){ 
      fprintf(stderr, "Invalid file descriptor. Either the file descriptor is invalid (they must be between 0 and %d.), or is already closed.\n", numFiles - 1);
      return false;
    }
    //assign fd to appropriate place
    ioe[i] = fd;
  }

  //check that command exists
  if (args[3] == NULL){ //check that command exists
    fprintf(stderr, "Must have name of the command. \n");
    return false;
  }
  

  return true;

}

void exeCom(char *args[], int *ioe, int *fdArr, int numFiles){

  int l;
  for(l = 4; args[l] != NULL; l++){ //set one after last element to null
    if (args[l][0] == '-' && args[l][1] == '-'){
      args[l] = NULL;
      break;
    }
    else{
      continue;
    }
  }

  int i;
  for (i = 0; i < 3; i++){
    close(i);
    dup(fdArr[ioe[i]]);
    close(fdArr[ioe[i]]);
    fdArr[ioe[i]] = -1;
  }

  closeFds(fdArr, numFiles);

  execvp(args[3], (args + 3));

}



int numArg(char *args[]){

  int i;
  int nArgs = 0;
  for (i = 0; args[i] != NULL; i++){
    if (args[i][0] == '-' && args[i][1] == '-'){
      return nArgs;
    }

    else{
      nArgs++;
    }

  }
  return nArgs;
}

void printOpt(bool isVerbose, char *args[], int index){

  if (isVerbose == 0){
    return;
  }
  int numPrint = numArg(args + index + 1);
  numPrint++; //
  int i;
  for (i = 0; i < numPrint; i++){

    if(i + 1 == numPrint){
      printf("%s\n", args[index + i]);
    }
    else{
      printf("%s ", args[index + i]);
    }
  }

}

void closeFds(int *fdArr, int numFiles){

  int i;
  for (i = 0; i < numFiles; i++){
    if(fdArr[i] != -1){
      if(close(fdArr[i]) == -1){
	fprintf(stderr, "Error closing file descriptor %d", i);
	perror(NULL);
	fprintf(stderr, "\n");
      }
    }
  }

  free(fdArr);


}
int *fAlloc(int *fdArr, int numFiles, int *nFd, pidWrapper *pidArr){

  if(fdArr == NULL){ //malloc
    fdArr = (int*)malloc((*nFd + 100) * sizeof(int));
    if(fdArr == NULL){
      fprintf(stderr, "Failed to allocate memory for the file descriptors.\n");
      closeFds(fdArr, numFiles);
      free(pidArr);
      _exit(1);
    }
    else{
      *nFd += 100;
    }
  }


    else{ //realloc
      int *temp;
      temp = (int*)realloc(fdArr,(*nFd + 100) * sizeof(int));
      if(temp == NULL){
	fprintf(stderr, "Failed to reallocate memory for the file descriptors.\n");
	closeFds(fdArr, numFiles);
	free(pidArr);
	_exit(1);
      }
      else{
	*nFd += 100;
	fdArr = temp;
      }



    }

  return fdArr;
}

int ctoi(char *numChar){

  int digits = 0;
  int i;
  //check number of digits, and make sure each character is a number
  for(i = 0; numChar[i] != '\0'; i++){
      if ((int)numChar[i] - 48 < 0 || (int)numChar[i] - 48 > 9)
	{
	  fprintf(stderr, "File descriptor must be a number.\n");
	  return -1;
	}
      digits++;
    }
    
    int fd = 0;
    int mult = 1;
    int k;
    for (k = digits - 1; k >=0; k--){ //convert characters to digits
      fd += ((int) numChar[k] - 48) * mult;
      mult *= 10;
    }

    return fd;

}

pidWrapper *pAlloc(pidWrapper *pidArr, int numProc, int *nPid, int *fdArr, int numFiles){

  if(pidArr == NULL){ //malloc
    pidArr = (pidWrapper*)malloc((*nPid + 100) * sizeof(pidWrapper));
    if(pidArr == NULL){
      fprintf(stderr, "Failed to allocate memory for the process IDs.\n");
      closeFds(fdArr, numFiles);
      free(pidArr);
      _exit(1);
    }
    else{
      *nPid += 100;
    }
  }


  else{ //realloc
    pidWrapper *temp;
    temp = (pidWrapper*)realloc(pidArr,(*nPid + 100) * sizeof(pidWrapper));
    if(temp == NULL){
      fprintf(stderr, "Failed to reallocate memory for the proces IDs.\n");
      closeFds(fdArr, numFiles);
      free(pidArr);
      _exit(1);
    }
    else{
      *nPid += 100;
      pidArr = temp;
    }
  }
    
  return pidArr;
    
}


bool makePipe(int *fdArr, int numFiles){

  int pipeFd[2];
  if(pipe(pipeFd) == -1){
    perror("pipe");
    fdArr[numFiles] = -1;
    fdArr[numFiles + 1] = -1;
    return false;
  }

  else{
    fdArr[numFiles] = pipeFd[0];
    fdArr[numFiles + 1] = pipeFd[1];
  }

  return true;


}

void sigHandler(int sig){

  fprintf(stderr, "Caught signal %d. Exitting.\n", sig);
  exit(sig);

}
