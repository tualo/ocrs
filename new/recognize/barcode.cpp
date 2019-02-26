std::string ImageRecognizeEx::getBarcode(){
  return code;
}

void  ImageRecognizeEx::useBarcodeClahe(bool val){
  barcodeClahe = val;
}


bcResult ImageRecognizeEx::barcode_internal(cv::Mat &part, bool forceFPCode) {
  _debugTime("start barcode_internal");

  bcResult res = {cv::Point(0,0),cv::Rect(0,0,1,1),std::string(""),std::string(""),false};

  if (part.channels()>1){
    throw std::runtime_error("Error: ImageRecognizeEx::barcode_internal not a gray image");
  }

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

  cv::Mat grayo=part.clone();
  zbar::Image* _image;
  zbar::ImageScanner* _imageScanner;
  _image = new zbar::Image(grayo.cols, grayo.rows, "Y800", nullptr, 0);
  _imageScanner = new zbar::ImageScanner();
  _imageScanner->set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 1);
  //showImage(grayo,10000);

  _image->set_data((uchar *)grayo.data, grayo.cols * grayo.rows);
  int n = _imageScanner->scan(*_image);
  //zbar::Image::SymbolIterator symbol = _image->symbol_begin();
  //std::cout << "--- "<< strdup(symbol->get_data().c_str()) << std::endl;

  for(zbar::Image::SymbolIterator symbol = _image->symbol_begin(); symbol != _image->symbol_end(); ++symbol) {
    if (showDebug){
      std::cout << "without thres Code " << symbol->get_data().c_str() << " Type " << symbol->get_type_name().c_str() << std::endl;
    }
    std::string code = std::string(symbol->get_data().c_str());
    codes += code+" ";
    std::string type = std::string(symbol->get_type_name().c_str());


    if ((code.length()>5) && (code.length() > res.code.length())) {
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
        if (showDebug){
          std::cout << "Code*: "<< res.found << " c:" << res.code << " type:" << type  << std::endl;
        }

        //resultThres = thres;
        res.type = std::string(symbol->get_type_name().c_str());

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
  _debugTime("stop barcode_internal -----> * "+res.code);
  return res;
}

bcResult ImageRecognizeEx::barcode_internal_old(cv::Mat &part, bool forceFPCode) {
  _debugTime("start barcode_internal");

  bcResult res = {cv::Point(0,0),cv::Rect(0,0,1,1),std::string(""),std::string(""),false};

  if (part.channels()>1){
    throw std::runtime_error("Error: ImageRecognizeEx::barcode_internal not a gray image");
  }

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


    bool codeRetry=false;
    if (showDebug){
      std::cout << "barcode_internal " << std::endl;
    }

    cv::Mat image_clahe;
    if (barcodeClahe==true){
      if (showDebug){
        std::cout << "barcode_internal CLAHE" << std::endl;
      }
      cv::Mat lab_image;
      if (part.channels()==1){
        cv::cvtColor(part, gray, CV_BGR2GRAY);
        cv::cvtColor(gray, lab_image, CV_BGR2Lab);
      }else{
        //cv::cvtColor(part, gray, CV_GRAY2BGR);
        cv::cvtColor(part, lab_image, CV_BGR2Lab);
      }
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

    std::cout << "before loop " << "i_bc_thres_stop" << i_bc_thres_stop << std::endl;
    codes="";
    // counting here down
    /*
    for (int thres=i_bc_thres_start;((thres<=i_bc_thres_stop)&&(
      res.found==false && codeRetry==false
    ));thres+=i_bc_thres_step){
      */

      for (int thres=i_bc_thres_stop;((thres>=i_bc_thres_start)&&(
        res.found==false && codeRetry==false
      ));thres-=i_bc_thres_step){

      //std::cout << " loop " << thres << std::endl;

      if (image_clahe.channels()==1){
        cv::threshold(image_clahe,gray,thres,255, CV_THRESH_BINARY );
      }else{
        cv::cvtColor(image_clahe, gray, CV_BGR2GRAY);
      //  cv::threshold(gray,gray,thres,255, CV_THRESH_BINARY );
      }
      //cv::normalize(gray, norm, min, max, type, dtype, mask);
      //cv::GaussianBlur(norm, norm, ksize, 0);




      zbar::Image image(norm.cols, norm.rows, "Y800", (uchar *)norm.data, norm.cols * norm.rows);
      scanner.scan(image);
      for(zbar::Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol) {
        if (showDebug){
          std::cout << "thres " << thres << " Code " << symbol->get_data().c_str() << " Type " << symbol->get_type_name().c_str() << std::endl;
        }
        std::string code = std::string(symbol->get_data().c_str());
        codes += code+" ";
        std::string type = std::string(symbol->get_type_name().c_str());


        if ((code.length()>5) && (code.length() > res.code.length())) {
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
            if (showDebug){
              std::cout << "Code*: "<< res.found << " c:" << res.code << " type:" << type  << std::endl;
            }

            //resultThres = thres;
            res.type = std::string(symbol->get_type_name().c_str());

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

      //std::cerr << "barcode_internal (loop) " << res.found << " c:" << code << std::endl;

    }
  }catch(cv::Exception cv_error){
    std::cerr << "barcode_internal()" << cv_error.msg << std::endl;
  }
  std::cerr << "barcode_internal " << res.found << " c:" << code << std::endl;

  _debugTime("stop barcode_internal -----> "+res.code);
  return res;
}



void ImageRecognizeEx::barcode(){
  initBarcodeRegions();

  if (barcodeFP==false){
    barcodeFP = forceFPNumber();
  }
  bcResult res = {cv::Point(0,0),cv::Rect(0,0,1,1),std::string(""),std::string(""),false};
  _debugTime("start barcode");
  try{
    if (barcodeRegions.empty()){

        std::cout << "There are not BC Regions defined " << std::endl;
        exit(-1);
    }
    RegionOfInterest* roi;
    cv::Mat bc_roi;
    for(std::list<RegionOfInterest*>::iterator list_iter = barcodeRegions.begin();
      list_iter != barcodeRegions.end(); list_iter++){

        if (res.found==false){
          roi=*list_iter;

          std::cout << "barcodeRegions " << roi.rect().x << " " << roi.rect().y << " " << roi.rect().w <<  " " << roi.rect().h << std::endl;

          roi->setImage(orignalImage);
          bc_roi = orignalImage(roi->rect());
          //cv::GaussianBlur(bc_roi,bc_roi,cv::Size(3,3),2,2);
          res = barcode_internal(bc_roi,barcodeFP);
          cv::rectangle(
            roiImage,
            roi->rect(),
            cv::Scalar(205, 205, 0),
            5
          );
          std::cout << "1 " << std::endl;

          //showImage(roiImage);
          if (res.found==true){
            code = res.code;
            cv::rectangle(
              roiImage,
              roi->rect(),
              cv::Scalar(205, 0, 0),
              5
            );
            initStatistics();
            updateStatistics("state","barcodecode");
            showImage(roiImage);
          }
          std::cout << "2 " << std::endl;
        }
    }
  }catch(cv::Exception cv_error){
    std::cerr << "barcode()" << cv_error.msg << std::endl;
  }
  _debugTime("stop barcode");
}

void ImageRecognizeEx::initBarcodeRegions(){
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
  alter table bbs_address_regions add addressposition char(1) default 'L';
  alter table bbs_address_regions add minwidth integer default 2;
  alter table bbs_address_regions add maxwidth integer default 12;


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

  std::string ormachine=" or machine='*' ";
  if (getExplicitMachine()==true){
    ormachine=" ";
  }
  std::string sql = "select machine, name, x, y, w, h, rotate, rotate_steps from bbs_barcode_regions where machine = '"+machine+"' "+ormachine+" order by position ";
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
    for(; mysql_next_result(con) == 0;) /* do nothing */;
    mysql_free_result(result);
  }




}

void ImageRecognizeEx::initAddressRegions(){
  addressRegions.empty();

  std::string addressposition = addressfield;
  int width=orignalImage.cols/oneCM;
  std::string swidth=std::to_string(width);

  std::string ormachine=" or machine='*' ";
  if (getExplicitMachine()==true){
    ormachine=" ";
  }
  std::string sql = "select machine, name, x, y, w, h, rotate, rotate_steps from bbs_address_regions where (machine = '"+machine+"' "+ormachine+" ) and (addressposition='"+addressposition+"' or addressposition='*') and maxwidth >= "+swidth+" and minwidth <= "+swidth+" order by addressposition desc,position ";
  std::cout << "oneCM " << oneCM << std::endl;
  std::cout << "orignalImage.cols " << orignalImage.cols << " w " << width << std::endl;
  std::cout << "sql " << sql << std::endl;
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
       //std::cout << "roi " << name << " x: " << x << " y: " << y << " w: " << w << " h: " << h << std::endl;
       RegionOfInterest* roi= new RegionOfInterest(machine+" "+name,x,y,w,h,rotate,rotate_steps);
       roi->setPixelPerCM(oneCM);
       addressRegions.push_back(roi);
    }
    for(; mysql_next_result(con) == 0;) /* do nothing */;
    mysql_free_result(result);
  }
}
