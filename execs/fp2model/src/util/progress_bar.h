#ifndef PROGRESS_BAR_H
#define PROGRESS_BAR_H

/* progress_bar.h:
 *
 * 	This class will print and update 
 * 	a progress bar to the screen, allowing
 * 	a user to see the status of computation
 * 	for a given program.
 */

#include <string>
#include <time.h>

/* the following defines the progress bar class */
class progress_bar_t
{
	/*** constants ***/
	public:

	/* terminal colors (for unix only) */
	enum COLOR
	{
		BLACK  = 40,
		RED    = 41,
		GREEN  = 42,
		YELLOW = 43,
		BLUE   = 44,
		PURPLE = 45,
		CYAN   = 46,
		WHITE  = 47
	};

	/*** parameters ***/
	private:

	double last_val; /* the value, in range [0,1] */
	clock_t last_time; /* the last time the progress bar was updated */
	int num_updates; /* number of times update has been called */

	int length; /* the length of the progress bar, in characters */
	int stripe_width; /* width of stripes for updating without vals */
	progress_bar_t::COLOR color; /* color of progress bar foreground */
	std::string name; /* the label to display */

	bool visible; /* true if printed to screen */
	double res; /* updates display in these increments of progress */
	clock_t min_time; /* won't update until this much time has passed
				since last update */

	/*** functions ***/
	public:

	/* constructors */
	progress_bar_t();
	~progress_bar_t();

	/* rendering */

	/* clear:
	 *
	 * 	Clears the progress bar
	 * 	from screen, and resets its
	 * 	value to be at 0%.
	 */
	void clear();

	/* update:
	 *
	 * 	Updates the value of this
	 * 	progress bar, which may
	 * 	also reprint it to the screen.
	 *
	 * arguments:
	 *
	 * 	val -	Number between 0 and 1
	 */
	void update(double val);

	/* update:
	 *
	 * 	Same as above, but will use
	 * 	val = curr / total
	 */
	inline void update(int curr, int total)
	{ this->update( ((double) curr) / total ); };

	/* update:
	 *
	 * 	Will update the progress bar, but
	 * 	instead of displaying a value, will
	 * 	just cycle.  Useful if the programmer
	 * 	wants to communicate to the user that
	 * 	the code is processing, but doesn't
	 * 	know how long it will take.
	 *
	 * 	Currently buggy on windows.
	 */
	void update();

	/* settings */

	/* set_name:
	 *
	 * 	Specifies the label of
	 * 	this progress bar.
	 */
	inline void set_name(const std::string& lab)
	{ this->name = lab; };

	/* set_length:
	 *
	 * 	Sets the length of the progress bar, in
	 * 	number of characters to be displayed.
	 */
	inline void set_length(int len)
	{ this->length = len; };
	
	/* set_color:
	 *
	 * 	Sets the color of the progress bar (for unix only).
	 */
	inline void set_color(progress_bar_t::COLOR c)
	{ this->color = c; };

	/* set_resolution:
	 *
	 * 	Specifies the resolution of the
	 * 	progress bar.  The display will only
	 * 	be updated in increments of this value.
	 */
	inline void set_resolution(double r)
	{ this->res = r; };

	/* set_min_time:
	 *
	 * 	Specifies the minimum amount of clock time required
	 * 	to pass before the progress bar will write to the screen
	 * 	again.
	 *
	 * arguments:
	 *
	 * 	mt -	Minimum time, in seconds
	 */
	inline void set_min_time(double mt)
	{ this->min_time = (clock_t) (mt * CLOCKS_PER_SEC); };
};

#endif
