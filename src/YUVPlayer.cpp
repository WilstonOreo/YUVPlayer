#include <stdlib.h>
#include <iostream>
#include <boost/program_options.hpp>
#include <GL/freeglut.h>
#include <omp.h>

#include "YUV.hpp"

using namespace boost;
namespace po = program_options;

using namespace std;

static YUV::YUVInStream *pYuvInStream;
static YUV::YUVOutStream *pYuvOutStream;
static YUV::YUVImage *pYuvImage;

// Use Look up tables to avoid costly multiplications
static int LookUp88[256];
static int LookUp182[256];
static int LookUp359[256];
static int LookUp452[256];

static int FPS;
static unsigned char * pTexData;

inline unsigned char clip(int f)
{
	return  (f<0)? 0 : (f>65535) ? 255 : f/256;
}

void display( )
{
  glClearColor(0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT);

  if (!pYuvImage || !pTexData || !pYuvInStream || !pYuvInStream->good()) return;

  pYuvInStream->read();

  #pragma omp parallel
  {
    int n = omp_get_num_threads();
  	vector<YUV::u8*> Y,U,V; 
	Y.resize(n); U.resize(n); V.resize(n);

	// Calculate initial pointer position for each thread
	for (int i = 0; i < n; i++)
	{
		Y[i] = pYuvImage->y() + i*pYuvImage->width()*pYuvImage->height()/n;
		U[i] = pYuvImage->u() + i*pYuvImage->width()/2*pYuvImage->height()/2/n;
		V[i] = pYuvImage->v() + i*pYuvImage->width()/2*pYuvImage->height()/2/n;
	}

	#pragma omp for
  	for (int y = 0; y < pYuvImage->height(); y++)
  	{
		int i = omp_get_thread_num();
		int pos = 0;

  		for (int x = 0; x < pYuvImage->width(); x++)
		{
			pos = (pYuvImage->height()-1-y)*pYuvImage->width()+x;
			pos += 2*pos;
			int py = 256*(*Y[i])- 128;

			// YUV --> RGB
			pTexData[pos  ] = clip(py + LookUp359[*V[i]] );
			pTexData[pos+1] = clip(py - LookUp88 [*U[i]] - LookUp182[*V[i]]);
			pTexData[pos+2] = clip(py + LookUp452[*U[i]] );

			// Pointer increasing for fast pixel access
			Y[i]++;
			U[i] += x & 1;
			V[i] += x & 1;
		}

		if (!(y & 1))
		{
			U[i] -= pYuvImage->width()/2;
			V[i] -= pYuvImage->width()/2;
		}
	}
  }

  glDrawPixels(pYuvImage->width(), pYuvImage->height(), GL_RGB, GL_UNSIGNED_BYTE, pTexData);

  if (pYuvOutStream) pYuvOutStream->write();
  glutSwapBuffers();
}

void update(int value) 
{
	glutPostRedisplay();
    glutTimerFunc(FPS, update, 0);
}


int main(int ac, char* av[])
{
	string appTitle = "YUV Player v0.3. -- written by Wilston Oreo.";
	cout << appTitle << endl;
	cout << "Licensed under LGPL." << endl;

	stringstream descStr; 
	descStr << "Allowed options";

	string inputFile,outputFile;
	int width,height,nFrames,fps = 1000/24;

	// Declare the supported options.
	po::options_description desc(descStr.str());
	desc.add_options()
		("help", "Display help message.")
		("inputfile,i", po::value<string>(&inputFile), "Input file")
		("width,w", po::value<int>(&width), "Width")
		("height,h", po::value<int>(&height), "Height")
		("nframes,n", po::value<int>(&nFrames), "Number of Frames")
		("fps,f" , po::value<int>(&fps), "Frames per second")
		("outputfile,o", po::value<string>(&outputFile), "Output file")
	;

	// Parse the command line arguments for all supported options
	po::variables_map vm;
	po::store(po::parse_command_line(ac, av, desc), vm);
	po::notify(vm);

	if (vm.count("help")) { cout << desc << endl; return 1; }
	if (!vm.count("inputfile") || !vm.count("width") || !vm.count("height"))
	{
		cerr << "No input file and/or no resolution specified." << endl;
		cout << desc << endl;
		return EXIT_FAILURE;
	}

	// Init OpenGL stuff ///////////////////////////////////
    glutInit(&ac, av);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutCreateWindow(appTitle.c_str());
    glutDisplayFunc(&display);

	FPS = (fps == 0) ? 0 : 1000/fps;
    glutTimerFunc(FPS, update, 0);

	pTexData = (unsigned char*)malloc( width * height * 3 );

	// Init YUV stuff //////////////////////////////////////
	pYuvImage = new YUV::YUVImage(width,height);
	pYuvInStream = new YUV::YUVInStream(inputFile,pYuvImage,nFrames);
	if (vm.count("outputfile")) 
		pYuvOutStream = new YUV::YUVOutStream(outputFile,pYuvImage);

	// Generate LookUp tables
	for (int i = 0; i < 256; i++)
	{
		int v = i - 128;
		LookUp88 [i] = 88*v;
		LookUp182[i] = 182*v;
		LookUp359[i] = 359*v;
		LookUp452[i] = 452*v;
	}

	glutMainLoop();

	// Free ////////////////////////////////////////////////
	pYuvInStream->close();
	if (pYuvOutStream)
	{
		pYuvOutStream->close();
		delete pYuvOutStream;
	}	
	delete pYuvInStream; delete pYuvImage;
	free(pTexData);

	return EXIT_SUCCESS;
}



