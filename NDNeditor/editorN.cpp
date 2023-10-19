#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <ncurses.h>
#include "editorN.h"

using namespace std;

FileContainer curFile; // object of the File Container class

#define CURLINEPOS curFile.curLinePos
#define CURCOLPOS curFile.curColPos

#define INFILENAME curFile.fileName

#define CURSORROW curFile.cursorRow
#define CURSORCOL curFile.cursorCol

#define DISPLINECTR curFile.dispLineCtr
#define DISPCOLCTR curFile.dispColCtr

#define LINESARRAY curFile.fileContent

#define CURLINE LINESARRAY.at(DISPLINECTR - 1)
#define CURCHAR CURLINE.at(DISPCOLCTR - 1)				  // curFile.fileContent.at(curFile.dispLineCtr - 1).at(curFile.dispColCtr -1)
#define CURLINESIZE LINESARRAY.at(DISPLINECTR - 1).size() // curFile.fileContent.at(curFile.dispLineCtr - 1).size()

#define FILETOTLINES curFile.filetotlines

/*
#define CURLINE         curFile.fileContent.at(curFile.dispLineCtr - 1)
#define CURCHAR         curFile.fileContent.at(curFile.dispLineCtr - 1).at(curFile.dispColCtr -1)
#define CURLINESIZE     curFile.fileContent.at(curFile.dispLineCtr - 1).size()
*/

unsigned int scrnWidth, prevLineSize;
int scrnHeight, touched;

// following two variables are for using in debugging
std::ostringstream myStrStream;
std::string dbgMsg;

WINDOW *mainscrwin, *hdrwin, *ftrwin;

//
// Constructor for the FileContainer class.
//

FileContainer::FileContainer()
{
	curLinePos = 0;	  // This indicates the Line in the vector from which we have to
					  // start redrawing the screen in case of vertical scrolling
	filetotlines = 0; // Total lines in the file

	curColPos = 0;	 // This indicates the Column in the vector from which we have to
					 // start redrawing the screen in case of horizontal scrolling
	cursorRow = 1;	 // This is used to track the row position of the cursor on the screen
	cursorCol = 1;	 // This is used to track the col position of the cursor on the screeen
	dispLineCtr = 1; // This is used to display the line# in the status line at the bottom
	dispColCtr = 1;	 // This is used to display the col# in the status line at the bottom
	fileName = "";
}
//
// Initializes the # define variables
//
void varInit()
{
	if (FILETOTLINES == 0)
	{
		LINESARRAY.push_back("");
		FILETOTLINES = LINESARRAY.size();
	}

	CURLINEPOS = 0;
	CURCOLPOS = 0;

	CURSORROW = 1;
	CURSORCOL = 1;

	DISPLINECTR = 1;
	DISPCOLCTR = 1;

	touched = 0;
}
//
// opens the file and displays contents of file
//
int fileOpenRead()
{
	string line;
	int rtn;

	ifstream infile;

	while (1)
	{
		infile.open(INFILENAME.c_str());
		if (infile.is_open())
		{
			break; // file open successful; break out of the infinite loop
		}
		else
		{

			rtn = dispError("Error opening file, press space bar to enter again.", "Any other key to return to editor...");
			if (rtn == 32) // space bar pressed; so let the user enter another filename
				getFileName(INFILENAME);
			else
				return 0; // user wants to return to the editor
		}
	}

	LINESARRAY.clear();
	while (std::getline(infile, line))
		LINESARRAY.push_back(line);

	infile.close();

	FILETOTLINES = LINESARRAY.size();

	varInit();

	return 1;
}
//
// writes the changes made by user to file
//
int fileSave()
{

	// dispError("fileSave ", INFILENAME.c_str());

	ofstream outfile(INFILENAME.c_str());
	if (!outfile.is_open())
	{
		dispError("Error saving to file", "Press any key...");
		return 0;
	}
	for (unsigned int i = 0; i < FILETOTLINES; i++)
		outfile << LINESARRAY.at(i) << endl;
	outfile.close();
	touched = 0;
	return 1;
}
//
// constructs the initial mainscrn, header and footer windows
//
void initWin()
{
	//
	// Ncurses stores window size in LINES and COLS
	// So we will use them to set scrnHeight and scrnWidth
	//

	scrnHeight = LINES - 4; // 2 lines reserved for horizontal border, 2 lines reserved for hdr and footer windows

	if (scrnHeight < 4)
		scrnHeight = 4;

	scrnWidth = COLS - 3; // 2 cols reserved for vertical border

	if (scrnWidth < 3)
		scrnWidth = 3;

	int height, width;

	height = LINES - 2;
	width = COLS;

	mainscrwin = newwin(height, width, 1, 0);

	hdrwin = newwin(1, width, 0, 0);
	ftrwin = newwin(1, width, LINES - 1, 0);

	keypad(mainscrwin, TRUE);

	CURSORROW = 1;
	CURSORCOL = 1;

	wmove(mainscrwin, 1, 1);
	box(mainscrwin, ACS_VLINE, ACS_HLINE);
}
//
// starts up the ncurse window
//
int maininit()
{
	initscr();
	cbreak();
	nonl();
	noecho();
	start_color(); /* Start color 			*/

	init_pair(4, COLOR_RED, COLOR_BLACK);
	init_pair(3, COLOR_CYAN, COLOR_BLACK);
	init_pair(2, COLOR_YELLOW, COLOR_BLACK);
	init_pair(1, COLOR_GREEN, COLOR_BLACK);

	initWin();

	return 0;
}
//
// Closes the ncurses window when program terminates
//
void mainexit()
{

	endwin();
}
//
// repaints the screen every time a scrolling or a editing functionality occurs
//
int showScreen()
{
	int lctr = 1, remainingLines;
	unsigned int lsize;

	remainingLines = FILETOTLINES - CURLINEPOS;

	string currentLine;
	wattron(mainscrwin, COLOR_PAIR(1));
	wattron(mainscrwin, A_BOLD);
	wclear(mainscrwin);
	// wclear(hdrwin) ;

	// dispError("ShowScreen 1", "Press any key to continue...");

	while (lctr <= scrnHeight)
	{
		currentLine = LINESARRAY.at(CURLINEPOS + lctr - 1);
		lsize = currentLine.size();

		wmove(mainscrwin, lctr, 1);

		if (lsize > scrnWidth)
		{
			unsigned int rlen;
			rlen = lsize - CURCOLPOS; // remaining length

			// if (DISPCOLCTR <= lsize )
			// show a substring. Length of substring will be smaller of 'remaining length' and scrnWidth
			waddstr(mainscrwin, currentLine.substr(CURCOLPOS, (rlen > scrnWidth ? scrnWidth : rlen)).c_str());
		}
		else					   // line is shorter than screen width
			if (CURCOLPOS < lsize) // add a substring of current line to window
				waddstr(mainscrwin, currentLine.substr(CURCOLPOS).c_str());

		/*
		myStrStream.str("");
		myStrStream << " CURLINE : " << DISPLINECTR << " CURCOL : " << DISPCOLCTR << " char : " << CURCHAR << "CURINESIZE : " << CURLINESIZE ;
		myStrStream << " CURLINE : " << lctr << " CURCOL : " << DISPCOLCTR << " string : " << currentLine.substr(CURCOLPOS).c_str()  ;
		dbgMsg = myStrStream.str();
		printMsg(dbgMsg) ;
		*/

		lctr++;
		if (lctr > remainingLines)
			break;
	}

	box(mainscrwin, ACS_VLINE, ACS_HLINE);
	wattroff(mainscrwin, COLOR_PAIR(1));
	wattroff(mainscrwin, A_BOLD);
	refreshwins();
	return 0;
}

void printMsg(string msgStr)
{

	wmove(hdrwin, 0, 0);
	wclear(hdrwin);
	wprintw(hdrwin, msgStr.c_str());
	wrefresh(hdrwin);
	wgetch(hdrwin);
	wmove(mainscrwin, CURSORROW, CURSORCOL);
	wrefresh(mainscrwin);
}
//
// paints the screen accordingly
//
void refreshwins()
{

	wclear(hdrwin);

	wattron(hdrwin, COLOR_PAIR(4));
	wattron(hdrwin, A_BOLD);
	wmove(hdrwin, 0, 1);
	wprintw(hdrwin, "F1 - Menu");
	wmove(hdrwin, 0, scrnWidth / 2.5);
	wprintw(hdrwin, "My Editor!");
	wmove(hdrwin, 0, scrnWidth - 7);
	wprintw(hdrwin, "F9 - Exit");
	wattroff(hdrwin, COLOR_PAIR(4));
	wattroff(hdrwin, A_BOLD);
	wrefresh(hdrwin);

	wclear(ftrwin);

	wattron(ftrwin, COLOR_PAIR(2));
	wattron(ftrwin, A_BOLD);

	wprintw(ftrwin, "File: ");
	wprintw(ftrwin, INFILENAME.c_str());
	wprintw(ftrwin, "    Total Lines : %d", FILETOTLINES);

	wmove(ftrwin, 0, scrnWidth - 17);
	wprintw(ftrwin, "Line : %d Col : %d", DISPLINECTR, DISPCOLCTR);

	wattroff(ftrwin, A_BOLD);
	wattroff(ftrwin, COLOR_PAIR(2));

	wrefresh(ftrwin);
	wmove(mainscrwin, CURSORROW, CURSORCOL);
	wrefresh(mainscrwin);
}
//
// inserts the character in the appropriate place
//
void insert_char(char ch)
{

	CURLINE.insert((DISPCOLCTR - 1) > 0 ? (DISPCOLCTR - 1) : 0, 1, ch);

	touched = 1;

	DISPCOLCTR++;
	CURSORCOL++;
	if (CURSORCOL > scrnWidth)
	{
		CURCOLPOS++;
		CURSORCOL = scrnWidth;
	}

	/*
	myStrStream.str("");
	myStrStream << " CURLINE : " << DISPLINECTR << " CURCOL : " << DISPCOLCTR << " char : " << CURCHAR << "CURINESIZE : " << CURLINESIZE ;
	myStrStream << " CURLINE : " << DISPLINECTR << " CURCOL : " << DISPCOLCTR << " FILETOTLINES : " << FILETOTLINES  ;
	dbgMsg = myStrStream.str();
	printMsg(dbgMsg) ;
	*/

	showScreen();
}
//
// If the user presses enter at a certain point in the line then the line splits text into the next line
//
void splitLine()
{

	const string tempStr = CURLINE.substr(DISPCOLCTR - 1);

	CURLINE.erase((DISPCOLCTR - 1) > 0 ? (DISPCOLCTR - 1) : 0, tempStr.size());

	// If cursor is on lastine or if there is only
	// one line in the file then append else insert

	if (DISPLINECTR == FILETOTLINES || FILETOTLINES == 1)
	{
		DISPLINECTR++;
		LINESARRAY.push_back(tempStr);
	}
	else
	{
		DISPLINECTR++;
		LINESARRAY.insert(LINESARRAY.begin() + DISPLINECTR - 1, tempStr);
	}

	CURCOLPOS = 0;
	DISPCOLCTR = 1;
	CURSORCOL = 1;
	CURSORROW++;
	CURSORROW = CURSORROW > scrnHeight ? scrnHeight : CURSORROW;
	FILETOTLINES = LINESARRAY.size();
	touched = 1;
}
//
// If the user presses backspace and the cursor is at the very beginning
// of the line then the the previous line is brought up
// or If the user presses delete and the cursor is at the very end then
// the text from line below is joined if it exists
//
void joinLine(int line2index)
{

	CURLINE.append(LINESARRAY.at(line2index - 1));
	LINESARRAY.erase(LINESARRAY.begin() + line2index - 1);
	FILETOTLINES = LINESARRAY.size();
	touched = 1;
}
//
// removes the character and moves the cursor into the space the character was previously in
//
void delete_char()
{

	if (CURLINESIZE < DISPCOLCTR)
	{
		if (FILETOTLINES > 1)
			if ((DISPLINECTR + 1) <= FILETOTLINES)
				joinLine(DISPLINECTR + 1);
	}
	else if (CURLINESIZE > 0)
		CURLINE.erase((DISPCOLCTR - 1) > 0 ? (DISPCOLCTR - 1) : 0, 1);
	else if (CURLINESIZE == 0)
	{
		if (FILETOTLINES > 1)
		{
			LINESARRAY.erase(LINESARRAY.begin() + DISPLINECTR - 1); // if all characters have been deleted then delete the line
			FILETOTLINES = LINESARRAY.size();
		}
	}
	touched = 1;

	showScreen();
}
//
// deletes the previous character
//
void backspace()
{
	if (DISPCOLCTR == 1 && DISPLINECTR != 1) // If on first char then join to previous line
	{
		// unsigned int prevValue = DISPCOLCTR ;

		leftArrow(); // this takes you to the previous line because we are on first char
		DISPCOLCTR = CURLINESIZE + 1;
		CURSORCOL = CURLINESIZE - CURCOLPOS + 1;
		joinLine(DISPLINECTR + 1); // we should then join the line where backspace was pressed
	}
	else if (!(DISPLINECTR == 1 && DISPCOLCTR == 1)) // backspace only when not on first line and first column
	{
		leftArrow();
		delete_char();
	}
}
//
// goes to beginning of the line
//
void lineBegin()
{
	CURSORCOL = 1;
	DISPCOLCTR = 1;
	CURCOLPOS = 0;
}
//
// goes to the beginning of the file
//
void FileBeg()
{
	CURSORCOL = 1;
	CURSORROW = 1;
	DISPCOLCTR = 1;
	CURCOLPOS = 0;
	CURLINEPOS = 0;
	DISPLINECTR = 1;
}
//
// goes to the end of the line
//
void lineEnd()
{

	DISPCOLCTR = CURLINESIZE;
	DISPCOLCTR = DISPCOLCTR > 0 ? DISPCOLCTR : 1;
	if (CURLINESIZE <= scrnWidth)
	{
		CURCOLPOS = 0;
		CURSORCOL = DISPCOLCTR;
	}
	else
	{
		CURCOLPOS = DISPCOLCTR - scrnWidth;
		CURCOLPOS = CURCOLPOS > 0 ? CURCOLPOS : 0;
		CURSORCOL = CURLINESIZE - CURCOLPOS;
	}

	// arrowCommon() ;
}
//
// when scrolling up or down, it determines where to position the cursor appropriately
// depending on line length in relation to screen width
//
void arrowCommon() // called from both downArrow() as well as upArrow() functions
{
	if (CURLINESIZE < scrnWidth && CURLINESIZE < prevLineSize && DISPCOLCTR > CURLINESIZE)
	{
		CURCOLPOS = 0;
		CURSORCOL = CURLINESIZE % scrnWidth;
		DISPCOLCTR = CURLINESIZE;
		CURSORCOL = CURSORCOL > 0 ? CURSORCOL : 1;
		DISPCOLCTR = DISPCOLCTR > 0 ? DISPCOLCTR : 1;
	}

	// current line is longer than screen width and current column count as displayed in the status
	// line is greater than the current line size

	if (CURLINESIZE > scrnWidth && CURLINESIZE < prevLineSize && DISPCOLCTR > CURLINESIZE)
	{
		CURCOLPOS = DISPCOLCTR - scrnWidth;
		CURCOLPOS = CURCOLPOS > 0 ? CURCOLPOS : 0;

		CURSORCOL = CURLINESIZE - CURCOLPOS;

		// CURSORCOL = CURSORCOL - (prevLineSize - CURLINESIZE) ;
		DISPCOLCTR = CURLINESIZE;
		CURSORCOL = CURSORCOL > 0 ? CURSORCOL : 1;
		DISPCOLCTR = DISPCOLCTR > 0 ? DISPCOLCTR : 1;
	}
}
//
// allows the user scroll down
//
void downArrow()
{

	// prevLineSize = CURLINESIZE ;

	DISPLINECTR++;
	if (DISPLINECTR > FILETOTLINES)
	{
		DISPLINECTR = FILETOTLINES;
		beep();
	}
	else
	{
		CURSORROW++;
		if (CURSORROW > scrnHeight)
		{
			CURSORROW = scrnHeight;
			CURLINEPOS++;
			if (CURLINEPOS >= FILETOTLINES)
				CURLINEPOS = FILETOTLINES - 1;
		}
	}

	arrowCommon();
}
//
// allows the user to scroll up
//
void upArrow()
{

	// prevLineSize = CURLINESIZE ;

	DISPLINECTR--;
	if (DISPLINECTR <= 0)
		DISPLINECTR = 1;

	CURSORROW--;

	if (CURSORROW < 1)
	{
		CURSORROW = 1;
		CURLINEPOS = DISPLINECTR - 1;
		if (CURLINEPOS < 0)
		{
			beep();
			CURLINEPOS = 0;
		}
	}

	arrowCommon();
}
//
// allows the user to scroll right
//
void rightArrow()
{
	if (DISPCOLCTR == CURLINESIZE + 1)
	{
		lineBegin();
		downArrow();
	}
	else
	{
		CURSORCOL++;
		DISPCOLCTR++;
		if (DISPCOLCTR > CURLINESIZE)
			DISPCOLCTR = CURLINESIZE + 1;

		if (CURSORCOL > scrnWidth)
		{
			CURCOLPOS++;
			CURSORCOL = scrnWidth + 1;
		}

		if (CURCOLPOS > CURLINESIZE - scrnWidth)
			CURCOLPOS = CURLINESIZE - scrnWidth;

		if (CURLINESIZE < scrnWidth)
		{
			CURCOLPOS = 0;
			if (CURSORCOL > CURLINESIZE)
				CURSORCOL = CURLINESIZE + 1;
		}
	}
}
//
// allows the user to scroll left
//
void leftArrow()
{
	if (CURSORCOL > 1)
		CURSORCOL--;
	DISPCOLCTR--;
	if (DISPCOLCTR < 1) // go to end of previous line if possibe
	{
		if (DISPLINECTR > 1) // try to wrap to end of previous line if not at the very first line
		{
			CURSORROW--;
			DISPLINECTR--;
			lineEnd();
		}
		else
			DISPCOLCTR = 1;
	}
	else if (DISPCOLCTR <= CURCOLPOS)
	{
		CURCOLPOS = DISPCOLCTR - 1;
		if (CURCOLPOS < 0)
			CURCOLPOS = 0;
	}
}
//
// Saves the file. When the user choose the save as option it
// pulls up a dialog box for the user to enter the filename
// they want to save the file as
//
void savePrompt(string str1, string str2)
{
	if (touched)
	{
		int rtn;
		rtn = dispError(str1, str2);
		if (rtn == 32) // space bar pressed; so save the file
		{
			if (INFILENAME.size() == 0)
				getFileName(INFILENAME);
			fileSave();
		}
		touched = 0;
	}
}
//
// takes care of the button presses
//
void handlekeys()
{
	int stayput = 1, x = 0;

	while (stayput)
	{
		x = wgetch(mainscrwin);

		if (x > 31 && x < 127)
		{
			// edit stub
			insert_char(x);
			continue;
		}

		prevLineSize = CURLINESIZE;

		switch (x)
		{
		case 13: // enter key

			splitLine();
			break;
		case KEY_HOME:
			lineBegin();
			break;
		case KEY_END:
			lineEnd();
			break;
		case KEY_F(5):
			FileBeg();
			break;
		case KEY_BACKSPACE:
			backspace();
			break;
		case KEY_DC:
			delete_char();
			break;
		case KEY_F(9):
			savePrompt("Work not saved, press space bar to save and exit", "Any other key to exit without saving...");
			stayput = 0;
			break;
		case KEY_F(1):
			int msel;
			msel = fileMenu();
			if (msel == 1) // save selected
			{
				if (INFILENAME.size() == 0)
					getFileName(INFILENAME);
				fileSave();
			}
			else if (msel == 0) // open selected
			{
				string curfilename = INFILENAME;
				savePrompt("Work not saved, press space bar to save and open", "Any other key to open without saving...");
				getFileName(INFILENAME);
				if (!fileOpenRead()) // in case the file operation fails restore the old file name.
				{
					INFILENAME.clear();
					INFILENAME = curfilename;
				}
			}
			else if (msel == 2) // save as selected
			{
				getFileName(INFILENAME);
				fileSave();
			}
			else if (msel == 3) // exit selected
			{
				savePrompt("Work not saved, press space bar to save and exit", "Any other key to exit without saving...");
				stayput = 0;
			}
			break;

		case KEY_UP:
			upArrow();
			break;

		case KEY_DOWN:
			downArrow();
			break;

		case KEY_SF:
		case KEY_NPAGE:
			if (DISPLINECTR >= FILETOTLINES)
				beep();
			else
			{
				DISPLINECTR += scrnHeight;
				if (DISPLINECTR > FILETOTLINES)
					DISPLINECTR = FILETOTLINES;
				if (DISPLINECTR > CURLINEPOS)
					CURLINEPOS = DISPLINECTR - 1;
				CURSORROW = 1;
				// showScreen();
			}
			break;
		case KEY_SR:
		case KEY_PPAGE:
			if (CURLINEPOS <= 0)
				beep();
			else
			{
				DISPLINECTR -= scrnHeight;
				CURLINEPOS = DISPLINECTR - 1;
				CURSORROW = 1;
				if (CURLINEPOS < 0)
				{
					CURLINEPOS = 0;
					DISPLINECTR = 1;
					CURSORROW = 1;
				}
				// showScreen();
			}
			break;

		case KEY_RIGHT:
			rightArrow();
			break;

		case KEY_RESIZE: // NCURSES sends KEY_RESIZE in the getch stream when a window is resized
			DISPCOLCTR = CURCOLPOS + 1;
			DISPLINECTR = CURLINEPOS + 1;

			delwin(mainscrwin);
			delwin(hdrwin);
			delwin(ftrwin);
			initWin();
			break;

		case KEY_LEFT:
			leftArrow();
			break;
		}
		showScreen();
	}
}
//
// Basically constructs the FileMenu
//
int fileMenu()
{
	vector<string> menuList; // vector of strings to store the menu items

	menuList.push_back("open");
	menuList.push_back("Save");
	menuList.push_back("Save As");
	menuList.push_back("Exit");

	WINDOW *menuwin;
	menuwin = newwin(6, 12, 1, 1);
	box(menuwin, 0, 0);

	int itemcount = menuList.size();
	for (int i = 0; i < itemcount; i++) //  Show the menu
	{
		wattron(menuwin, COLOR_PAIR(3));
		mvwprintw(menuwin, i + 1, 2, "%s", menuList.at(i).c_str());
		wattroff(menuwin, COLOR_PAIR(3));
	}
	wrefresh(menuwin);

	keypad(menuwin, TRUE);

	curs_set(0); // hides the default screen cursor.

	int ch, i = 0, selectionMade = 0, escapePressed = 0;

	wattron(menuwin, A_STANDOUT);
	mvwprintw(menuwin, i + 1, 2, "%s", menuList.at(i).c_str());
	wattroff(menuwin, A_STANDOUT);

	while (!selectionMade) // Get the user selection
	{
		ch = wgetch(menuwin);

		wattron(menuwin, COLOR_PAIR(3));
		mvwprintw(menuwin, i + 1, 2, "%s", menuList.at(i).c_str());
		wattroff(menuwin, COLOR_PAIR(3));

		switch (ch)
		{
		case KEY_UP:
			i--;
			i = (i < 0) ? itemcount - 1 : i;
			break;
		case KEY_DOWN:
			i++;
			i = (i > itemcount - 1) ? 0 : i;
			break;
		case 27:
			selectionMade = 1;
			escapePressed = 1;
			break;
		case 13:
		case 32:
			selectionMade = 1;
			break;
		}

		wattron(menuwin, A_STANDOUT);
		mvwprintw(menuwin, i + 1, 2, "%s", menuList.at(i).c_str());
		wattroff(menuwin, A_STANDOUT);
	}
	curs_set(1);

	delwin(menuwin);
	return escapePressed ? -1 : i;
}
//
// Prompts the dialog box when the user want to open a file
// or save it with a different filename
//
void getFileName(string &fldStr)
{
	int x, h, w;

	string nfldPrompt = "file name : ";
	int fldYCoordinate = 2;
	unsigned int fldXCoordinate = nfldPrompt.size() + 1;

	unsigned int cursorXPos = fldXCoordinate;
	unsigned int strColPos = 0;
	string emptyStr;
	WINDOW *fldWin;

	unsigned int height, width;

	INFILENAME.clear();

	height = 6;
	h = (LINES - height) / 2;
	h = h > 0 ? h : 1; // y coordinate

	width = 70;
	w = (COLS - width) / 2;
	w = w > 0 ? w : 1; // x coordinate

	fldWin = newwin(height, width, h, w);

	keypad(fldWin, TRUE);

	//        wmove(fldWin,1,1);

	box(fldWin, ACS_VLINE, ACS_HLINE);

	wmove(fldWin, 2, 2);
	wprintw(fldWin, nfldPrompt.c_str());
	wmove(fldWin, fldYCoordinate, cursorXPos);
	wrefresh(fldWin);

	while (1)
	{
		x = wgetch(fldWin);
		if (x == 13) // enter key pressed
			break;
		switch (x)
		{
		case KEY_BACKSPACE:
			if (cursorXPos > fldXCoordinate)
			{
				cursorXPos--;
				if (strColPos > 0)
				{
					strColPos--;
					strColPos = strColPos < 0 ? 0 : strColPos;
					if (fldStr.size() > 0)
						fldStr.erase(strColPos, 1);
					wmove(fldWin, fldYCoordinate, fldXCoordinate);
					wprintw(fldWin, "%s", emptyStr.c_str());

					wmove(fldWin, fldYCoordinate, fldXCoordinate);
					wprintw(fldWin, "%s", fldStr.c_str());
					emptyStr.pop_back();
				}
			}
			else
				beep();
			break;
		case KEY_DC: /* Del key  pressed*/
			if (strColPos >= 0 && strColPos < fldStr.size())
			{
				if (fldStr.size() > 0)
					fldStr.erase(strColPos, 1);
				wmove(fldWin, fldYCoordinate, fldXCoordinate);
				wprintw(fldWin, "%s", emptyStr.c_str());

				wmove(fldWin, fldYCoordinate, fldXCoordinate);
				wprintw(fldWin, "%s", fldStr.c_str());
				if (emptyStr.size() > 0)
					emptyStr.pop_back();
			}
			break;
		case KEY_LEFT:
			if (cursorXPos > fldXCoordinate)
			{
				cursorXPos--;
				if (strColPos > 0)
					strColPos--;
			}
			else
				beep();
			break;
		case KEY_RIGHT:
			if (cursorXPos < fldXCoordinate + fldStr.size())
			{
				cursorXPos++;
				if (strColPos < width - 5)
					strColPos++;
			}
			else
				beep();
			break;
		default:
			if (!((x >= 'A' && x <= 'z') || (x >= '0' && x <= '9') || x == '/' || x == ' ' || x == '.' || x == '_' || x == '-'))
			{
				beep();
				break;
			}
			if (strColPos == 0 && x == ' ')
				break;

			if (strColPos < width - 5)
			{
				fldStr.insert(strColPos > 0 ? strColPos : 0, 1, x);
				emptyStr.push_back(' ');
				strColPos++;
				cursorXPos++;
				wmove(fldWin, fldYCoordinate, fldXCoordinate);
				wprintw(fldWin, "%s", fldStr.c_str());
			}
			break;
		}
		wmove(fldWin, fldYCoordinate, cursorXPos);
		wrefresh(fldWin);
	}
	/*
	wmove(fldWin,fldYCoordinate + 2,cursorXPos);
	wprintw(fldWin,"%s",fldStr.c_str()) ;
	wrefresh(fldWin);
	wgetch(fldWin) ;
	*/
	delwin(fldWin);
}
//
// shows the errors when the user uses the fileMenu
// options inappropriately
//
int dispError(string fldStr1, string fldStr2)
{
	WINDOW *fldWin;

	int height, width, h, w, ch;

	height = 6;
	h = (LINES - height) / 2;
	h = h > 0 ? h : 1; // y coordinate

	width = 70;
	w = (COLS - width) / 2;
	w = w > 0 ? w : 1; // x coordinate

	fldWin = newwin(height, width, h, w);

	keypad(fldWin, TRUE);

	box(fldWin, ACS_VLINE, ACS_HLINE);

	wmove(fldWin, 1, 2);
	wprintw(fldWin, "%s", fldStr1.c_str());
	wmove(fldWin, 3, 2);
	wprintw(fldWin, "%s", fldStr2.c_str());

	wrefresh(fldWin);

	ch = wgetch(fldWin);
	delwin(fldWin);
	return ch;
}
//
// Checks to see if user enters a fileName
// to open with the program
//
void chkArgs(int argc, char *argv[])
{
	INFILENAME.clear();

	if (argc > 1)
	{
		INFILENAME = argv[1];
		if (!fileOpenRead()) // file open not successful but the user wanted to start blank
		{

			INFILENAME.clear();
			varInit();
		}
	}
	else
		varInit();
}
