#include "conf_reader.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>

/**
 * @file    conf_reader.cpp
 * @author  Eric Turner <elturner@eecs.berkeley.edu>
 * @brief   Reads .conf files
 *
 * @section DESCRIPTION
 *
 * The conf_reader_t class will parse .conf files or formatted
 * input streams.  The parsing is flexible enough to allow
 * a user to specify delimiter, comment, and newline characters.
 */

using namespace std;
using namespace conf;

/*-----------------------------------*/
/* reader_t function implementations */
/*-----------------------------------*/

reader_t::reader_t()
{
	/* set to default */
	this->reset();
}

reader_t::reader_t(const std::string& filename)
{
	/* set to default */
	this->reset();
	
	/* parse the file */
	this->parse(filename);
}

reader_t::reader_t(std::istream& is)
{
	/* set to default */
	this->reset();
	
	/* parse the stream */
	this->parse(is);
}

void reader_t::reset()
{
	/* clear any info */
	this->clear();

	/* clear all sets */
	this->keywords.clear();
	this->linebreaks.clear();
	this->comments.clear();
	this->delimiters.clear();
	this->verbose = true;
	
	/* set defaults */
	this->linebreaks.insert('\n');
	this->linebreaks.insert(';');
	this->comments.insert('#');
	this->delimiters.insert(' ');
	this->delimiters.insert('\t');
	this->delimiters.insert('\r');
}
			
void reader_t::set_linebreak(char b)
{
	/* clear any stored values */
	this->linebreaks.clear();
	
	/* newline is forced to always be a linebreak */
	this->linebreaks.insert('\n');
	this->add_linebreak(b);
}

void reader_t::add_linebreak(char b)
{
	/* append new linebreak character */
	this->linebreaks.insert(b);
}			

void reader_t::set_comment(char c)
{
	this->comments.clear();
	this->add_comment(c);
}

void reader_t::add_comment(char c)
{
	/* check if invalid character */
	if(c == '\n')
	{
		/* ignore this */
		if(this->verbose)
			cerr << "[conf::reader_t::add_comment]\t"
			     << "Newlines ('\\n') are reserved and "
			     << "cannot be used for comments." << endl;
		return;
	}

	/* add comment */
	this->comments.insert(c);
}

void reader_t::set_delimiter(char d)
{
	/* clear existing delimiters */
	this->delimiters.clear();
	this->add_delimiter(d);
}
			
void reader_t::add_delimiter(char d)
{
	/* check if invalid character */
	if(d == '\n')
	{
		/* ignore this */
		if(this->verbose)
			cerr << "[conf::reader_t::add_delimiter]\t"
			     << "Newlines ('\\n') are reserved and "
			     << "cannot be used for delimiters." << endl;
		return;
	}

	/* add it */
	this->delimiters.insert(d);
}
			
void reader_t::add_keyword(const std::string& k, int num_args)
{
	pair<map<string, int>::iterator, bool> ins;

	/* add keyword */
	ins = this->keywords.insert(pair<string,int>(k,num_args));
	if( !(ins.second) )
	{
		/* if got here, means keyword was already
		 * defined.  Just update its num args */
		ins.first->second = num_args;
	}
}
			
int reader_t::parse(const std::string& filename)
{
	ifstream infile;

	/* open the file */
	infile.open(filename.c_str());
	if(!(infile.is_open()))
	{
		/* check if we should say error message */
		if(this->verbose)
		{
			cerr << "[conf::reader_t::parse]\t"
			     << "Unable to open input file: "
			     << filename << endl;
		}

		/* return error */
		return -1;
	}

	/* parse the file */
	if(this->parse(infile))
	{
		/* an error occurred during parsing */
		if(this->verbose)
		{
			cerr << "[conf::reader_t::parse]\t"
			     << "Unable to parse input file: "
			     << filename << endl;
		}
		return -2;
	}
	
	/* clean up */
	infile.close();
	return 0;
}
			
int reader_t::parse(std::istream& is)
{
	map<string,int>::iterator it;
	vector<string> lines;
	vector<string> tokens;
	command_t cmd;
	string currline;
	bool iscomment;
	size_t i, n, j, m;
	char c;

	/* iterate until end of stream */
	iscomment = false;
	while(!is.eof())
	{
		/* get the next character from the stream */
		is.get(c);

		/* check for a line separator, which
		 * denotes the end of current line. */
		if(this->linebreaks.count(c) > 0)
		{
			/* store current line */
			if(!(currline.empty()))
			{
				lines.push_back(currline);
				currline.clear();
			}

			/* start of a new line */
			iscomment = false;
			continue;
		}

		/* check if we are currently in a
		 * comment, in which case, ignore everything */
		if(iscomment)
			continue;

		/* check if starting new comment */
		if(this->comments.count(c) > 0)
		{
			/* any line currently in progress
			 * ends here */
			if(!(currline.empty()))
			{
				lines.push_back(currline);
				currline.clear();
			}
			
			/* now in a comment state until end of line */
			iscomment = true;
			continue;
		}

		/* we're in a valid portion of a line,
		 * so add character */
		currline += c;
	}

	/* push any remaining values */
	if(!(currline.empty()))
	{
		lines.push_back(currline);
		currline.clear();
	}

	/* now we have a set of stripped lines, where each
	 * line can have at most one command on it.
	 *
	 * Iterate through them */
	n = lines.size();
	for(i = 0; i < n; i++)
	{
		/* clear tokens */
		currline.clear();
		tokens.clear();

		/* iterate over the characters in this line */
		m = lines[i].size();
		for(j = 0; j < m; j++)
		{
			/* get this character */
			c = lines[i][j];

			/* if the character is a delimiter,
			 * then push current line to list
			 * and reset it.
			 *
			 * If not a delimiter, add to current
			 * line. */
			if(this->delimiters.count(c) > 0)
			{
				/* check if we have any tokens */
				if(currline.empty())
					continue;

				/* add to list */
				tokens.push_back(currline);
				currline.clear();
			}
			else
			{
				/* not a delimiter, so add it
				 * to currline */
				currline += c;
			}
		}

		/* push any remaining values */
		if(!(currline.empty()))
		{
			tokens.push_back(currline);
			currline.clear();
		}

		/* check if we have any tokens */
		if(tokens.empty())
			continue;

		/* make a new command */
		cmd.keyword = tokens[0];
		cmd.args.clear();
		cmd.args.insert(cmd.args.end(),
			tokens.begin()+1, tokens.end());

		/* check if we recognize the command */
		it = this->keywords.find(cmd.keyword);
		if(it == this->keywords.end())
		{
			/* error: command not recognized */
			if(this->verbose)
			{
				cerr << "[conf::reader_t::parse]\t"
				     << "Unrecognized command: \""
				     << cmd.keyword << "\"" << endl;
			}
			return -1;
		}

		/* check if it has the correct number of arguments */
		if(it->second >= 0 && it->second != cmd.args.size())
		{
			/* incorrect number of arguments */
			if(this->verbose)
			{
				cerr << "[conf::reader_t::parse]\t"
				     << "Syntax error!  \""
				     << cmd.keyword << "\" expects "
				     << it->second << " argument"
				     << (it->second == 1 ? "" : "s")
				     << ", but was given "
				     << cmd.args.size() << endl;
			}
			return -2;
		}

		/* add this command to the records */
		this->commands.push_back(cmd);
	}

	/* success */
	return 0;
}
			
int reader_t::serialize(const std::string& filename) const
{
	ofstream outfile;

	/* prepare file for writing */
	outfile.open(filename.c_str());
	if(!(outfile.is_open()))
	{
		/* unable to open file */
		if(this->verbose)
		{
			/* print error */
			cerr << "[conf::reader_t::serialize]\t"
			     << "Unable to open output file: "
			     << filename << endl;
		}
		return -1;
	}

	/* write the file */
	this->serialize(outfile);

	/* clean up */
	outfile.close();
	return 0;
}
			
void reader_t::serialize(std::ostream& os) const
{
	size_t i, num_cmds, j, num_args;

	/* iterate over the commands stored */
	num_cmds = this->commands.size();
	for(i = 0; i < num_cmds; i++)
	{
		/* print the keyword */
		os << this->commands[i].keyword;

		/* print the args */
		num_args = this->commands[i].args.size();
		for(j = 0; j < num_args; j++)
		{
			/* print delimiter and argument */
			os << *(this->delimiters.begin());
			os << this->commands[i].args[j];
		}

		/* print a newline */
		os << *(this->linebreaks.begin());
	}
}

/*------------------------------------*/
/* command_t function implementations */
/*------------------------------------*/

void command_t::print() const
{
	size_t i, n;

	/* print keyword */
	cout << "[" << this->keyword << "]";

	/* print args */
	n = this->args.size();
	for(i = 0; i < n; i++)
		cout << " " << this->args[i];
	cout << endl;
}
