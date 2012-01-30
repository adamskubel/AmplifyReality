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
	float buttonSize = DefaultButtonSize;

	if (buttonSize > boundaryRectangle.width/2.0f || buttonSize > boundaryRectangle.height /2.0f)
	{
		buttonSize = round(min(boundaryRectangle.height /2.0f,boundaryRectangle.width/2.0f));
		LOGD(LOGTAG_INPUT,"Setting page button size to %f",buttonSize);
	}

	
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

	contentRect = Rect(boundaryRectangle.x,boundaryRectangle.y,boundaryRectangle.width,boundaryRectangle.height-buttonSize);
	for (int i=0;i<Children.size();i++)
	{
		Children.at(i)->DoLayout(contentRect);
	}

}

UIElement * PageDisplay::GetElementAt(Point2i point)
{
	if (!isVisible)
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
	page->DoLayout(contentRect);
	Children.push_back(page);
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
	if (currentPage + 1 < Children.size())
	{
		nextPage->SetVisible(false);
	}
	else
	{
		nextPage->SetVisible(true);
	}

	if (currentPage - 1 >= 0)
	{
		previousPage->SetVisible(false);
	}
	else
	{
		previousPage->SetVisible(true);
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


