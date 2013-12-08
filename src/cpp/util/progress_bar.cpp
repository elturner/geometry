#include "progress_bar.h"
#include <string>
#include <time.h>
#include <stdio.h>

/* implementation of progress bar class */

progress_bar_t::progress_bar_t()
{
	/* set reasonable default values */
	this->last_val = 0.0;
	this->min_time = CLOCKS_PER_SEC / 5; /* default max freq: 30 Hz */
	this->num_updates = 0;

	this->name = "progress";
	this->length = 50;
	this->stripe_width = 20;
	this->color = progress_bar_t::GREEN;

	this->visible = false;
	this->res = 0.001;
	this->min_time = 0;
}

progress_bar_t::~progress_bar_t()
{
	/* clear from screen */
	this->clear();
}

#ifdef _WIN32

/* The following implmentation is for a Windows environment, which
 * supports different terminal codes.  As such, colors and multi-line
 * progress bars are NOT supported */

void progress_bar_t::clear()
{
	size_t i, n;

	/* clear from screen if visible */
	if(this->visible)
	{
		/* total length of line */
		n = 9 + this->length + this->name.size();

		/* delete and clear line */
		for(i = 0; i < n; i++)
			printf("\b");
		for(i = 0; i < n; i++)
			printf(" ");
		for(i = 0; i < n; i++)
			printf("\b");
	}

	/* reset parameters */
	this->last_val = 0.0;
	this->visible = false;
	this->num_updates = 0;
}

void progress_bar_t::update(double val)
{
	size_t i, k, n;
	clock_t now;
	double diff;

	/* clear from screen if visible */
	now = clock();
	if(this->visible)
	{
		/* total length of line */
		n = 9 + this->length + this->name.size();

		/* delete and clear line */
		for(i = 0; i < n; i++)
			printf("\b");
		for(i = 0; i < n; i++)
			printf(" ");
		for(i = 0; i < n; i++)
			printf("\b");
		
		/* check if we should bother drawing */
		if(now - this->last_time <= this->min_time)
			return; /* too soon since last draw */
		diff = val - this->last_val;
		if((diff < this->res && diff >= 0)
				|| (-diff < this->res && diff < 0))
			return; /* not enough progress to update */
	}

	/* draw header */
	printf("%s [", this->name.c_str());

	/* determine number of characters to draw */
	k = (int) (this->length * val);
	for(i = 0; i < k; i++)
		printf("=");
	
	/* draw blanks */
	for(i = k; i < this->length; i++)
		printf(" ");

	/* draw percentage */
	printf("] %4.1f%c", 100*val, 0x25);
	fflush(stdout);

	/* update stored value */
	this->last_val = val;
	this->last_time = now;
	this->visible = true;
	this->num_updates++;
}

void progress_bar_t::update()
{
	size_t i, n;
	clock_t now;

	/* clear from screen if visible */
	now = clock();
	if(this->visible)
	{
		/* total length of line */
		n = 9 + this->length + this->name.size();

		/* delete and clear line */
		for(i = 0; i < n; i++)
			printf("\b");
		for(i = 0; i < n; i++)
			printf(" ");
		for(i = 0; i < n; i++)
			printf("\b");
		
		/* check if we should bother drawing */
		if(now - this->last_time <= this->min_time)
			return; /* too soon since last draw */
	}

	/* draw header */
	printf("%s [", this->name.c_str());

	/* draw striped pattern */
	for(i = 0; i < this->length; i++)
	{
		/* print stripes of texture */
		if((i % this->stripe_width) 
			== (this->num_updates % this->stripe_width))
		{
			/* textured stripe */
			printf("=");
		}
		else
		{
			/* non-colored stripe */
			printf(" ");
		}
	}


	/* draw tail */
	printf("]      ");
	fflush(stdout);

	/* update stored value */
	this->last_time = now;
	this->visible = true;
	this->num_updates++;
}

#else

/* The following functions are used in Unix environments, and
 * support color and multi-layer progress bars */

void progress_bar_t::clear()
{
	/* clear from screen if visible */
	if(this->visible)
	{
		/* delete previous two lines */
		printf("%c[1F", 0x1b); /* move up */
		printf("%c[2K", 0x1b); /* clear */
		printf("%c[1F", 0x1b); /* move up */
		printf("%c[2K", 0x1b); /* clear */
	}

	/* reset parameters */
	this->last_val = 0.0;
	this->visible = false;
	this->num_updates = 0;
}

void progress_bar_t::update(double val)
{
	int i, k;
	clock_t now;
	double diff;

	/* check state */
	now = clock();
	if(!(this->visible))
	{
		/* this is the first time drawing */
		printf("\n\n");
		this->visible = true;
	}
	else
	{	
		/* check if we should bother drawing */
		if(now - this->last_time <= this->min_time)
			return; /* too soon since last draw */
		diff = val - this->last_val;
		if((diff < this->res && diff >= 0)
				|| (-diff < this->res && diff < 0))
			return; /* not enough progress to update */
	}
	
	/* update screen with new info */
	
	/* delete previous line */
	printf("%c[1F", 0x1b);
	printf("%c[2K", 0x1b);
	
	/* determine number of characters to draw */
	k = (int) (this->length * val);
	printf("%s: ", this->name.c_str());
	printf("%c[0;%dm", 0x1b, this->color); /* color it */
	for(i = 0; i < k; i++)
		printf(" ");
	printf("%c[0m", 0x1b); /* reset color */
	
	/* draw blanks */
	printf("%c[0;1;40m", 0x1b); /* make it white */
	for(i = k; i < this->length; i++)
		printf(" ");
	printf("%c[0m", 0x1b); /* reset color */
	
	/* draw percentage */
	printf(" %4.1f%c\n", 100 * val, 0x25);
	
	/* update stored value */
	this->last_val = val;
	this->last_time = now;
	this->num_updates++;
}
	
void progress_bar_t::update()
{
	int i;
	clock_t now;

	/* check state */
	now = clock();
	if(!(this->visible))
	{
		/* this is the first time drawing */
		printf("\n\n");
		this->visible = true;
	}
	else
	{	
		/* check if we should bother drawing */
		if(now - this->last_time <= this->min_time)
			return; /* too soon since last draw */
	}
	
	/* update screen with new info */
	
	/* delete previous line */
	printf("%c[1F", 0x1b);
	printf("%c[2K", 0x1b);
	
	/* determine number of characters to draw */
	printf("%s: ", this->name.c_str());
	for(i = 0; i < this->length; i++)
	{
		/* print stripes of color */
		if((i % this->stripe_width) 
			== (this->num_updates % this->stripe_width))
		{
			/* colored stripe */
			printf("%c[0;%dm %c[0m", 
				0x1b, this->color, 0x1b); /* color it */
		}
		else
		{
			/* non-colored stripe */
			printf(" ");
		}
	}
	printf("\n");
	
	/* update stored value */
	this->last_time = now;
	this->num_updates++;
}

#endif
