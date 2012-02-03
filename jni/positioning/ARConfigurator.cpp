#include "ARConfigurator.hpp"


ARConfigurator::ARConfigurator(Engine * engine) 
{
	SetDefaults();

	int padding = 20;
	DoLayout(Rect(padding,padding,engine->imageWidth-padding*2, engine->imageHeight - padding*2));

	GridLayout * myGrid = new GridLayout(Size2i(100,100),Size2i(3,2));
	
	NumberSpinner * positionSpinner = new NumberSpinner("T-Alpha",PositionFilterAlpha,0.05f,"%2.2f");
	positionSpinner->AddValueChangedDelegate(NumberSpinnerEventDelegate::from_method<ARConfigurator,&ARConfigurator::PositionFilterAlphaChanged>(this));
	myGrid->AddChild(positionSpinner,Point2i(0,0));
	positionSpinner->SetMaximum(1.0f);
	positionSpinner->SetMinimum(0.0f);

	NumberSpinner * rotationSpinner = new NumberSpinner("R-Alpha",RotationFilterAlpha,0.05f,"%2.2f");
	rotationSpinner->AddValueChangedDelegate(NumberSpinnerEventDelegate::from_method<ARConfigurator,&ARConfigurator::RotationFilterAlphaChanged>(this));
	myGrid->AddChild(rotationSpinner,Point2i(1,0));
	rotationSpinner->SetMaximum(1.0f);
	rotationSpinner->SetMinimum(0.0f);

	NumberSpinner * numberSpinner2 = new NumberSpinner("MinFPScore",MinFinderPatternScore,10.0f,"%3.1f");
	numberSpinner2->AddValueChangedDelegate(NumberSpinnerEventDelegate::from_method<ARConfigurator,&ARConfigurator::MinimumFinderPatternScoreChanged>(this));
	myGrid->AddChild(numberSpinner2,Point2i(2,0));
	numberSpinner2->SetMinimum(100.0f);
	numberSpinner2->SetMaximum(300.0f);

	NumberSpinner * minAlignmentSpinner = new NumberSpinner("MinAlignScore",MinAlignmentScore,10.0f,"%3.1f");
	minAlignmentSpinner->AddValueChangedDelegate(NumberSpinnerEventDelegate::from_method<ARConfigurator,&ARConfigurator::MinimumAlignmentPatternScoreChanged>(this));
	myGrid->AddChild(minAlignmentSpinner,Point2i(2,1));
	minAlignmentSpinner->SetMinimum(100.0f);
	minAlignmentSpinner->SetMaximum(300.0f);
	
	
	engine->inputHandler->AddGlobalTouchDelegate(TouchEventDelegate::from_method<ARConfigurator,&ARConfigurator::GlobalTouchEvent>(this));	


	LOGD(LOGTAG_INPUT,"Adding grid to page display");
	AddChild(myGrid);

	GridLayout * nextPageGrid = new GridLayout(Size2i(100,100),Size2i(3,2));
	
	NumberSpinner * testSpinner = new NumberSpinner("Cats",0.1f,0.05f,"%2.2f");
	nextPageGrid->AddChild(testSpinner,Point2i(0,0));

	AddChild(nextPageGrid);

	SetVisible(false);
}

void ARConfigurator::SetDefaults()
{
	PositionFilterAlpha = 0.9f;	
	RotationFilterAlpha = 0.9f;	
	MinFinderPatternScore = 190;
	MinAlignmentScore = 180;
}

void ARConfigurator::PositionFilterAlphaChanged(void * sender, NumberSpinnerEventArgs args)
{
	PositionFilterAlpha = args.NewValue;
}

void ARConfigurator::RotationFilterAlphaChanged(void * sender, NumberSpinnerEventArgs args)
{
	RotationFilterAlpha = args.NewValue;
}


void ARConfigurator::MinimumFinderPatternScoreChanged(void * sender, NumberSpinnerEventArgs args)
{
	MinFinderPatternScore = args.NewValue;
}

void ARConfigurator::MinimumAlignmentPatternScoreChanged(void * sender, NumberSpinnerEventArgs args)
{
	MinAlignmentScore = args.NewValue;
}


void ARConfigurator::ToggleVisibility(void * sender, EventArgs args)
{
	isVisible = !isVisible;
}

void ARConfigurator::GlobalTouchEvent(void* sender, TouchEventArgs args)
{
	LOGI(LOGTAG_MAIN,"Received touch event: %d", args.InputType);
	switch (currentDrawMode)
	{
	case(DrawModes::BinaryImage):
		currentDrawMode = DrawModes::ColorImage;
		break;
	case(DrawModes::GrayImage):
		currentDrawMode = DrawModes::BinaryImage;
		break;
	case(DrawModes::ColorImage):
		currentDrawMode = DrawModes::GrayImage;
		break;
	}
}




