#ifndef PROGRESS_BAR_H
#define PROGRESS_BAR_H

#include <string>

using namespace std;

/* reserve_progress_bar:
 *
 * 	Reserves space for a progress bar.
 */
void reserve_progress_bar();

/* progress_bar:
 *
 * 	Prints a progress bar to screen, with
 * 	the amount completed specified by 'amount'
 *
 * arguments:
 *
 * 	name -		The name of this progress bar.
 * 	amount -	Value between 0 and 1. Denotes
 * 			amount of progress bar to draw.
 */
void progress_bar(string name, double amount);

/* delete_progres_bar:
 *
 *	Removes the last line on the screen.
 */
void delete_progress_bar();


#endif
