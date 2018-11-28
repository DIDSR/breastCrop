/*! \file breastCrop.cxx
 *  \brief Program to crop compressed breast phantoms
 *  \author Christian G. Graff
 *  \version 1.0
 *  \date 2018
 *
 *  \copyright To the extent possible under law, the author(s) have 
 *  dedicated all copyright and related and neighboring rights to this 
 *  software to the public domain worldwide. This software is 
 *  distributed without any warranty.  You should have received a copy
 *  of the CC0 Public Domain Dedication along with this software. 
 *  If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 *
 */
 
#include "breastCrop.hxx"

#define FNAME_SIZE 256

namespace po = boost::program_options;

int main(int argc, char* argv[]){

  // command line parameters
  po::options_description opts("All options");
  opts.add_options()
    ("seed,s", po::value<int>()->required(), "input seed")
    ("dir,d", po::value<std::string>()->default_value("."),"work directory")
    ("gap,g", po::value<double>()->required(), "air gap (mm)")
    ("xsize,x", po::value<int>()->required(), "x voxels")
    ("ysize,y", po::value<int>()->required(), "y voxels")
    ("zsize,z", po::value<int>()->required(), "z voxels")
    ("help,h", "print help message")
    ;

  po::variables_map vm;

  // get command line arguments
  po::store(parse_command_line(argc,argv,opts), vm);

  if (vm.count("help")) {
    cout << "\n";
    cout << "breastCrop - cropping compressed breast phantoms\n\n";
    cout << opts << "\n";
    return 1;
  }

  if(!vm.count("seed") || !vm.count("gap") || !vm.count("xsize") || !vm.count("ysize") || !vm.count("zsize")){
    cout << "Command line options missing\n" ;
    cout << opts << "\n";
    return(EXIT_FAILURE); 
  }
  
  int seed = vm["seed"].as<int>();
  std::string workDir = vm["dir"].as<std::string>();

  char inputHdrFilename[FNAME_SIZE];
  char inputgzFilename[FNAME_SIZE];
  
  // input uncropped phantom files
  sprintf(inputHdrFilename, "%s/pc_%d.mhd", workDir.c_str(), seed);
  sprintf(inputgzFilename, "%s/pc_%d.raw.gz", workDir.c_str(), seed);
  	
  // phantom
  vtkSmartPointer<vtkImageData> input =
    vtkSmartPointer<vtkImageData>::New();

  // input phantom dimensions
  double inputSpacing[3];
  int inputDim[3];
  double inputOrigin[3];

  // parse MHD header
  FILE *inputHdr = fopen(inputHdrFilename,"r");
  if(inputHdr == NULL){
    cerr << "Unable to open MHD header file " << inputHdrFilename << "\n";
    return(EXIT_FAILURE);
  } else {
    char hdrString[FNAME_SIZE];
    for(int i=0; i<6; i++){
      if(fgets(hdrString, FNAME_SIZE, inputHdr) == NULL){
	cerr << "Error parsing header\n";
	return(EXIT_FAILURE);
      }
    }
    if(fgets(hdrString, FNAME_SIZE, inputHdr) == NULL){
      cerr <<"Error parsing header\n";
      return(EXIT_FAILURE);
    }
    sscanf(hdrString, "%*s %*s %lf %lf %lf", &inputOrigin[0], &inputOrigin[1], &inputOrigin[2]);
    if(fgets(hdrString, FNAME_SIZE, inputHdr) == NULL){
      cerr <<"Error parsing header\n";
      return(EXIT_FAILURE);
    }
    if(fgets(hdrString, FNAME_SIZE, inputHdr) == NULL){
      cerr <<"Error parsing header\n";
      return(EXIT_FAILURE);
    }
    sscanf(hdrString,"%*s %*s %lf %lf %lf",&inputSpacing[0], &inputSpacing[1], &inputSpacing[2]);
    if(fgets(hdrString, FNAME_SIZE, inputHdr) == NULL){
      cerr <<"Error parsing header\n";
      return(EXIT_FAILURE);
    }
    sscanf(hdrString,"%*s %*s %d %d %d",&inputDim[0], &inputDim[1], &inputDim[2]);
    fclose(inputHdr);
  }

  input->SetDimensions(inputDim);
  input->SetSpacing(inputSpacing);
  input->SetOrigin(inputOrigin);

  // allocate memory and load phantom data from gzip file
  input->AllocateScalars(VTK_UNSIGNED_CHAR, 1);

  gzFile gzf = gzopen(inputgzFilename, "rb");

  if(gzf == NULL){
    cerr << "Unable to open gzip phantom data file" << inputgzFilename << "\n";
    return(EXIT_FAILURE);
  } else {
    unsigned char *p = static_cast<unsigned char *>(input->GetScalarPointer());
    unsigned char *buffer;
    int bytesRead;
    long long int totalBytesRead = 0;
    buffer = (unsigned char *)malloc(inputDim[1]*inputDim[2]*sizeof(char));
    gzbuffer(gzf, inputDim[1]*inputDim[2]);

    for(int i=0; i<inputDim[0]; i++){
      bytesRead = gzread(gzf, buffer, inputDim[1]*inputDim[2]);
      if(bytesRead < 0){
	cout << gzerror(gzf, &bytesRead) << "\n";
      } else {
	totalBytesRead += bytesRead;
      }
      memcpy(p, buffer, inputDim[1]*inputDim[2]);
      p+=inputDim[1]*inputDim[2];
    }

    gzclose(gzf);
    free(buffer);
    cout << "Total bytes read = " << totalBytesRead << "\n";
  }
	
  tissueStruct tissue;

  tissue.bg = 0;
  tissue.skin = 2;
  tissue.nipple = 33;
  tissue.fat = 1;
  tissue.cooper = 88;
  tissue.gland = 29;
  tissue.TDLU = 95;
  tissue.duct = 125;
  tissue.artery = 150;
  tissue.vein = 225 ;
  tissue.muscle = 40;
  tissue.mass = 200;
  tissue.calc = 250;
  tissue.paddle = 50;

  double gap = vm["gap"].as<double>();

  int targetDim[3] = {vm["xsize"].as<int>(), vm["ysize"].as<int>(), vm["zsize"].as<int>()};

  int dim2[3];
  input->GetDimensions(dim2);
  int startExtent[6];
  input->GetExtent(startExtent);
  double res[3];
  input->GetSpacing(res);

  int cropExtent[6];
  
  // search +z dimension for top paddle location
  bool foundPt = false;
  int curPos[3] = {startExtent[0] + (startExtent[1]-startExtent[0])/2, startExtent[2] + (startExtent[3]-startExtent[2])/2, startExtent[5]};
  
  while(!foundPt && curPos[2] > startExtent[4]){
    unsigned char* voxVal = static_cast<unsigned char *>(input->GetScalarPointer(curPos));
    if(*voxVal == tissue.paddle){
      foundPt = true;
    } else {
      curPos[2] = curPos[2] - 1;
    }
  }

  if(curPos[2] <= startExtent[4]){
    cerr << "Error: Top paddle not found.\n";
    return(1);
  }

  cropExtent[5] = curPos[2];

  // search -z dimension for bottom paddle location
  foundPt = false;
  curPos[2] = startExtent[4];

  while(!foundPt && curPos[2] < cropExtent[5]){
    unsigned char* voxVal = static_cast<unsigned char *>(input->GetScalarPointer(curPos));
    if(*voxVal == tissue.paddle){
      foundPt = true;
    } else {
      curPos[2] = curPos[2] + 1;
    }
  }

  if(curPos[2] >= cropExtent[5]){
    cerr << "Error: Bottom paddle not found.\n";
    return(1);
  }

  cropExtent[4] = curPos[2];

  // search +x dimension for first breast material
  foundPt = false;
  int xPos = startExtent[1];

  while(!foundPt){
    for(int a=cropExtent[4]; a<=cropExtent[5]; a++){
      for(int b=startExtent[2]; b<=startExtent[3]; b++){
	unsigned char* voxVal = static_cast<unsigned char *>(input->GetScalarPointer(xPos,b,a));
	if(*voxVal != tissue.bg && *voxVal != tissue.paddle){
	  foundPt = true;
	}
      }
    }
    if(!foundPt){
      xPos--;
    }
  }
  
  // add air gap
  cropExtent[1] = xPos + (int)(ceil(gap/res[0]));
  if(cropExtent[1] > startExtent[1]){
    cropExtent[1] = startExtent[1];
  }

	// search -x dimension for first sign of paddle
  foundPt = false;
  xPos = startExtent[0];

  while(!foundPt){
    for(int a=cropExtent[4]; a<=cropExtent[5]; a++){
      for(int b=startExtent[2];b<=startExtent[3]; b++){
	unsigned char* voxVal = static_cast<unsigned char *>(input->GetScalarPointer(xPos,b,a));
	if(*voxVal == tissue.paddle){
          foundPt = true;
	}
      } 
    }
    if(!foundPt){
      xPos++;
    }
  }

  cropExtent[0] = xPos;

  // search -y dimension for first breast material
  foundPt = false;
  int yPos = startExtent[2];

  while(!foundPt){
    for(int a=cropExtent[0]; a<=cropExtent[1]; a++){
      for(int b=cropExtent[4];b<=cropExtent[5]; b++){
        unsigned char* voxVal = static_cast<unsigned char *>(input->GetScalarPointer(a,yPos,b));
        if(*voxVal != tissue.bg && *voxVal != tissue.paddle){
          foundPt = true;
        }
      }
    }
    if(!foundPt){
      yPos++;
    }
  }

	// add air gap
  cropExtent[2] = yPos - (int)(ceil(gap/res[1]));
  if(cropExtent[2] < startExtent[2]){
    cropExtent[2] = startExtent[2];
  }

	// search +y dimension for first breast material
  foundPt = false;
  yPos = startExtent[3];

  while(!foundPt){
    for(int a=cropExtent[0]; a<=cropExtent[1]; a++){
      for(int b=cropExtent[4];b<=cropExtent[5]; b++){
        unsigned char* voxVal = static_cast<unsigned char *>(input->GetScalarPointer(a,yPos,b));
        if(*voxVal != tissue.bg && *voxVal != tissue.paddle){
          foundPt = true;
        }
      }
    }
    if(!foundPt){
      yPos--;
    }
  }

	// add air gap
  cropExtent[3]= yPos + (int)(ceil(gap/res[1]));
  if(cropExtent[3] > startExtent[3]){
    cropExtent[3] = startExtent[3];
  }

  // achieve target dimensions by adjusting crop
  if(targetDim[0] > 0){
    cropExtent[1] = targetDim[0] + cropExtent[0] - 1;
    if(cropExtent[1] > startExtent[1]){
      cropExtent[1] = startExtent[1];
      cropExtent[0] = cropExtent[1] + 1 - targetDim[0];
    }

    cropExtent[3] = targetDim[1] + cropExtent[2] - 1;
    if(cropExtent[3] > startExtent[3]){
      cropExtent[3] = startExtent[3];
      cropExtent[2] = cropExtent[3] + 1 - targetDim[1];
    }
    
    cropExtent[5] = cropExtent[4] + targetDim[2] - 1;
    if(cropExtent[5] > startExtent[5]){
      cropExtent[5] = startExtent[5];
      cropExtent[4] = cropExtent[5] + 1 - targetDim[2];
    }
  }

  // have cropped Extent
  input->Crop(cropExtent);

  char outputHdrFilename[FNAME_SIZE];
  char outputgzFilename[FNAME_SIZE];

	// output data filenames
  sprintf(outputHdrFilename, "%s/pc_%d_crop.mhd", workDir.c_str(), seed);
  sprintf(outputgzFilename,"%s/pc_%d_crop.raw.gz", workDir.c_str(), seed);
	
  // save gzip data
  int finalDim[3];
  input->GetDimensions(finalDim);

  gzFile gzoutf = gzopen(outputgzFilename, "wb");

  if(gzoutf == NULL){
    cerr << "Unable to open gzip file for writing\n";
  } else {
    gzbuffer(gzoutf, finalDim[1]*finalDim[2]);
    unsigned char *p = static_cast<unsigned char*>(input->GetScalarPointer());
    for(int i=0; i<finalDim[0]; i++){
      gzwrite(gzoutf, static_cast<const void*>(p), finalDim[1]*finalDim[2]);
      p += finalDim[1]*finalDim[2];
    }

    gzclose(gzoutf);
  }

  // save MHD header
  FILE *hdrFile = fopen(outputHdrFilename, "w");

  if(hdrFile == NULL){
    cerr << "Unable to open mhd file for writing\n";
  } else {
    fprintf(hdrFile, "ObjectType = Image\n");
    fprintf(hdrFile, "NDims = 3\n");
    fprintf(hdrFile, "BinaryData = True\n");
    fprintf(hdrFile, "BinaryDataByteOrderMSB = False\n");
    fprintf(hdrFile, "CompressedData = False\n");
    fprintf(hdrFile, "TransformMatrix = 1 0 0 0 1 0 0 0 1\n");
    double finalOrigin[3];
    double finalSpacing[3];
    int finalExtent[3];
    input->GetSpacing(finalSpacing);
    input->GetOrigin(finalOrigin);
    input->GetExtent(finalExtent);
    finalOrigin[0] += finalExtent[0]*inputSpacing[0];
    finalOrigin[1] += finalExtent[2]*inputSpacing[1];
    finalOrigin[2] += finalExtent[4]*inputSpacing[2];
    fprintf(hdrFile, "Offset = %6.4f %6.4f %6.4f\n", finalOrigin[0], finalOrigin[1], finalOrigin[2]);
    fprintf(hdrFile, "CenterOfRotation = 0 0 0\n");
    fprintf(hdrFile, "ElementSpacing = %6.4f %6.4f %6.4f\n", inputSpacing[0], inputSpacing[1], inputSpacing[2]);
    fprintf(hdrFile, "DimSize = %d %d %d\n", finalDim[0], finalDim[1], finalDim[2]);
    fprintf(hdrFile, "AnatomicalOrientation = ???\n");
    fprintf(hdrFile, "ElementType = MET_UCHAR\n");
    fprintf(hdrFile, "ObjectType = Image\n");
    fprintf(hdrFile, "ElementDataFile = pc_%d_crop.raw\n", seed);
    fclose(hdrFile);
  }

  return(0);
}
