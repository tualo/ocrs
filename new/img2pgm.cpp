#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/algorithm/string.hpp>


#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <pthread.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <stdio.h>

#include "args.hxx"


bool writeMatBinary(std::ofstream* ofs, const cv::Mat& out_mat)
{
	if(!ofs->is_open()){
		return false;
	}
	if(out_mat.empty()){
		int s = 0;
		ofs->write((const char*)(&s), sizeof(int));
		return true;
	}
	int type = out_mat.type();
	ofs->write((const char*)(&out_mat.rows), sizeof(int));
	ofs->write((const char*)(&out_mat.cols), sizeof(int));
	ofs->write((const char*)(&type), sizeof(int));
	ofs->write((const char*)(out_mat.data), out_mat.elemSize() * out_mat.total());

	return true;
}

int main( int argc, char** argv ){
  args::ArgumentParser parser("Convert any image to portable graymap.", "");
  args::HelpFlag help(parser, "help", "Display this help menu", { "help"});
  args::ValueFlag<std::string> in_filename(parser, "infilename", "The input filename", {'f',"infile"});
  args::ValueFlag<std::string> out_filename(parser, "outfilename", "The output filename", {'o',"outfile"});

  try
  {
    parser.ParseCLI(argc, argv);
  }
  catch (args::Help)
  {
      std::cout << parser;
      return 0;
  }
  catch (args::ParseError e)
  {
      std::cerr << e.what() << std::endl;
      std::cerr << parser;
      return 1;
  }

  const char* str_infile;
  const char* str_outfile;

  if ((in_filename)&&(out_filename)){
    str_infile= (args::get(in_filename)).c_str();
    str_outfile= (args::get(out_filename)).c_str();
    cv::Mat orignalImage = cv::imread( str_infile, cv::IMREAD_GRAYSCALE);


    std::ofstream* pFile;
    pFile = new std::ofstream(str_outfile, std::ios::out | std::ios::binary);

    writeMatBinary(pFile,orignalImage);
    pFile->close();
/*
    pFile->put(orignalImage.cols);
    pFile->put(orignalImage.rows);
    const char *b= reinterpret_cast<char *>(orignalImage.data);
    pFile->write(b,orignalImage.cols*orignalImage.rows);
    pFile->close();

    */
//    cv::imwrite(str_outfile, orignalImage, params);
  }



}
