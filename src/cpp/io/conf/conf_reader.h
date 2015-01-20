#ifndef CONF_READER_H
#define CONF_READER_H

/**
 * @file    conf_reader.h
 * @author  Eric Turner <elturner@eecs.berkeley.edu>
 * @brief   Reads .conf files
 *
 * @section DESCRIPTION
 *
 * The conf_reader_t class will parse .conf files or formatted
 * input streams.  The parsing is flexible enough to allow
 * a user to specify delimiter, comment, and newline characters.
 */

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <map>

/**
 * The conf namespace is used to house classes related
 * to .conf files.
 */
namespace conf
{
	/* the following classes are defined in this namespace */
	class reader_t;
	class keyword_t;
	class command_t;

	/* this value represents a variable number of arguments for
	 * a keyword */
	static const int VARARGS = -1;

	/**
	 * The conf::reader_t class is used to parse .conf files
	 */
	class reader_t
	{
		/* parameters */
		private:

			/*------------*/
			/* parameters */
			/*------------*/

			/**
			 * The set of valid command keywords
			 *
			 * Maps from keyword to number of arguments.
			 * If a negative value is supplied as number
			 * of arguments, then a variable number of
			 * arguments can be given.
			 */
			std::map<std::string, keyword_t> keywords;

			/**
			 * The line-break characters
			 *
			 * These characters signify that
			 * a line has ended.  There must be
			 * at most one command per line.
			 *
			 * Empty lines are ignored.
			 *
			 * By default, these are newlines
			 */
			std::set<char> linebreaks;
			
			/**
			 * The comment characters
			 *
			 * Any parts of an input file/stream
			 * that occur between any of these
			 * characters and a line-break character
			 * will be ignored.
			 *
			 * By default, these are {'#'}
			 */
			std::set<char> comments;

			/**
			 * The deliminer characters
			 *
			 * These characters are used
			 * to delimite the arguments of
			 * commands.
			 *
			 * By default, they are whitespace.
			 */
			std::set<char> delimiters;

			/**
			 * If true, then will print to stderr
			 * any errors that are encountered.
			 *
			 * If false, nothing will be printed.
			 */
			bool verbose;

			/*----------*/
			/* contents */
			/*----------*/

			/**
			 * The list of commands parsed so far
			 */
			std::vector<command_t> commands;

		/* functions */
		public:

			/*--------------*/
			/* constructors */
			/*--------------*/

			/**
			 * Constructs empty reader_t object
			 */
			reader_t();

			/**
			 * Constructs reader and populates with file
			 *
			 * Given a file path, will populate this
			 * reader object with the contents of the
			 * file.
			 *
			 * If failure occurs (due to non-existent
			 * or empty file), then the reader object
			 * will be empty after this call.
			 *
			 * @param filename   The file to parse
			 */
			reader_t(const std::string& filename);

			/**
			 * Constructs reader and populates with stream
			 *
			 * Given an input stream, will populate this
			 * reader object with the contents of the
			 * stream.
			 *
			 * @param is   The input stream
			 */
			reader_t(std::istream& is);

			/*----------------*/
			/* initialization */
			/*----------------*/

			/**
			 * Clears all information from this structure
			 *
			 * Does NOT reset the character sets used
			 * for parsing.  Only clears the list
			 * of parsed commands.
			 */
			inline void clear()
			{ this->commands.clear(); };

			/**
			 * Resets delimiters, newlines, and comments
			 *
			 * Will reset the characters used to 
			 * parsing to factory defaults.
			 *
			 * Also calls clear_keywords()
			 */
			void reset();

			/**
			 * Resets the linebreak characters to
			 * be only the specified value
			 *
			 * Will clear any existing linebreak characters,
			 * and set the new linebreak character to
			 * be the given argument.
			 *
			 * @param b  The new linebreak character to use
			 */
			void set_linebreak(char b);

			/**
			 * Adds a linebreak character to check for
			 *
			 * The specified character will be considered
			 * a linebreak character from now on.
			 *
			 * @param b  The new linebreak character to use
			 */
			void add_linebreak(char b);

			/**
			 * Resets the comment characters to be
			 * only the specified value
			 *
			 * Will clear any existing comment characters,
			 * and set the new comment character to
			 * be the given argument.
			 *
			 * @param c  The new comment character to use
			 */
			void set_comment(char c);

			/**
			 * Adds a comment character to check for
			 *
			 * The specified character will be considered
			 * a comment character from now on.
			 *
			 * @param c   The new comment character
			 */
			void add_comment(char c);

			/**
			 * Resets the delimiter characters to be
			 * only the specified value.
			 *
			 * Will clear any existing delimiter characters,
			 * and set the new delimiter character to
			 * be the given argument.
			 *
			 * @param d   The new delimiter character to use
			 */
			void set_delimiter(char d);

			/**
			 * Adds a delimiter character to check for
			 *
			 * The specified character will be considered
			 * a delimiter character from now on.
			 *
			 * @param d   The new delimiter character
			 */
			void add_delimiter(char d);

			/**
			 * Clears any stored commands to check for
			 *
			 * After this call, the list of valid command
			 * keywords will be cleared.
			 */
			inline void clear_keywords()
			{ this->keywords.clear(); };

			/**
			 * Adds the specified command keyword
			 *
			 * Will add a specified keyword to look
			 * for, along with the number of arguments
			 * associated with this keyword.
			 *
			 * If a negative value is specified for
			 * num_args, then a variable number of
			 * arguments is allowed for this keyword.
			 *
			 * @param k         The new keyword
			 * @param helptext  Any helptext for this keyword
			 * @param num_args  The number of arguments
			 *                  to parse for this keyword
			 */
			void add_keyword(const std::string& k, 
					const std::string& helptext="",
					int num_args = VARARGS);

			/**
			 * Sets the verbose flag
			 *
			 * If the verbose flag is set to true (which
			 * it is by default), then any errors
			 * encountered will be printed to stderr.
			 *
			 * If verbose is set to false, then nothing
			 * will be printed to stderr.
			 *
			 * @param v   The new value of verbose flag
			 */
			inline void set_verbose(bool v)
			{ this->verbose = v; };

			/*-----*/
			/* i/o */
			/*-----*/

			/**
			 * Parses the specified file
			 *
			 * Given a filename, will parse the
			 * file using the existing settings.
			 *
			 * Any commands parsed from this file
			 * will be stored in this object.
			 *
			 * Any commands pre-existing in this
			 * object will still remain.  The new
			 * file will be appended to the current
			 * list.
			 *
			 * @param filename   The file to parse
			 *
			 * @return   Returns zero on success, non-zero
			 *           on failure.
			 */
			int parse(const std::string& filename);

			/**
			 * Parses the specified input stream
			 *
			 * Given an input stream, will parse
			 * the contents with current settings.
			 *
			 * Any commands found will be appending to
			 * the list of existing commands.
			 *
			 * @param is   The input stream to parse
			 *
			 * @return     Returns zero on success,
			 *             non-zero on failure.
			 */
			int parse(std::istream& is);

			/**
			 * Serializes the data in this structure
			 * to the specified file.
			 *
			 * Given a filepath, will export this
			 * structure to the file location.
			 *
			 * @param filename   Where to write this structure.
			 *
			 * @return     Returns zero on success, non-zero
			 *             on failure.
			 */
			int serialize(const std::string& filename) const;

			/**
			 * Serializes the data in this structure
			 * to the specified output stream
			 *
			 * Given a stream, will write this info
			 * to it.
			 *
			 * @param os    The output stream to write to
			 *
			 * @return      Returns zero on success, non-zero
			 *              on failure.
			 */
			void serialize(std::ostream& os) const;

			/**
			 * Prints help text about allowed keywords 
			 * and current settings.
			 *
			 * Given an output stream, will write ascii
			 * help text about how to format input to
			 * this reader.
			 *
			 * @param os  The output stream to write to
			 */
			void helptext(std::ostream& os) const;

			/*-----------*/
			/* accessors */
			/*-----------*/

			/**
			 * Checks if this object is empty
			 *
			 * A reader_t is empty if it has seen
			 * no commands so far.
			 */
			inline bool empty() const
			{ return this->commands.empty(); };

			/**
			 * Retrieves the number of commands in
			 * this structure.
			 *
			 * @return   The number of stored commands
			 */
			inline size_t size() const
			{ return this->commands.size(); };

			/**
			 * Get a reference to a given command
			 *
			 * @param i  Get the i'th command
			 */
			inline const command_t& get(size_t i) const
			{ return this->commands[i]; };
	};

	/**
	 * The conf::keyword_t class is used to define valid input
	 * commands and their expected attributes.
	 */
	class keyword_t
	{
		/* security */
		friend class reader_t;

		/* parameters */
		private:

			/**
			 * The keyword string that indicates this call
			 */
			std::string name;

			/**
			 * How many arguments to expect for a call to
			 * this command keyword.
			 *
			 * If a negative value is supplied, then
			 * a variable number of arguments can be
			 * taken.
			 */
			int num_args;

			/**
			 * Help text describing the purpose of the
			 * command this keyword represents.
			 */
			std::string helptext;

		/* functions */
		public:

			/*--------------*/
			/* constructors */
			/*--------------*/

			/**
			 * Makes empty keyword
			 */
			keyword_t() : name(""), num_args(-1), helptext("")
			{};

			/**
			 * Makes keyword from given arguments
			 *
			 * @param n   The name of this keyword
			 * @param na  Number of arguments
			 * @param h   Help text
			 */
			keyword_t(const std::string& n,
				const std::string& h, int na)
				: name(n), helptext(h), num_args(na)
			{};

			/*-----------*/
			/* operators */
			/*-----------*/

			inline keyword_t& operator= (const keyword_t& other)
			{
				/* copy and return */
				this->name = other.name;
				this->helptext = other.helptext;
				this->num_args = other.num_args;
				return *(this);
			};
	};

	/**
	 * The conf::command_t class is used to represent a single
	 * command from a .conf file
	 *
	 * A command consist of a keyword followed by zero or more
	 * arguments.
	 */
	class command_t
	{
		/* security */
		friend class reader_t;

		/* parameters */
		private:

			/**
			 * The command keyword
			 */
			std::string keyword;

			/**
			 * The list of arguments for this command
			 */
			std::vector<std::string> args;

		/* functions */
		public:

			/*-----------*/
			/* accessors */
			/*-----------*/

			/**
			 * Get the keyword
			 */
			inline const std::string& get_keyword() const
			{ return this->keyword; };

			/**
			 * Get the number of arguments
			 */
			inline size_t num_args() const
			{ return this->args.size(); };
				
			/**
			 * Get the i'th argument
			 */
			inline const std::string& get_arg(size_t i) const
			{ return this->args[i]; };

			/**
			 * Get the i'th argument as a templated type
			 */
			template <typename T> inline T get_arg_as(
					size_t i) const
			{
				std::stringstream ss;
				T a;

				/* get val */
				ss.str(this->get_arg(i));
				ss >> a;

				/* return it */
				return a;
			};

			/*-----------*/
			/* debugging */
			/*-----------*/
			
			/**
			 * Prints the value of this structure
			 * to the screen.
			 */
			void print() const;
	};
}

#endif
