#include "ImageRecognize.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <pthread.h>
#include <vector>
#include <string>

#include "glob.h"

#define NUM_THREADS 100


/* MySQL Connector/C++ specific headers */
/*
#include <driver.h>
#include <connection.h>
#include <statement.h>
#include <prepared_statement.h>
#include <resultset.h>
#include <metadata.h>
#include <resultset_metadata.h>
#include <exception.h>
#include <warning.h>
*/

struct thread_info {
  int index;
  std::string filename;
};
//typedef struct thread_info thread_info;
struct thread_info threadInfo[NUM_THREADS];// = {0,0,0,0};// = malloc(sizeof(struct thread_block_info));

std::vector<std::string> files;
glob::Glob* gl;

void* processImage(void *data ){
  struct thread_info *tib;
  tib = (struct thread_info *)data;

  std::cout << " inside thread " << tib->filename << std::endl;
  ImageRecognize* ir = new ImageRecognize();
  const char* fn = tib->filename.c_str();

  ir->open( fn );

  pthread_exit((void*)42);
	return 0;
}



/**
 * @function main
 */
int main( int argc, char** argv )
{
  double t = (double)cv::getTickCount();
  int cthread = 0;


  gl=new glob::Glob(argv[1]);
  if (!gl->hasNext()){
    ImageRecognize* ir = new ImageRecognize();
    ir->open(argv[1]);
  }else{

    while (gl->hasNext()) {
      //std::cout << gl->GetFileName() << " #"<< cthread << std::endl;
      if (cthread>=NUM_THREADS){
        cthread=0;
      }
      if (cthread<NUM_THREADS){


        std::stringstream str;
        str << argv[1] << "/" << gl->GetFileName();
        std::string filename = str.str();
        char *fn = new char[filename.length() + 1];
        strcpy(fn, filename.c_str());

        thread_info* t_info = &threadInfo[cthread];
        t_info->filename = fn;
        pthread_t thread;
        int ct = pthread_create( &thread, NULL, processImage, (void*) t_info);
        if( ct  != 0) {
         printf("something went wrong while threading %i\n",ct);
        }else{

        }
        std::cout << gl->GetFileName() << " started" << std::endl;
        cthread++;
        if (cthread==1){
          pthread_join(thread,NULL);
          cthread=0;

          t = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
          std::cout << "Times passed in seconds: " << t << std::endl;
          t = (double)cv::getTickCount();

        }else{
          pthread_detach(thread);
        }
      }
      gl->Next();
      /*
      ImageRecognize* ir = new ImageRecognize();
      std::stringstream str;
      str << argv[1] << "/" << gl->GetFileName();
      std::string filename = str.str();
      char *fn = new char[filename.length() + 1];
      strcpy(fn, filename.c_str());
      ir->open(fn);
      delete[] fn;
      t = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
      std::cout << "Times passed in seconds: " << t << std::endl;
      t = (double)cv::getTickCount();
      */
    }
  }
  t = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
  std::cout << "Times passed in seconds: " << t << std::endl;

  //ImageRecognize* ir = new ImageRecognize();
  //ir->open(argv[1]);

  //filename
  //const unsigned char * constStr =
  /*
  thread_info packet_info = {0,reinterpret_cast<const unsigned char *> (strVar.c_str())};
  pthread_t thread;
  int ct = pthread_create( &thread, NULL, processImage, (void*)packet_info);
  if( ct  != 0) {
   printf("something went wrong while threading %i\n",ct);
   return 0;
  }
  pthread_join(thread,NULL);
  */
  return -1;
}
