/*
 * $ hexdump -e file
 * 
 * I added an -e flag which prints the number of the last block
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>
#include <ctype.h>

/*
 * $ hexdump -s10 -l3 file
 * hexdump shows hex values of tiny file system blocks
 * hexdump shows hex valuse of any file in 512 byte blocks
 * -s10 is the start block, -s must include a number
 * -l3 is the number of blocks, -l must include a number
 * file is the tiny file system
 */

#define BSIZE 512

#define uint unsigned int

int blocks = 10, start_block = 0;
char filename[100];

void panic(char *m){
  printf("%s\n",m);
  exit(-1);
}

/*
 * $ hexdump -e file
 * hexdump will print number of the last block with data on it
 */
void end_block(){

}

int get_opts(int count, char *args[]) {
  //printf("DEBUG - get_opts - count == %d\n",count);
  int opt, len, i, good = 1;
  while (good && (opt = getopt(count, args, "s:l:e:")) != -1) {
    int len, i;
    //printf("DEBUG - get_opts - opt is %c\n", opt);
    switch (opt) {
      case 's':
        //printf("DEBUG - case s\n");
        len = strlen(optarg);
        for (i=0;i<len; i++)
          if (!isdigit(optarg[i])) {
            fprintf(stderr, "-s value must be a number.\n");
            good = 0;
            break;
          }
        if (good)
          start_block = atoi(optarg);
        break;
      case 'l':
        //printf("DEBUG - case l\n");
        len = strlen(optarg);
        for (i=0;i<len; i++)
          if (!isdigit(optarg[i])) {
            fprintf(stderr, "-l value must be a number.\n");
            good = 0;
            break;
          }
        if (good)
            blocks = atoi(optarg);
        break;
      case 'e': //my addition
        //printf("DEBUG - case e\n");
        len = strlen(optarg);
        good = 1; //sets good value to 1
        start_block = -1; //this allows us to compare in main()
        //printf("DEBUG - set start_block to -1\n");
        break;
      case ':':
        //printf("DEBUG - case :\n");
        fprintf(stderr, "option missing value\n");
        break;
      case '?':
        //printf("DEBUG - case ?\n");
        if (optopt == 'e'){
          fprintf(stderr, "DEBUG - case? - using e flag\n");
          break;}
        else if (optopt == 'l')
          fprintf(stderr, "Option -%c requires an argument.\n", optopt);
        else if (optopt == 's')
          fprintf(stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint(optopt))
          fprintf(stderr, "Unknown option `-%c'.\n", optopt);
        else
         fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
        good = 0;
        break;
    }
  }
  //printf("DEBUG - comparing good (%i) and optind\n",good);
  if(start_block != -1 && good && optind > count-1) {
    //printf("DEBUG - comparison bad\n");
    fprintf(stderr, "Invalid number of arguments. %d\n", optind);
    good = 0;
    return good;  //FIXME
  }
  //printf("DEBUG - in elseif now good (%i)\n", good);
  else if (good == 1) {
    //printf("DEBUG - comparison good, copying now\n");
    //printf("DEBUG - filename(%s); args(%s)\n", filename, args[optind]);
    if(start_block != -1){
      strcpy(filename, args[optind]);
    }else{
      strcpy(filename, args[2]);
    }
  }
  //printf("DEBUG - good is (%d), start is (%d) \n",good,start_block);
  return good;

}

int fs;

int bread(uint block, unsigned char *buf) {
  int off = lseek(fs, block*BSIZE, SEEK_SET);
  if (off < 0)
    panic("bread lseek fail");
  //printf("off : %d\n", off);
  memset(buf, 0, BSIZE);
  int sz = read(fs, buf, BSIZE);
  if (sz < 0)
    panic("bread read fail");
  //printf("block : %d | sz : %d\n", block, sz);
  return sz;
}

int openfs(char *name) {
  fs = open(name, O_RDWR, S_IRUSR | S_IWUSR);
  if (fs < 0)
    panic("openfs open fail");
  return 0;
}

int closefs() {
  close(fs);
  return 0;
}

#define LINE 32

int main(int argc, char *argv[]) {

  //printf("DEBUG - main - before getting status\n");
  int status = get_opts(argc, argv);

  if (!status){
    printf("DEBUG - main - status is bad\n");
    exit(-1);
  }
  //printf("DEBUG - main - status good\n");

  openfs(filename);
  //printf("DEBUG - main - after openfs\n");
  unsigned char buf[BSIZE];

  if(start_block < 0) {  //to use -e
    //printf("DEBUG - main - using -e\n");  //DEBUG
    int num_blocks = 1;
    start_block = 0;
    while(1){
      //printf("DEBUG - num_blocks: %d\n",num_blocks);
      if (bread(num_blocks, buf) == 0) {
        break;
      } 
      num_blocks++;
    }

    printf("The last block is block %d\n",num_blocks);
    return 0;
  }

  for (int i = start_block; i < start_block + blocks; i++) {
    if (bread(i, buf) > 0) {
      printf("block: %05d: \n", i);
      for (int j = 0; j < BSIZE/LINE; j++) {
        printf("0x%08x  ", i*BSIZE+j*LINE);
        for (int k = 0; k < LINE; k++) {
          printf("%02x", buf[j*LINE+k]);
          if ((j*LINE+k+1) % 4 == 0)
            printf(" ");
        }
        printf("  ");
        for (int k = 0; k < LINE; k++)
        //if (buf[j*LINE+k] >= 'A' && buf[j*LINE+k] <= 'z')
        if (isprint(buf[j*LINE+k]))
          printf("%c", buf[j*LINE+k]);
        else
          printf(" ");
        printf("\n");
      }
    }
    else {
      break; } // reached EOF
  }
  closefs();
}
// end
