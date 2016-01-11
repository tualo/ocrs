#include "ImageRecognize.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <pthread.h>
#include <vector>
#include <string>

#include "glob.h"

struct thread_info {
  int index;
};
typedef struct thread_info thread_info;

std::vector<std::string> files;
glob::Glob* gl;

void* processImage(void *data ){
  struct thread_info *tib;
  tib = (struct thread_info *)data;

  pthread_exit((void*)42);
	return 0;
}



/**
 * @function main
 */
int main( int argc, char** argv )
{
  double t = (double)cv::getTickCount();



  gl=new glob::Glob(argv[1]);
  if (!gl->hasNext()){
    ImageRecognize* ir = new ImageRecognize();
    ir->open(argv[1]);
  }else{
    while (gl->hasNext()) {
      std::cout << gl->GetFileName() << std::endl;
      gl->Next();
      ImageRecognize* ir = new ImageRecognize();
      std::stringstream str;
      str << argv[1] << "/" << gl->GetFileName();
      std::string filename = str.str();
      char *fn = new char[filename.length() + 1];
      strcpy(fn, filename.c_str());
      ir->open(fn);
      delete[] fn;
    }
  }

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
  t = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
  std::cout << "Times passed in seconds: " << t << std::endl;
  return -1;
}
