
void  ImageRecognizeEx::useBarcodeClahe(bool val){
  barcodeClahe = val;
}

bcResult ImageRecognizeEx::barcode_internal(cv::Mat &part, bool forceFPCode) {

  bcResult res = {cv::Point(0,0),cv::Rect(0,0,1,1),std::string(""),std::string(""),false};

  try{


    cv::Mat gray;
    cv::Mat norm;
    cv::Mat mask;
    int type = cv::NORM_MINMAX;
    int dtype = -1;
    int min=0;
    int max=255;
    cv::Point point;
    cv::Size ksize(5,5);

    int rel=0;
    int tmp=0;
    bool codeRetry=false;
    if (showDebug){
      std::cout << "barcode_internal " << std::endl;
    }

    cv::Mat image_clahe;
    if (barcodeClahe==true){
      cv::Mat lab_image;
      //cv::cvtColor(part, gray, CV_GRAY2BGR);
      cv::cvtColor(part, lab_image, CV_BGR2Lab);

      // Extract the L channel
      std::vector<cv::Mat> lab_planes(3);
      cv::split(lab_image, lab_planes);  // now we have the L image in lab_planes[0]

      // apply the CLAHE algorithm to the L channel
      cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
      clahe->setClipLimit(4);
      cv::Mat dst;
      clahe->apply(lab_planes[0], dst);

      // Merge the the color planes back into an Lab image
      dst.copyTo(lab_planes[0]);
      cv::merge(lab_planes, lab_image);

     // convert back to RGB
     cv::cvtColor(lab_image, image_clahe, CV_Lab2BGR);
    }else{
      image_clahe=part.clone();
    }


    codes="";
    // counting here down
    for (int thres=15;((thres<220)&&(
      res.found==false && codeRetry==false
    ));thres+=5){

      cv::cvtColor(image_clahe, gray, CV_BGR2GRAY);
      cv::threshold(gray,gray,thres,255, CV_THRESH_BINARY );
      cv::normalize(gray, norm, min, max, type, dtype, mask);
      cv::GaussianBlur(norm, norm, ksize, 0);


  //    cv::adaptiveThreshold(gray,norm,255,CV_ADAPTIVE_THRESH_GAUSSIAN_C,CV_THRESH_BINARY,7,40);


      zbar::ImageScanner scanner;
      scanner.set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 1);

      scanner.set_config(zbar::ZBAR_CODE128, zbar::ZBAR_CFG_ENABLE, 1);
      scanner.set_config(zbar::ZBAR_CODE128, zbar::ZBAR_CFG_ADD_CHECK, 1);
      scanner.set_config(zbar::ZBAR_CODE128, zbar::ZBAR_CFG_EMIT_CHECK, 0);

      scanner.set_config(zbar::ZBAR_CODE39, zbar::ZBAR_CFG_ENABLE, 1);
      scanner.set_config(zbar::ZBAR_CODE39, zbar::ZBAR_CFG_ADD_CHECK, 1);
      scanner.set_config(zbar::ZBAR_CODE39, zbar::ZBAR_CFG_EMIT_CHECK, 0);


      scanner.set_config(zbar::ZBAR_I25, zbar::ZBAR_CFG_ENABLE, 1);
      scanner.set_config(zbar::ZBAR_I25, zbar::ZBAR_CFG_ADD_CHECK, 1);
      scanner.set_config(zbar::ZBAR_I25, zbar::ZBAR_CFG_EMIT_CHECK, 0);


      zbar::Image image(norm.cols, norm.rows, "Y800", (uchar *)norm.data, norm.cols * norm.rows);
      //showImage(norm);
      scanner.scan(image);
      /*
      int n = scanner.scan(image);

      if (showDebug){
        std::cout << "found " << n << " barcode(s) " << std::endl;
      }
      */

      for(zbar::Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol) {
        if (showDebug){
          std::cout << "thres " << thres << " Code " << symbol->get_data().c_str() << " Type " << symbol->get_type_name().c_str() << std::endl;
        }
        std::string code = std::string(symbol->get_data().c_str());
        codes += code+" ";
        std::string type = std::string(symbol->get_type_name().c_str());
        if ((code.length() > res.code.length())){
          if (
            (
              ( (type=="I2/5") && (is_digits(code)) ) ||
              ( (type!="I2/5")  )
            ) && (
              code.substr(0,4) != "0000"
            )
          ){
            if (showDebug){
              std::cout << "Code Length: " << code.length()-1 << std::endl;
            }
            if (type=="I2/5"){
              res.code = code.substr(0,code.length()-1);
              if (code.length()-1<11){
              }else{
                res.found = true;
              }
            }else{
              res.code = code;//std::string(symbol->get_data().c_str());
              res.found = true;
            }

            //resultThres = thres;
            res.type = std::string(symbol->get_type_name().c_str());
            int loc_size = symbol->get_location_size();

            int min_x=9999;
            int max_x=0;
            int min_y=9999;
            int max_y=0;

            for(int i=0;i<loc_size;i++){
              tmp = (symbol->get_location_y(i)*100/gray.rows);

              if (max_y<symbol->get_location_y(i)){
                max_y=symbol->get_location_y(i);
              }
              if (min_y>symbol->get_location_y(i)){
                min_y=symbol->get_location_y(i);
              }

              if (max_x<symbol->get_location_x(i)){
                max_x=symbol->get_location_x(i);
              }
              if (min_x>symbol->get_location_x(i)){
                min_x=symbol->get_location_x(i);
              }
              if (rel<tmp){
                rel=tmp;
                res.point = cv::Point(symbol->get_location_x(i),symbol->get_location_y(i));
              }
            }

            int margin = 0;//oneCM*1;

            if (min_x>margin){
              min_x-=margin;
            }else{
              min_x=0;
            }

            if (min_y>margin){
              min_y-=margin;
            }else{
              min_y=0;
            }

            if (max_x<gray.cols-margin){
              max_x+=margin;
            }else{
              max_x=gray.cols;
            }

            if (max_y<gray.rows-margin){
              max_y+=margin;
            }else{
              max_y=gray.rows;
            }
            // to do
            res.rect = cv::Rect(min_x,min_y,max_x-min_x,max_y-min_y);
          }

          if (forceFPCode){
            if ( (type=="I2/5") && (is_digits(code)) && (code.length()-1==11) ){

              res.found = true;
            }else{
              // if code does not match
              res.found = false;
            }
          }

        }
      }
      image.set_data(NULL, 0);
      scanner.recycle_image(image);
    }
  }catch(cv::Exception cv_error){
    std::cerr << "barcode_internal()" << cv_error.msg << std::endl;
  }
  return res;
}



void ImageRecognizeEx::barcode(){
  bcResult res = {cv::Point(0,0),cv::Rect(0,0,1,1),std::string(""),std::string(""),false};
  try{
    if (barcodeRegions.empty()){

        std::cout << "There are not BC Regions defined " << std::endl;
        exit(-1);
    }

    RegionOfInterest* roi;
    cv::Mat bc_roi;
    for(std::list<RegionOfInterest*>::iterator list_iter = barcodeRegions.begin();
      list_iter != barcodeRegions.end(); list_iter++){
        roi=*list_iter;
        if (showDebug){
          std::cout << "check region: " << roi->name()  << std::endl;
        }
        roi->setImage(orignalImage);
        if (showDebug){
          std::cout << "check region: " << roi->name() << " w" << roi->rect().width << " h"<< roi->rect().height << std::endl;
        }


        bc_roi = orignalImage(roi->rect());
        res = barcode_internal(bc_roi,barcodeFP);
        cv::rectangle(
          roiImage,
          roi->rect(),
          cv::Scalar(205, 205, 0),
          5
        );
        showImage(roiImage);
        if (res.found==true){
          code = res.code;
          cv::rectangle(
            roiImage,
            roi->rect(),
            cv::Scalar(205, 0, 0),
            5
          );
          showImage(roiImage);
        }

    }
  }catch(cv::Exception cv_error){
    std::cerr << "barcode()" << cv_error.msg << std::endl;
  }
}

void ImageRecognizeEx::initRegions(){
  /*
  create table bbs_barcode_regions(
    machine varchar(5) not null,
    name varchar(20) not null,

    x fixed(5,2) not null default 1,
    y fixed(5,2) not null default 1,
    w fixed(5,2) not null default 1,
    h fixed(5,2) not null default 1,
    rotate integer not null default 0,

    primary key (machine,name)
  );
  alter table bbs_barcode_regions add position integer default 1;
  alter table bbs_barcode_regions add rotate_steps integer default 0;

  insert into bbs_barcode_regions(
    machine,
    name,
    x,
    y,
    w,
    h,
    rotate
  ) values (
    '*',
    'FP Standard',
    2,
    0.5,
    2,
    17,
    0
  ) on duplicate key update
  x=values(x),
  y=values(y),
  h=values(h),
  w=values(w),
  rotate=values(rotate);




  create table bbs_address_regions(
    machine varchar(5) not null,
    name varchar(20) not null,

    x fixed(5,2) not null default 1,
    y fixed(5,2) not null default 1,
    w fixed(5,2) not null default 1,
    h fixed(5,2) not null default 1,
    rotate integer not null default 0,

    primary key (machine,name)
  );
  alter table bbs_address_regions add position integer default 1;
  alter table bbs_address_regions add rotate_steps integer default 0;


  insert into bbs_address_regions(
    machine,
    name,
    x,
    y,
    w,
    h,
    rotate
  ) values (
    '*',
    'DIN LANG',
    5,
    1,

    1,
    1,
    0
  ) on duplicate key update
  x=values(x),
  y=values(y),
  h=values(h),
  w=values(w),
  rotate=values(rotate);
  */


  std::string sql = "select machine, name, x, y, w, h, rotate, rotate_steps from bbs_barcode_regions where machine = '"+machine+"' or machine='*' order by position ";
  if (showDebug){
    std::cout << "Query BC Regions: " << sql << std::endl;
  }

  if (mysql_query(con, sql.c_str())){
    std::cout << "EE " << sql << std::endl;
    fprintf(stderr, "%s\n", mysql_error(con));
  }else{
    MYSQL_RES *result;
    MYSQL_ROW row;
    unsigned int num_fields;


    std::string machine;
    std::string name;
    double x;
    double y;
    double h;
    double w;
    int rotate;
    int rotate_steps;

    result = mysql_use_result(con);
    num_fields = mysql_num_fields(result);
    while ((row = mysql_fetch_row(result))){
       //unsigned long *lengths;
       machine=std::string( row[0] );
       name=std::string( row[1] );

       x = atof(row[2]);
       y = atof(row[3]);
       w = atof(row[4]);
       h = atof(row[5]);
       rotate = atoi(row[6]);
       rotate_steps = atoi(row[7]);

       RegionOfInterest* roi= new RegionOfInterest(machine+" "+name,x,y,w,h,rotate,rotate_steps);
       roi->setPixelPerCM(oneCM);
       barcodeRegions.push_back(roi);


    }
    mysql_free_result(result);
  }



  sql = "select machine, name, x, y, w, h, rotate, rotate_steps from bbs_address_regions where machine = '"+machine+"' or machine='*' order by position ";
  if (showDebug){
    std::cout << "Query Address Regions: " << sql << std::endl;
  }

  if (mysql_query(con, sql.c_str())){
    std::cout << "EE " << sql << std::endl;
    fprintf(stderr, "%s\n", mysql_error(con));
  }else{
    MYSQL_RES *result;
    MYSQL_ROW row;
    unsigned int num_fields;


    std::string machine;
    std::string name;
    double x;
    double y;
    double h;
    double w;
    int rotate;
    int rotate_steps;

    result = mysql_use_result(con);
    num_fields = mysql_num_fields(result);
    while ((row = mysql_fetch_row(result))){
       //unsigned long *lengths;
       machine=std::string( row[0] );
       name=std::string( row[1] );

       x = atof(row[2]);
       y = atof(row[3]);
       w = atof(row[4]);
       h = atof(row[5]);
       rotate = atoi(row[6]);
       rotate_steps = atoi(row[7]);

       std::cout << "roi " << name << " x: " << x << " y: " << y << " w: " << w << " h: " << h << std::endl;

       RegionOfInterest* roi= new RegionOfInterest(machine+" "+name,x,y,w,h,rotate,rotate_steps);
       roi->setPixelPerCM(oneCM);
       addressRegions.push_back(roi);


    }
    mysql_free_result(result);
  }

}
