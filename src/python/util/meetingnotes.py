#!/usr/bin/python

##
# @file   meetingnotes.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief  This program will create a new folder and start a new text doc
#
# @section DESCRIPTION
#
# Helper script for taking notes.  Will make a folder with today's
# date and named with the participants of the meeting.
#

import os
import sys
from datetime import date
import subprocess

## 
# The following constants are used by this program
#
SCRIPT_LOCATION = os.path.dirname(__file__)
NOTES_FILENAME  = 'notes.txt'
TEXT_EDITOR     = '/usr/bin/vim'

##
# The main function will be called on execution of this code
#
def main():

	# get the tag for this meeting
	tag = ''
	if len(sys.argv) == 2:
		tag = sys.argv[1] # pass on command-line
	else:
		tag = raw_input('Enter a tag for this meeting: ')

	# get current date
	t = date.today()

	# generate folder for meeting
	d = os.path.join(SCRIPT_LOCATION, \
			'%4d%02d%02d_%s' % (t.year, t.month, t.day, tag))
	if not os.path.exists(d):
		os.makedirs(d)

	# generate a notes file inside this folder
	n = os.path.abspath(os.path.join(d, NOTES_FILENAME))
	subprocess.call( [TEXT_EDITOR, n], executable=TEXT_EDITOR, \
			cwd=d, shell=True)

	# done
	return 0

##
# Boilerplate code to call main function
#
if __name__ == '__main__':
	main()
