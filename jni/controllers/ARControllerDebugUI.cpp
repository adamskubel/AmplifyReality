#include "ARController.hpp"

ARControllerDebugUI::ARControllerDebugUI(Engine * engine, Point2i position) : PageDisplay()
{	
	int padding = 20;
	myGrid = new GridLayout(Size2i(100,100),Size2i(4,3));
	
	rotationLabel = new DataDisplay("%3.2f",Point2i(0,0),Size2i(0,0),Colors::Red);	
	translationLabel = new DataDisplay("%3.2f",Point2i(0,0),Size2i(0,0),Colors::Red);

	myGrid->AddChild(rotationLabel,Point2i(0,1));
	myGrid->AddChild(translationLabel,Point2i(3,1));

	Mat oneMatrix = Mat::ones(3,1,CV_64F);
	
	Mat oneMatrix2 = Mat::ones(1,3,CV_64F);
		
	rotationLabel->SetData(&oneMatrix);
	rotationLabel->SetData(&oneMatrix2);

	AddChild(myGrid);

	Button * testButton = new Button("Page2 lol",Rect(),Colors::MediumSeaGreen);
	
	AddChild(testButton);

	
	SetPage(0);

	DoLayout(Rect(0,0,engine->imageWidth,engine->imageHeight));
	
}
//
//ARControllerDebugUI::~ARControllerDebugUI()
//{
//	delete myGrid;
//}