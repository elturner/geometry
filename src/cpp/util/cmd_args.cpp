#include "cmd_args.h"
#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <stdlib.h>

/**
 * @file cmd_args.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file implements the cmd_args_t and cmd_tag_t
 * classes, which are used to parse command-line arguments
 * into a user-friendly struct for a program to receive
 * configuration options.
 */

using namespace std;

/* the following make formatting assumptions */
#define STANDARD_PAGE_WIDTH 79 /* assume 80 character wide pages */
#define DEFAULT_TAB_SIZE 4
#define WHITESPACE " \t\r\n" /* whitespace characters */

/* cmd_args_t function defintions */

cmd_args_t::cmd_args_t(bool printInOrder)
{
	/* empty structures for now */
	this->program_description = "";
	this->tab_width = DEFAULT_TAB_SIZE;
	this->tags.clear();
	this->files.clear();
	this->required_file_types.clear();
	this->filetype_purposes.clear();
	this->arg_order.clear();
	this->print_in_order = printInOrder;
}

cmd_args_t::~cmd_args_t()
{
	/* clear memory */
	this->tags.clear();
	this->files.clear();
	this->required_file_types.clear();
	this->filetype_purposes.clear();
	this->arg_order.clear();
}

void cmd_args_t::add(const string& t, const string& d, bool o, int n)
{
	cmd_tag_t tag;

	/* create a new tag structure
	 * and add it to the map */
	arg_order.push_back(t);
	tag.init(t, d, o, n);
	this->tags.insert(make_pair(t, tag));
}
		
void cmd_args_t::add_required_file_type(const string& ext, int m,
                                        const string& purpose)
{
	/* insert this file type into the required file types map */
	this->required_file_types[ext] = m;
	this->filetype_purposes[ext] = purpose;
}

int cmd_args_t::parse(int argc, char** argv)
{
	map<string, cmd_tag_t>::iterator it;
	map<string, vector<string> >::iterator fit;
	map<string, int>::iterator rit;
	unsigned int num_required;
	string ext, tag;
	int i, j;

	/* check arguments */
	if(argc <= 0 || argv == NULL)
		return -1;

	/* iterate through command-line arguments,
	 * ignoring the first one, since it's the
	 * program name */
	for(i = 1; i < argc; i++)
	{
		/* check if this argument
		 * is a known tag */
		tag = string(argv[i]);
		it = this->tags.find(tag);
		if(it == this->tags.end())
		{
			/* not a tag, attempt to parse as file */
			ext = cmd_args_t::file_extension(tag);
			if(ext.size() == 0)
			{
				/* not a file */
				cerr << "Unknown command-line argument: "
				     << tag << endl
				     << "Please see usage information:"
				     << endl;
				this->print_usage(argv[0]);
				return -2;
			}

			/* this is a file, so save it in the appropriate
			 * list for later */
			files[ext].push_back(tag); 
		}
		else
		{
			/* record that we saw this tag */
			it->second.found = true;

			/* check for its values */
			for(j = 0; 
				j < it->second.num_vals || it->second.num_vals == FLEX_ARGS; 
				j++)
			{
				/* verify we don't go out of bounds */
				i++;
				if(i >= argc)
				{
					// In flex args mode this is okay
					if(it->second.num_vals == FLEX_ARGS)
						return 0;

					/* insufficient number of values
					 * for this tag! */
					cerr << "The " << tag << " tag"
					     << " takes ";
					if(it->second.num_vals == 1)
						cerr << "an argument.";
					else
						cerr << it->second.num_vals
						     << " arguments.";
					cerr << "  Please see usage info:"
					     << endl;
					this->print_usage(argv[0]);
					return -3;
				}

				/* check if the next item is a valid tab */
				if(this->tags.count(argv[i]))
				{
					i--;
					break;
				}

				/* save this value */
				it->second.found_vals.push_back(
				           string(argv[i]));
			}
		}
	}

	/* check that all required tags were present */
	for(it = this->tags.begin(); it != this->tags.end(); it++)
	{
		/* ignore optional tags */
		if(it->second.optional)
			continue;

		/* verify that this tag was present */
		if(!(it->second.found))
		{
			/* missing required tag */
			cerr << "Must specify " << it->second.tag
			     << " tag!  See usage information:"
			     << endl;
			print_usage(argv[0]);
			return -4;
		}
	}

	/* check that all required files were present */
	for(rit = this->required_file_types.begin();
			rit != this->required_file_types.end(); rit++)
	{
		/* determine how many of this file were required */
		num_required = (unsigned int) rit->second;
		if(num_required == 0)
			continue;

		/* check how many occurances of this file type there were */
		fit = this->files.find(rit->first);
		if(fit == this->files.end()
			|| fit->second.size() < (unsigned int) rit->second)
		{
			/* error, not enough instances of this file type */
			cerr << "Must specify at least " << rit->second
			     << " *." << rit->first;
			if(rit->second == 1)
				cerr << " file!";
			else
				cerr << " files!";
			cerr << "  See usage information:" << endl;
			print_usage(argv[0]);
			return -5;
		}
	}

	/* success */
	return 0;
}

void cmd_args_t::print_usage(char* prog_name) const
{
	map<string, cmd_tag_t>::const_iterator it;
	map<string, int>::const_iterator rit;
	map<string, string>::const_iterator pit;
	string tab;
	stringstream line;
	int i, indent;

	/* set reasonable tab width */
	tab = this->generate_tab(); /* tab width generated from defined settings */
	
	/* print program description if available */
	if(!(this->program_description.empty()))
	{
		/* write subsection header */
		line.str("");
		cerr << endl << " Description:" << endl
		     << endl;

		/* write description as indented line */
		line << tab << this->program_description;
		cmd_args_t::write_line_with_indent(line.str(), tab.size());
		cerr << endl;
	}

	/* print program name */
	line.str("");
	cerr << endl
	     << " Usage:" << endl
	     << endl;
	line << tab << prog_name << " ";
	indent = line.str().size();
	if(indent >= STANDARD_PAGE_WIDTH)
		indent = tab.size();

	/* print all possible tags */
	if(!this->print_in_order)
	{
		for(it = this->tags.begin(); it != this->tags.end(); it++)
		{
			/* print tag name */
			line << (it->second.optional ? "[ " : "")
			     << it->second.tag << " ";
			
			/* print arguments for this tag */
			for(i = 0; i < it->second.num_vals; i++)
				line << "<arg_" << (i+1) << "> ";

			/* end optional bracket */
			if(it->second.optional)
				line << "] ";
		}
	}
	else
	{
		for(size_t j = 0; j < arg_order.size(); j++)
		{
			/* find the correct tag */
			it = tags.find(arg_order[j]);

			/* print tag name */
			line << (it->second.optional ? "[ " : "")
			     << it->second.tag << " ";
			
			/* print arguments for this tag */
			for(i = 0; i < it->second.num_vals; i++)
				line << "<arg_" << (i+1) << "> ";

			/* end optional bracket */
			if(it->second.optional)
				line << "] ";
		}
	}

	/* print files if necessary */
	if(this->required_file_types.size() > 0)
		line << "<files...>";
	cmd_args_t::write_line_with_indent(line.str(), indent);
	
	/* print details about tags */
	cerr << endl << endl;
	if(this->tags.size() > 0)
	{
		cerr << " Where:" << endl
		     << endl;

		if(!this->print_in_order)
			/* iterate over each tag */
			for(it = this->tags.begin(); it != this->tags.end(); it++)
			{
				/* prepare tag string */
				line.str("");
				line << tab << it->second.tag; 
				if(it->second.tag.size() < tab.size())
					line << tab.substr(it->second.tag.size());
				else
					line << tab;
				indent = line.str().size();
		
				/* print description with indents */
				stringstream ss;
				if(it->second.num_vals == FLEX_ARGS)
					ss << "*";
				else
					ss << it->second.num_vals;
				line 
					 << ((it->second.num_vals > 0 || 
					 	it->second.num_vals == FLEX_ARGS) ? 
					 	"Nargs : " + ss.str() : "") << tab
					 << (it->second.optional ? "Optional." : "")
					 << endl << endl
				     << it->second.description << endl << endl; 
				cmd_args_t::write_line_with_indent(line.str(),
				                                   2*tab.size());
			}
		else
		{
			for(size_t j = 0; j < arg_order.size(); j++)
			{
				/* find the correct tag */
				it = tags.find(arg_order[j]);

				/* prepare tag string */
				line.str("");
				line << tab << it->second.tag; 
				if(it->second.tag.size() < tab.size())
					line << tab.substr(it->second.tag.size());
				else
					line << tab;
				indent = line.str().size();
		
				/* print description with indents */
				stringstream ss;
				if(it->second.num_vals == FLEX_ARGS)
				{
					ss << "*";
				}
				else
					ss << it->second.num_vals;
				line 
					 << ((it->second.num_vals > 0 || 
					 	it->second.num_vals == FLEX_ARGS) ? 
					 	"Nargs : " + ss.str() : "") << tab
					 << (it->second.optional ? "Optional." : "")
					 << endl << endl
				     << it->second.description << endl << endl; 
				cmd_args_t::write_line_with_indent(line.str(),
				                                   2*tab.size());
			}
		}
	}

	/* print required file information */
	if(this->required_file_types.size() > 0)
	{
		cerr << " Required files:" << endl
		     << endl;

		/* iterate over required file types */
		for(rit = this->required_file_types.begin();
				rit != this->required_file_types.end();
				rit++)
		{
			/* prepare indent string */
			line.str("");
			line << tab << "*." << rit->first; 
			if(rit->first.size()+2 < tab.size())
				line << tab.substr(rit->first.size()+2);
			else
				line << tab;
			indent = line.str().size();
	
			/* get purpose of file */
			pit = this->filetype_purposes.find(rit->first);

			/* print info about this file type */
			if(rit->second == 0)
				line << "Optional file arguments.";
			else if(rit->second == 1)
				line << "At least one file required.";
			else
				line << "At least " << rit->second
				     << " files required.";
			line << ((pit == this->filetype_purposes.end())
			          ? "" : ("  " + pit->second))
			     << endl << endl;
			cmd_args_t::write_line_with_indent(line.str(),
			                                   indent);
		}
	}

	/* add a few new lines */
	cerr << endl << endl;
}
		
bool cmd_args_t::tag_seen(const std::string& tag) const
{
	map<string, cmd_tag_t>::const_iterator it;

	/* check if the specified tag has been seen */
	it = this->tags.find(tag);
	if(it == this->tags.end())
		return false; /* tag not valid */
	
	/* return whether or not it was found */
	return it->second.found;
}

bool cmd_args_t::tag_seen(const string& tag, vector<string>& vals) const
{
	map<string, cmd_tag_t>::const_iterator it;

	/* check if the specified tag has been seen */
	it = this->tags.find(tag);
	if(it == this->tags.end())
		return false; /* tag not valid */
	if(!(it->second.found))
		return false; /* not seen */

	/* copy values to caller */
	vals.clear();
	vals.insert(vals.begin(),
	            it->second.found_vals.begin(),
	            it->second.found_vals.end());
	return true;
}
		
string cmd_args_t::get_val(const string& t, unsigned int n) const
{
	map<string, cmd_tag_t>::const_iterator it;

	/* check if this tag was seen */
	it = this->tags.find(t);
	if(it == this->tags.end())
		return "";

	/* check if n'th value exists */
	if(it->second.found_vals.size() <= n)
		return "";

	/* return the n'th value */
	return it->second.found_vals[n];
}
		
void cmd_args_t::files_of_type(const string& format,
                               vector<string>& files) const
{
	map<string, vector<string> >::const_iterator fit;
	
	/* clear input list */
	files.clear();

	/* check for files of the specified format */
	fit = this->files.find(format);
	if(fit == this->files.end())
		return;

	/* populate it with the found files */
	files.insert(files.begin(), fit->second.begin(), fit->second.end());
}
		
string cmd_args_t::file_extension(const string& fn)
{
	size_t p;

	/* find the last period in the fn variable */
	p = fn.find_last_of('.');
	if(p == string::npos)
		return "";
	
	/* return the file extension */
	return fn.substr(p+1);
}
		
void cmd_args_t::write_line_with_indent(const std::string& line,
                                        int indent)
{
	stringstream ss;
	string to_print;
	int i;
	size_t p, newline;

	/* first, check for new lines in this input */
	newline = line.find_first_of('\n');
	if(newline != string::npos && newline < line.size()-1)
	{
		/* there exists a newline in the middle of
		 * this input, so we write it as multiple lines */
		to_print = line.substr(0, newline+1); /* include newline */
		cmd_args_t::write_line_with_indent(to_print, indent);

		/* write remainder of input */
		for(i = 0; i < indent; i++)
			ss << " ";
		ss << line.substr(newline+1);
		cmd_args_t::write_line_with_indent(ss.str(), indent);
		
		/* we're done here */
		return;
	}

	/* check base case */
	if(line.size() <= STANDARD_PAGE_WIDTH)
	{
		cerr << line;
		return;
	}

	/* parse current line from input */
	to_print = line.substr(0, STANDARD_PAGE_WIDTH);
	
	/* try to split the line at whitespace */
	p = to_print.find_last_of(WHITESPACE);
	if(p > (size_t) indent)
		to_print = line.substr(0, p+1);
	else
		p = STANDARD_PAGE_WIDTH;

	/* print the resulting first line */
	cerr << to_print << endl;

	/* indent the remainder */
	for(i = 0; i < indent; i++)
		ss << " ";
	ss << line.substr(p+1);
	cmd_args_t::write_line_with_indent(ss.str(), indent);
}

std::string cmd_args_t::generate_tab() const
{
	string tab;
	for(size_t i = 0; i < tab_width; i++)
		tab += " ";
	return tab;
}

/* cmd_tag_t function implementations */

cmd_tag_t::cmd_tag_t()
{
	/* set default values */
	this->tag = "--";
	this->description = "The default tag";
	this->optional = true;
	this->found = false;
	this->num_vals = 0;
	this->found_vals.clear();
}

cmd_tag_t::~cmd_tag_t()
{
	/* clear all containers */
	this->found_vals.clear();
}

void cmd_tag_t::init(const string& t, const string& d, bool o, int n)
{
	/* populate fields */
	this->tag = t;
	this->description = d;
	this->optional = o;
	this->found = false;
	this->num_vals = n;
	this->found_vals.clear();
}

