#ifndef CMD_ARGS_H
#define CMD_ARGS_H

/**
 * @file cmd_args.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This helper class is used to parse command-line arguments
 * for a program run through the terminal.  It allows an easy
 * interface for programs to specify which arguments are option,
 * required, and what values to look for with these arguments.
 */

#include <string>
#include <set>
#include <map>
#include <vector>
#include <sstream>

/* the following are classes defined in this file */
class cmd_args_t;
class cmd_tag_t;

/**
 * The cmd_args_t class specifies all command-line arguments
 *
 * This class houses the arguments that are parsable by this
 * program, and uses these to interpret the arguments given
 * by the user.
 */
class cmd_args_t
{
	/* parameters */
	private:

		/* list of tags the program is interested in
		 * searching for, mapped to the arguments parsed
		 * after this tag */
		std::map<std::string, cmd_tag_t> tags;

		/* if files are specified on the command line without
		 * tags before them, they will be stored here, organized
		 * by their filetype suffix. */
		std::map<std::string, std::vector<std::string> > files;

		/* this map indicates the list of required files to
		 * be presented on the command-line, based on file extension
		 * along with the minimum number of each type of 
		 * file required. */
		std::map<std::string, int> required_file_types;

	/* functions */
	public:

		/**
		 * Initializes empty structure 
		 */
		cmd_args_t();

		/**
		 * Frees all memory and resources
		 */
		~cmd_args_t();

		/**
		 * Adds tag information to this structure
		 *
		 * Will add a tag to look for when parsing the command-
		 * line arguments.  This tag can be followed by zero or
		 * more values, which will also be stored.
		 *
		 * @param t    The tag string.
		 * @param d    The english description of this tag.
		 * @param o    Indicates if tag is optional. Default true.
		 * @param n    Num args expected after tag. Default zero.
		 */
		void add(const std::string& t, const std::string& d,
		         bool o = true, int n = 0);

		/**
		 * Adds a required file type to this structure
		 *
		 * When parsing the command-line, an error will be
		 * thrown if fewer than this many instances of the
		 * specified file type are provided.
		 *
		 * @param ext   The extension to look for when parsing
		 * @param m     The minimum number of occurances of ext
		 */
		void add_required_file_type(const std::string& ext,
		                            int m = 1);

		/**
		 * Parses the specified command-line arguments
		 *
		 * Will check the specified command-line arguments for
		 * the existing tags stored in this structure.  Will record
		 * which tags were observed on the command-line and what
		 * their arguments were.
		 *
		 * If one or more non-optional tags are missing, then
		 * will print usage information and return failure.
		 *
		 * @param argc   The number of command-line arguments
		 * @param argv   The array of command-line arguments
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int parse(int argc, char** argv);

		/**
		 * Prints usage information based on existing tags
		 *
		 * Given the added tags, will print usage information
		 * to stderr.
		 *
		 * @param prog_name   The name of this program
		 */
		void print_usage(char* prog_name) const;

		/**
		 * Retrieves whether this tag was parsed and its values
		 *
		 * Checks if a specific tag was parsed.  If that tag
		 * was parsed, then it will populate the specified
		 * vector with the values that succeeded that tag.
		 *
		 * @param tag    The tag to check.
		 * @param vals   Where to store the values for this tag.
		 *
		 * @return       Returns true iff this tag was seen.
		 */
		bool tag_seen(const std::string& tag) const;
		bool tag_seen(const std::string& tag,
		              std::vector<std::string>& vals) const;

		/**
		 * Retrieves the n'th value associated with the given tag
		 *
		 * Given a particular command-line tag, this function
		 * will return the n'th value associated with that tag.
		 * If the tag was unseen or has fewer than n values,
		 * then this function will return the empty string.
		 *
		 * @param t    The tag to reference
		 * @param n    The index of the value to return
		 *
		 * @return     Returns the n'th value of tag t
		 */
		std::string get_val(const std::string& t,
		                    unsigned int n=0) const;
		
		/**
		 * Retrieves the n'th value as the specified type
		 *
		 * Operates in the same manner as get_val(), but will
		 * return the value as the specified type.
		 *
		 * @param t    The tag to reference
		 * @param n    The index of the value to return
		 *
		 * @return     Returns the n'th value of tag t
		 */
		template <typename T> inline T get_val_as(
		                       const std::string& t,
		                       unsigned int n=0) const
		{
			std::stringstream ss;
			T ret;

			/* store the value as a string */
			ss << this->get_val(t, n);

			/* return as type */
			ss.seekg (0, ss.beg);
			ss >> ret;
			return ret;
		}

		/**
		 * Gets all the files specified of the given format
		 *
		 * Populates the list provided with all the filenames
		 * for the specified file format that were present
		 * on the command-line.
		 *
		 * If the format specified is an empty string, then
		 * all files of all formats will be given.
		 *
		 * @param format     The file format to filter by
		 * @param files      The list of filenames to populate
		 */
		void files_of_type(const std::string& format,
		                   std::vector<std::string>& files) const;

	/* helper functions */
	private:

		/**
		 * Find the file extension for the given filename
		 *
		 * Parses the file extension out of the specified file
		 * name.  If the given string is not a file, or if no
		 * extension can be found, will return the empty string.
		 *
		 * @param fn   The file name to parse
		 *
		 * @return     Returns the extension of this file, or ""
		 */
		static std::string file_extension(const std::string& fn);

		/**
		 * Writes the given line to cerr with specified indent
		 *
		 * Writes the given line to cerr.  If the line exceeds
		 * standard page width, then it will be broken into
		 * multiple lines, where each additionaly line will
		 * be given the specified indent.
		 *
		 * @param line    The line to write
		 * @param indent  The num spaces indent for additional lines
		 */
		static void write_line_with_indent(const std::string& line,
		                                   int indent);
};

/**
 * The cmd_tag_t class represents a single tag
 *
 * This tag can appear on the command-line followed
 * by zero or more values.  This tag can be optional
 * or required.
 */
class cmd_tag_t
{
	/* the primary class can access these parameters */
	friend class cmd_args_t;

	/* parameters */
	private:

		std::string tag; /* the tag string searched for */
		std::string description; /* description of tag */
		
		bool optional; /* indicates if tag is optional */
		bool found; /* indicates if tag was found on cmd-line */
		
		int num_vals; /* number of values to expect after tag */

		std::vector<std::string> found_vals; /* values parsed
		                                      * from cmd line */

	/* functions */
	public:

		/**
		 * Constructor initializes empty tag.
		 */
		cmd_tag_t();

		/**
		 * Frees all memory and resources.
		 */
		~cmd_tag_t();

		/**
		 * Initializes this tag based on information provided.
		 *
		 * This function initializes this tag structure to
		 * indicate the values expected by this tag and what
		 * this tag represents to the program.
		 *
		 * @param t    The tag string.
		 * @param d    The english description of this tag.
		 * @param o    Indicates if tag is optional.
		 * @param n    Num args expected after tag.
		 */
		void init(const std::string& t, const std::string& d,
		          bool o, int n);

		/* operators */

		inline cmd_tag_t operator = (const cmd_tag_t& rhs)
		{
			/* change each field */
			this->tag = rhs.tag;
			this->description = rhs.description;
			this->optional = rhs.optional;
			this->found = rhs.found;
			this->num_vals = rhs.num_vals;
			this->found_vals.clear();
			this->found_vals.insert(this->found_vals.begin(),
			                        rhs.found_vals.begin(),
			                        rhs.found_vals.end());

			/* return the changed structure */
			return (*this);
		};
};

#endif
