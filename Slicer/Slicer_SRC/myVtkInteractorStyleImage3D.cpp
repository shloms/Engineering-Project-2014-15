#include "myVtkInteractorStyleImage3D.h"
#include <vtkRendererCollection.h>
#include <vtkPolyData.h>
#include <vtkCellData.h>
#include <vtkRegularPolygonSource.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include "constants.h"
#include <vtkDataSetMapper.h>
#include <vtkStructuredPoints.h>
#include <vtkCell.h>
#include <vtkPointData.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkExtractVOI.h>
#include <vtkStructuredPointsWriter.h>
#include <vtkCellPicker.h>
#include <vtkPointPicker.h>
#include <vtkImageMapToColors.h>
#include <vtkImageOpenClose3D.h>
#include <vtkPropPicker.h>
#include <vtkImageEuclideanDistance.h>
#include <algorithm>
#include <vtkAppendPolyData.h>
#include <vtkCleanPolyData.h>
#include <vtkDijkstraGraphGeodesicPath.h>
#include <vtkPolyDataWriter.h>
#include <vtkCamera.h>
#include <sstream>


myVtkInteractorStyleImage3D::myVtkInteractorStyleImage3D()
{}

void myVtkInteractorStyleImage3D::SetStatusMapper(vtkTextMapper* statusMapper) {
	_StatusMapper = statusMapper;
}

void myVtkInteractorStyleImage3D::Initialize(std::string outputName, vtkSphereSource* sphereCursor){
	_drawSize = DEFAULT_DRAW_SIZE;
	_lal = LeapAbstractionLayer::getInstance();
	_sphereCursor = sphereCursor;
	_outputName = outputName;
	_hfMode = false;
	_rotLock = true;
	_currSource = -1;
	//Start movement:
	this->StartRotate();
	cout << "3D Interactor iniitiated." << std::endl;
}

void myVtkInteractorStyleImage3D::LoadFromFile(){
}

void myVtkInteractorStyleImage3D::ResetAll(){
	vtkRenderer* ren = this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
	vtkPolyDataMapper* mapper = (vtkPolyDataMapper*)ren->GetActors()->GetLastActor()->GetMapper();
	vtkPolyData* mesh = mapper->GetInput();
	vtkPointData* pd = mesh->GetPointData();
	vtkIntArray* scalars = (vtkIntArray*)pd->GetScalars();
	for (int i = 0; i < mesh->GetNumberOfPoints();i++){
		scalars->SetValue(i, NOT_ACTIVE);
	}
	scalars->Modified();
	mesh->Modified();
	pd->Modified();
	mapper->Update();
}

void myVtkInteractorStyleImage3D::WriteToFile() {
	cout << "Writing segmentation to file..." << endl;
	vtkRenderer* ren = this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
	vtkPolyDataMapper* mapper = (vtkPolyDataMapper*)ren->GetActors()->GetLastActor()->GetMapper();
	vtkPolyData* mesh = mapper->GetInput();
	vtkSmartPointer<vtkPolyDataWriter> writer =
		vtkSmartPointer<vtkPolyDataWriter>::New();
	writer->SetInputData(mesh);
	writer->SetFileName(_outputName.c_str());
	writer->Write();
	cout << "Done." << endl;
}

typedef void(myVtkInteractorStyleImage3D::*workerFunction)();

void myVtkInteractorStyleImage3D::doSegment() {	
}

void myVtkInteractorStyleImage3D::OnLeftButtonDown()
{
	std::cout << "Pressed left mouse button." << std::endl;
}

void myVtkInteractorStyleImage3D::OnRightButtonDown()
{
	std::cout << "Pressed left mouse button." << std::endl;
	// Forward events
	vtkInteractorStyleJoystickCamera::OnRightButtonDown();
}

void myVtkInteractorStyleImage3D::OnRightButtonUp()
{
}

void myVtkInteractorStyleImage3D::OnLeftButtonUp()
{
}

void myVtkInteractorStyleImage3D::OnTimer(){
	//cout << "Got a leap event!" << endl;
	// render
	int * winSize = Interactor->GetRenderWindow()->GetSize();
	if (this->Interactor->GetShiftKey()){
		Interactor->SetEventPosition(-2*_lal->getX() + winSize[0] / 2, winSize[1] / 2 + 1.5*_lal->getZ());
	}
	else{
		Interactor->SetEventPosition(winSize[0] / 2, winSize[1] / 2);
	}
	// Trying to set cursor:
	//cout << "Trying to set cursor:" << endl;

	//_sphereCursor->SetCenter(
	//	0,
	//	1.0,
	//	0);
	//cout << _lal->getX()<< ":"
	//	<< _lal->getY()-200 << ":"
	//	<< _lal->getZ() << endl;
	//cout << "Cursor set." << endl;
	//_sphereCursor->Modified();
	//Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActors()->GetLastActor()->GetMapper()->Update();
	// Delegate
	vtkInteractorStyleJoystickCamera::OnTimer();
	
}

void myVtkInteractorStyleImage3D::OnKeyUp() {}
void myVtkInteractorStyleImage3D::OnMouseMove(){
	vtkInteractorStyleJoystickCamera::OnMouseMove();
}
void myVtkInteractorStyleImage3D::OnKeyDown() {
	std::string key = this->GetInteractor()->GetKeySym();
	if (key.compare("Up") == 0) {
		cout << "Up arrow key was pressed." << endl;
	}
	else if (key.compare("Down") == 0) {
		cout << "Down arrow key was pressed." << endl;
	}
	else if (key.compare("1") == 0) {
		cout << "Draw size was changed to " << _drawSize - 1 << endl;
	}
	else if (key.compare("2") == 0) {
		cout << "Draw size was changed to " << _drawSize + 1 << endl;
	}
	else if (key.compare("o") == 0) {
		cout << "Orientation key was pressed." << endl;
	}
	else if (key.compare("f") == 0) {
		_hfMode = !_hfMode;
		cout << "Hands free mode: " << (_hfMode ? "ON" : "OFF") << endl;
	}
	else if (key.compare("s") == 0) {
		WriteToFile();
	}
	else if (key.compare("a") == 0) {
		cout << "a pressed!" << endl;
		if (_rotLock){
			_rotLock = false;
			int x = this->Interactor->GetEventPosition()[0];
			int y = this->Interactor->GetEventPosition()[1];
			this->FindPokedRenderer(x, y);
			if (this->CurrentRenderer == NULL)
			{
				cout << "Renderer is null" << endl;
				return;
			}
			this->GrabFocus(this->EventCallbackCommand);
			this->StartRotate();
		}
		else{
			this->EndRotate();
			if (this->Interactor)
			{
				this->ReleaseFocus();
			}
			_rotLock = true;
		}
	}
	else if (key.compare("r") == 0) {
		this->ResetAll();
	}
	else if (key.compare("l") == 0) {
		this->LoadFromFile();
	}
	else if (key.compare("space") == 0) {
		int* clickPos = this->GetInteractor()->GetEventPosition();
		vtkSmartPointer<vtkPropPicker>  picker =
			vtkSmartPointer<vtkPropPicker>::New();
		vtkRenderer* ren = this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
		picker->Pick(clickPos[0], clickPos[1], 0, ren);
		double* pos = picker->GetPickPosition();
		vtkPolyDataMapper* mapper = (vtkPolyDataMapper*)ren->GetActors()->GetLastActor()->GetMapper();
		vtkPolyData* mesh = mapper->GetInput();
		vtkPointData* pd = mesh->GetPointData();
		vtkIntArray* scalars = (vtkIntArray*)pd->GetScalars();

		vtkIdType id = mesh->FindPoint(pos);
		cout << "The Point Is: " << id << endl;
		cout << "Number of components is: " << scalars->GetNumberOfComponents() << endl;
		if (id > -1 && _currSource == -1){
			_currSource = id;
			cout << "Start set." << endl;
		}
		else if (id > -1 && _currSource != -1){
			//Make a line connection.
			cout << "End set." << endl;
			vtkSmartPointer<vtkDijkstraGraphGeodesicPath> dijkstra =
				vtkSmartPointer<vtkDijkstraGraphGeodesicPath>::New();
			dijkstra->SetInputData(mesh);
			dijkstra->SetStartVertex(_currSource);
			dijkstra->SetEndVertex(id);
			dijkstra->Update();
			cout << "Found Shortest path!" << endl;
			vtkPolyData* path = dijkstra->GetOutput();
			cout << "Num. of points: " <<path->GetNumberOfPoints() << endl;
			for (int i = 0; i < path->GetNumberOfPoints(); i++){
				scalars->SetComponent(mesh->FindPoint(path->GetPoint(i)),0,FOREGROUND);
			}
			_currSource = -1;
		}
		if (id > -1){
			scalars->SetComponent(id, 0, FOREGROUND);
		}
		scalars->Modified();
		mesh->Modified();
		pd->Modified();
		mapper->Update();
	}
	else if (key.compare("m") == 0) {
		cout << "m pressed event!" << endl;
	}
	// forward event
}

void myVtkInteractorStyleImage3D::OnMouseWheelForward() {
	// don't forward events, otherwise the image will be zoomed
	// in case another interactorstyle is used (e.g. trackballstyle, ...)
	// vtkInteractorStyleImage::OnMouseWheelForward();
}
void myVtkInteractorStyleImage3D::OnMouseWheelBackward() {
	//std::cout << "Scrolled mouse wheel backward." << std::endl;
	// don't forward events, otherwise the image will be zoomed
	// in case another interactorstyle is used (e.g. trackballstyle, ...)
	// vtkInteractorStyleImage::OnMouseWheelBackward();
}

myVtkInteractorStyleImage3D::~myVtkInteractorStyleImage3D(){
}

vtkStandardNewMacro(myVtkInteractorStyleImage3D);
