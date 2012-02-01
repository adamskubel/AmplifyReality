#include "PageDisplay.hpp"


PageDisplay::PageDisplay()
{
	nextPage = NULL;
	previousPage = NULL;
	currentPage = -1;
}

PageDisplay::~PageDisplay()
{
	delete nextPage;
	delete previousPage;
	Children.clear();
}

void PageDisplay::DoLayout(Rect boundaryRectangle)
{
	LOGD(LOGTAG_INPUT,"Laying out PageDisplay, Rect=[%d,%d,%d,%d]",boundaryRectangle.x,boundaryRectangle.y,boundaryRectangle.width,boundaryRectangle.height);
	float buttonSize = DefaultButtonSize;

	if (buttonSize > boundaryRectangle.width/2.0f || buttonSize > boundaryRectangle.height /2.0f)
	{
		buttonSize = round(min(boundaryRectangle.height /2.0f,boundaryRectangle.width/2.0f));
		LOGD(LOGTAG_INPUT,"Setting page button size to %f",buttonSize);
	}
	
	contentRect = Rect(boundaryRectangle.x,boundaryRectangle.y,boundaryRectangle.width,boundaryRectangle.height-buttonSize);
	LOGD(LOGTAG_INPUT,"PageDisplay ContentRect=[%d,%d,%d,%d]",boundaryRectangle.x,boundaryRectangle.y,boundaryRectangle.width,boundaryRectangle.height);
	
	//Bottom right corner
	Rect nextButtonBoundary = Rect(
		(boundaryRectangle.width-buttonSize) + boundaryRectangle.x,
		(boundaryRectangle.height-buttonSize) + boundaryRectangle.y,
		buttonSize,buttonSize);
	
	//Bottom left corner
	Rect previousButtonBoundary = Rect(boundaryRectangle.x,
		(boundaryRectangle.height-buttonSize) + boundaryRectangle.y,
		buttonSize,buttonSize);

	if (nextPage != NULL && previousPage != NULL)
	{
		nextPage->DoLayout(nextButtonBoundary);
		previousPage->DoLayout(previousButtonBoundary);
	}
	else
	{
		nextPage = new Button(">>",nextButtonBoundary,Colors::Blue);
		nextPage->AddClickDelegate(ClickEventDelegate::from_method<PageDisplay,&PageDisplay::NextPage>(this));

		previousPage = new Button("<<",previousButtonBoundary,Colors::Blue);
		previousPage->AddClickDelegate(ClickEventDelegate::from_method<PageDisplay,&PageDisplay::PreviousPage>(this));
	}

	for (int i=0;i<Children.size();i++)
	{
		Children.at(i)->DoLayout(contentRect);
	}

	AdjustButtons();
	layoutDefined = true;
}

UIElement * PageDisplay::GetElementAt(Point2i point)
{
	if (!isVisible || currentPage < 0)
		return NULL;


	UIElement * button = nextPage->GetElementAt(point);
	if (button != NULL)
		return button;
	button = previousPage->GetElementAt(point);
	if (button != NULL)
		return button;

	return Children.at(currentPage)->GetElementAt(point);
}


void PageDisplay::AddChild(GraphicalUIElement * page)
{
	if (currentPage == -1)
		currentPage = 0;

	Children.push_back(page);

	if (layoutDefined)
	{	
		page->DoLayout(contentRect);
		AdjustButtons();
	}
}

void PageDisplay::NextPage(void * sender, EventArgs args)
{
	if (currentPage + 1 < Children.size())
	{
		currentPage++;
		AdjustButtons();
	}
}

void PageDisplay::PreviousPage(void * sender, EventArgs args)
{
	if (currentPage - 1 >= 0)
	{
		currentPage--;
		AdjustButtons();
	}
}

void PageDisplay::AdjustButtons()
{
	if (nextPage != NULL && previousPage != NULL)
	{
		if (currentPage + 1 < Children.size())
		{
			nextPage->SetVisible(true);
		}
		else
		{
			nextPage->SetVisible(false);
		}

		if (currentPage - 1 >= 0)
		{
			previousPage->SetVisible(true);
		}
		else
		{
			previousPage->SetVisible(false);
		}
	}
}




void PageDisplay::SetPage(int pageNumber)
{
	if (pageNumber < Children.size())
		currentPage = pageNumber;
	else		
		LOGW(LOGTAG_INPUT,"Cannot set page number to %d, only %d children",pageNumber,Children.size());
}

void PageDisplay::Draw(Mat * rgbaImage)
{
	if (!IsVisible() || !layoutDefined)
		return;

	//LOGD(LOGTAG_INPUT,"Drawing page display, pagenum=%d",currentPage);
	if (nextPage != NULL && previousPage != NULL)
	{
		if (nextPage->IsVisible())
			nextPage->Draw(rgbaImage);
		if (previousPage->IsVisible())
			previousPage->Draw(rgbaImage);
	}
	if (currentPage >= 0)
	{
		if (Children.at(currentPage)->IsVisible())
			Children.at(currentPage)->Draw(rgbaImage);
	}
}


