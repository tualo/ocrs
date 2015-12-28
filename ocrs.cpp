#include "ImageRecognize.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <pthread.h>
#include <glob.h>
#include <vector>
#include <string>

int MAX_FILENAME_LENGTH=255;

struct thread_info {
  int block_id;
  int loop;
  unsigned char data[MAX_FILENAME_LENGTH];
};


inline std::vector<std::string> glob(const std::string& pat){
  using namespace std;
  glob_t glob_result;
  glob(pat.c_str(),GLOB_TILDE,NULL,&glob_result);
  vector<string> ret;
  for(unsigned int i=0;i<glob_result.gl_pathc;++i){
      ret.push_back(string(glob_result.gl_pathv[i]));
  }
  globfree(&glob_result);
  return ret;
}

/**
 * @function main
 */
int main( int argc, char** argv )
{
  double t = (double)cv::getTickCount();


  ImageRecognize* ir = new ImageRecognize();
  ir->open(argv[1]);

  pthread_t thread;
   int ct = pthread_create( &thread, NULL, writePacket, (void*)packet_info);
   if( ct  != 0) {
     printf("something went wrong while threading %i\n",ct);
     return 0;
   }
   pthread_join(thread,NULL);

  t = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
  std::cout << "Times passed in seconds: " << t << std::endl;
  return -1;
}
