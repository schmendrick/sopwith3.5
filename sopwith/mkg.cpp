#include <fstream>
#include <iostream>
#include <string>
#include <cerrno>

static const int buflen=256;

char* findstart(char* p)
{
  while (*p==' ')
    ++p;
  return p;
}

int getint(std::ifstream& i)
{
  char l[buflen];
  i.getline(l,buflen);
  char* p=findstart(l);
  return atoi(p);
}

std::string getstring(std::ifstream& i)
{
  char l[buflen];
  i.getline(l,buflen);
  char* p=findstart(l);
  char* q=p;
  while (*q!=' ' && *q!='\r' && *q!='\n' && *q!=0)
    ++q;
  *q=0;
  return p;
}

int main(int argc,char* argv[])
{
  /* Check arguments */
  std::string datafilename("mkg"); // Default argument
  if (argc>1)
    datafilename=argv[1];

  /* Open file */
  std::string datafilenametxt(datafilename+".txt");
  /* Although the data file is a text file it needs to be opened in binary
     mode for tellg() and seekg() to work in the library supplied with GCC.
     This seems to be a known bug in the library. */
  std::ifstream datafile(datafilenametxt.c_str(),std::ios::in | std::ios::binary);
  if (!datafile) {
    datafile.clear();
    datafile.open(datafilename.c_str(),std::ios::in | std::ios::binary);
    if (!datafile) {
      std::cout << "Error: Cannot open file \"" << argv[1] << "\": " <<
                   sys_errlist[errno] << std::endl;
      return 1;
    }
  }

  /* Find start of file */
  char l[buflen];
  do
    datafile.getline(l,buflen);
  while (l[0]!='*');

  /* Read colour information */
  static const int numcols=4;
  char colours[256];
  for (int i=0;i<numcols;++i)
    colours[getint(datafile)]=i;

  /* Read input file name and open input file */
  std::string inputfilename(getstring(datafile));
  std::string inputfilenamespr(inputfilename+".spr");
  std::ifstream inputfile(inputfilenamespr.c_str(),std::ios::in | std::ios::binary);
  if (!inputfile) {
    inputfile.clear();
    inputfile.open(inputfilename.c_str(),std::ios::in | std::ios::binary);
    if (!inputfile) {
      std::cout << "Error: Cannot open input file \"" << inputfilename <<
                   "\" mentioned in \"" << argv[1] << "\": " <<
                   sys_errlist[errno] << std::endl;
      return 1;
    }
  }

  /* Read output file name and open output file */
  std::string outputfilename(getstring(datafile));
  std::ofstream outputfile(outputfilename.c_str(),std::ios::out);
  if (!outputfile) {
    std::cout << "Error: Cannot open output file \"" << outputfilename <<
                 "\" mentioned in \"" << argv[1] << "\": " <<
                 sys_errlist[errno] << std::endl;
    return 1;
  }

  outputfilename=getstring(datafile);

  /* Read number of records */
  int num=getint(datafile);

  std::streampos dstart=datafile.tellg();

  outputfile.setf(outputfile.hex,outputfile.basefield);

  /* Read records */
  for (int i=0;i<num;++i) {
    getint(datafile);

    /* Read and output sprite name */
    outputfile << "unsigned char " << getstring(datafile);

    /* Read dimensions */
    int dimensions=getint(datafile);
    static const int maxdimensions=10;
    int size[maxdimensions];
    for (int i=0;i<dimensions;++i) {
      size[i]=getint(datafile);
      outputfile << "[0x" << size[i] << "]";
    }
    int height=getint(datafile);
    int width=(getint(datafile)+3)&(~3);

    /* Output C code */
    outputfile << "[0x" << height*width/4 << "]={" << std::endl << "  ";
    int c=dimensions-1;
    int counter[maxdimensions];
    for (int i=0;i<dimensions;++i) {
      counter[i]=0;
      outputfile << "{";
    }
    do {
      int k=0;

      /* Output actual data */
      for (int y=0;y<height;++y) {
        for (int x=0;x<width/4;++x) {
          int d=0;
          for (int b=6;b>=0;b-=2) {
            char col;
            inputfile.get(col);
            d|=(colours[col]&3)<<b;
          }
          if (d<16)
            outputfile << "0x0" << d;
          else
            outputfile << "0x" << d;

          /* Make it look pretty */
          bool f=false;
          if (y==height-1 && x==width/4-1) {
            f=true;
            while (c>=0) {
              outputfile << "}";
              ++counter[c];
              if (counter[c]==size[c]) {
                counter[c]=0;
                c--;
              }
              else
                break;
            }
            if (c>=0)
              outputfile << ",";
            else
              outputfile << "};" << std::endl;
          }
          else
            outputfile << ",";
          ++k;
          if (k==16 || f) {
            k=0;
            outputfile << std::endl;
            if (c>=0 || !f) {
              outputfile << "  ";
              for (int i=0;i<dimensions;++i)
                if (i>=c && f)
                  outputfile << "{";
                else
                  outputfile << " ";
              c=dimensions-1;
            }
          }
        } // x loop
        for (int x=width;x<32;++x) {
          char col;
          inputfile.get(col);
        }
      } // y loop
      for (int y=height;y<32;++y)
        for (int x=0;x<32;++x) {
          char col;
          inputfile.get(col);
        }
    } while (c>=0);
  }

  outputfile.close();
  outputfile.open(outputfilename.c_str(),std::ios::out);
  if (!outputfile) {
    std::cout << "Error: Cannot open output file \"" << outputfilename <<
                 "\" mentioned in \"" << argv[1] << "\": " <<
                 sys_errlist[errno] << std::endl;
    return 1;
  }

  datafile.seekg(dstart);
  inputfile.seekg(0,std::ios::beg);

  outputfile.setf(outputfile.hex,outputfile.basefield);

  /* Read records */
  for (int i=0;i<num;++i) {
    int f=getint(datafile);

    /* Read and output sprite name */
    if (f==0)
      getstring(datafile);
    else
      outputfile << "unsigned char " << getstring(datafile) << "coll";

    /* Read dimensions */
    int dimensions=getint(datafile);
    static const int maxdimensions=10;
    int size[maxdimensions];
    int bytes=1024,numcols=1;
    if (f==1) {
      numcols=getint(datafile);
      --dimensions;
    }
    for (int i=0;i<dimensions;++i) {
      size[i]=getint(datafile);
      bytes*=size[i];
      if (f!=0)
        outputfile << "[0x" << size[i] << "]";
    }
    int height=getint(datafile);
    int width=(getint(datafile)+7)&(~7);

    if (f==0) {
      inputfile.seekg(bytes,std::ios::cur);
      continue;
    }

    /* Output C code */
    outputfile << "[0x" << height*width/8 << "]={" << std::endl << "  ";
    int c=dimensions-1;
    int counter[maxdimensions];
    for (int i=0;i<dimensions;++i) {
      counter[i]=0;
      outputfile << "{";
    }
    do {
      int k=0;

      /* Output actual data */
      for (int y=0;y<height;++y) {
        for (int x=0;x<width/8;++x) {
          int d=0;
          for (int b=0;b<8;++b) {
            char col;
            inputfile.get(col);
            d|=(colours[col]!=0 ? 1 : 0)<<b;
          }
          if (d<16)
            outputfile << "0x0" << d;
          else
            outputfile << "0x" << d;

          /* Make it look pretty */
          bool f=false;
          if (y==height-1 && x==width/8-1) {
            f=true;
            while (c>=0) {
              outputfile << "}";
              ++counter[c];
              if (counter[c]==size[c]) {
                counter[c]=0;
                --c;
              }
              else
                break;
            }
            if (c>=0)
              outputfile << ",";
            else
              outputfile << "};" << std::endl;
          }
          else
            outputfile << ",";
          ++k;
          if (k==16 || f) {
            k=0;
            outputfile << std::endl;
            if (c>=0 || !f) {
              outputfile << "  ";
              for (int i=0;i<dimensions;++i)
                if (i>=c && f)
                  outputfile << "{";
                else
                  outputfile << " ";
              c=dimensions-1;
            }
          }
        } // x loop
        for (int x=width;x<32;++x) {
          char col;
          inputfile.get(col);
        }
      } // y loop
      for (int y=height;y<32;++y)
        for (int x=0;x<32;++x) {
          char col;
          inputfile.get(col);
        }
    } while (c>=0);

    if (f==1)
      inputfile.seekg(bytes*(numcols-1),std::ios::cur);
  }
}
