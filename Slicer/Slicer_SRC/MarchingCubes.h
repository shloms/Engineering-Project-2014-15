#ifndef __MARCHING_CUBES__
#define __MARCHING_CUBES__

#include <vtkVersion.h>
#include <vtkSmartPointer.h>
#include <vtkMarchingCubes.h>
#include <vtkVoxelModeller.h>
#include <vtkSphereSource.h>
#include <vtkImageData.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageMapToColors.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>

#include <vtkStructuredPoints.h>
#include "constants.h"
#include <vtkImageViewer2.h>
#include <vtkImageMaskBits.h>
#include <vtkCamera.h>


/** This class is reponsible for creating the mesh using MarchingCubes algorithm*/
class MarchingCubes {
	
public:
	MarchingCubes(vtkStructuredPoints* selection);
	~MarchingCubes();
	
private:
	vtkSmartPointer<vtkMarchingCubes>  _surface;
	vtkSmartPointer<vtkRenderWindowInteractor> _renderWindowInteractor;
	vtkSmartPointer<vtkPolyDataMapper> _mapper;
	vtkSmartPointer<vtkActor>          _actor;
	vtkSmartPointer<vtkImageMaskBits>  _mask;

	vtkSmartPointer<vtkImageViewer2>   _viewer;
	vtkStructuredPoints* _selection;

};




#endif