#include <iostream>
#include <string>
#include <stdio.h>

using namespace std;

#define PROGRESS_BAR_LENGTH 50

void reserve_progress_bar()
{
	/* reserve two lines */
	cout << endl << endl;
}

void progress_bar(string name, double amount)
{
	int i, k;

	/* delete previous line */
	printf("%c[1F", 0x1b);
	printf("%c[2K", 0x1b);

	/* determine number of characters to draw */
	k = (int) (PROGRESS_BAR_LENGTH * amount);
	cout << " " << name << ": ";
	printf("%c[0;42m", 0x1b); /* make it green */
	for(i = 0; i < k; i++)
		cout << " ";
	printf("%c[0m", 0x1b); /* reset color */
	
	/* draw blanks */
	printf("%c[0;1;40m", 0x1b); /* make it white */
	for(i = k; i < PROGRESS_BAR_LENGTH; i++)
		cout << " ";
	printf("%c[0m", 0x1b); /* reset color */
	
	/* draw percentage */
	printf(" %4.1f%c", 100 * amount, 0x25);
	cout << endl;
}

void delete_progress_bar()
{	
	/* delete previous two lines */
	printf("%c[1F", 0x1b); /* move up */
	printf("%c[2K", 0x1b); /* clear */
	printf("%c[1F", 0x1b); /* move up */
	printf("%c[2K", 0x1b); /* clear */
}
