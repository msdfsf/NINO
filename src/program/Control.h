#pragma once

#include <stddef.h>
#include "ControlEvent.h"
#include "StringAlignment.h"
#include "DisplayOrder.h"
#include <list>
#include "OverflowType.h"

typedef long long CTRL_PARAM;

class Control {

	public:

		enum ProcessMessage {
			
			PM_OK,
			PM_PREVENT_EVENT

		};

		static const int SCROLL_STEP = 10; // step in pixels of each scroll (mouse wheel) tick
		
		static Control* window;

		static Control* focusedControl;

		Control();
		Control(Control* parent);
		Control(Control* parent, int x, int y, int width, int height);
		Control(Control* parent, double x, double y, double width, double height);
		
		virtual void draw();

		virtual int processMessage(
			ControlEvent::ControlEvent controlEvent, 
			CTRL_PARAM paramA,
			CTRL_PARAM paramB
		);

		void addControl(Control* control);
		void pushControlFront(Control* control);

		Control* parent;

		Control** childrens;
		Control** childrensDrawLast;
		static std::list<Control*> childrensAboveAll;

		int childrenCount = 0;

		int id; // index in the parents array

		DisplayOrder::DisplayOrder displayOrder;

		int x;
		int y;

		int width;
		int height;

		int marginTop;
		int marginBottom;
		int marginLeft;
		int marginRight;

		int paddingTop;
		int paddingBottom;
		int paddingLeft;
		int paddingRight;

		int actualColor;
		int actualBackColor;

		int color;
		int backgroundColor;

		int selectFrontColor;
		int selectBackColor;

		int hoverFrontColor;
		int hoverBackColor;

		int borderColor;
		int borderLeftWidth;
		int borderRightWidth;
		int borderTopWidth;
		int borderBottomWidth;

		int innerBorderBevelWidth;
		int outerBorderBevelWidth;

		char* borderTitleText;
		int borderTitleLength;
		int borderTitlePadding;
		int borderTitleFontSize;

		char* text;
		int textLength;
		int fontSize;
		StringAlignment::StringAlignment textAlignment;

		int cursor;

		int selected;
		int visible;

		int focused;

		int draggable;
		int droppable;

		int passMouseEvents;
		int breakEventsChain;

		int trackAnyClick;

		int scrollOffsetY = 0;
		
		int overflowType = 0;
		int overflowBehaviour = 0;
		
		// used in draw to send info to children about parent overflow
		int overflowTop = 0;
		int overflowBottom = 0;

		// is calculated in mouse move event, so, if default event processing function is used
		// can be safely used in mouse move event if needed
		int mouseInBounds = 0;

		void setX(int x);
		void setY(const int y);

		void setX(double x);
		void setY(double y);

		void setWidth(int width);
		void setHeight(int height);

		void setWidth(double width);
		void setHeight(double height);

		void setPadding(int width);

		void setBorderWidth(int borderWidth);

		void setBorderBevelWidth(int width);

		void setColor(int color);
		void setBackColor(int color);
		void setColor(int color, int backColor);

		void setText(char* text, const int textLen);
		void setText(wchar_t* text, const int textLen);

		void setBorderTitle(char* text, const int len);

		void setCursor(int cursor);

		void focus();

		void select(int redraw = 0);
		void unselect(int redraw = 0);
		void toggleSelect();

		void show(int redraw = 0);
		void hide(int redraw = 0);

		void getRelativeCoords(int* x, int* y);
		void getRelativeCoordsWithBorders(int* x, int* y);

		int getInBoxX();
		int getInBoxY();

		int getInBoxWidth();
		int getInBoxHeight();

		int swapChildrens(const int idA, const int idB);

		int scrollY(int value);
		int scrollY(const int value, const int fromIdx);

		int isInBounds(const int x, const int y);

		// events
		void (*eMouseClick) (Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) = NULL;
		void (*eMouseScroll) (Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) = NULL;
		void (*eMouseMove) (Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) = NULL;
		void (*eMouseDblClick) (Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) = NULL;
		void (*eMouseUp) (Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) = NULL;
		void (*eMouseDown) (Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) = NULL;

		void (*eDragStart) (Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) = NULL;
		void (*eDragEnd) (Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) = NULL;
		void (*eDrag) (Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) = NULL;
		void (*eDrop) (Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) = NULL;

		void (*eCharInput) (Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) = NULL;

		void (*eChange) (Control* source, CTRL_PARAM paramA, CTRL_PARAM paramB) = NULL;

		~Control();

	private:

};