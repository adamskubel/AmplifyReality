#include "ARController.hpp"

ARControllerDebugUI::ARControllerDebugUI(Engine * engine, Point2i position) : PageDisplay()
{	
	int padding = 20;
	myGrid = new GridLayout(Size2i(4,3));
	
	rotationLabel = new DataDisplay("%3.2lf",Colors::Red);	
	translationLabel = new DataDisplay("%3.2lf",Colors::Blue);


	myGrid->AddChild(rotationLabel,Point2i(0,1));
	myGrid->AddChild(translationLabel,Point2i(0,2));

	Mat oneMatrix = Mat::ones(1,3,CV_64F);	
	Mat oneMatrix2 = Mat::zeros(1,3,CV_64F);
		
	rotationLabel->SetData(&oneMatrix);
	translationLabel->SetData(&oneMatrix2);

	certaintyIndicator = new CertaintyIndicator(0);
	myGrid->AddChild(certaintyIndicator,Point2i(3,2));
	certaintyIndicator->SetMaxRadius(20); //Override radius set by grid. Should have a true "max radius" field to avoid this.

	AddChild(myGrid);

	SetPage(0);

	LOGD(LOGTAG_INPUT,"Laying out DebugUI");

	DoLayout(Rect(0,0,engine->imageWidth,engine->imageHeight));

	
	LOGD(LOGTAG_INPUT,"DebugUI Layout complete");
	
	


}

void ARControllerDebugUI::SetRotation(Mat * mat)
{
	rotationLabel->SetData(mat);
}
void ARControllerDebugUI::SetTranslation(Mat * mat)
{
	translationLabel->SetData(mat);
}

void ARControllerDebugUI::SetPositionCertainty(float certainty)
{
	certaintyIndicator->SetCertainty(certainty);
}