#!/usr/bin/python

##
# @file emailer.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief  This program calls a subprocess, then sends an email when complete
#
# @section DESCRIPTION
#
# This python script allows a user to run a process, as if calling that
# process directly on the shell.  However, when the process is completed,
# this program will send an email to the given gmail account.
#

import sys
import socket
import timeit
import subprocess
import smtplib
import getpass
from email.mime.text import MIMEText

##
# The main function will be called on execution of this code
#
def main():

	# we want to run a subprocess from this program, which is
	# specified through the command-line arguments
	args = sys.argv[1:] # ignore first argument: program name

	# before we execute this subprocess, prepare the email that
	# will be sent when the subprocess completes.  To do this,
	# we need the credentials
	handle = raw_input('What is your gmail username ' \
			+ '(without the @gmail.com)? ')
	passwd = getpass.getpass()
	host = socket.gethostname()

	# run the process
	print ""
	print "Running command: " + (" ".join(args))
	print ""
	tic = timeit.default_timer()
	ret = subprocess.call(args, shell=False)
	toc = timeit.default_timer()

	# format the message
	email = handle + '@gmail.com'
	msg = MIMEText("This is an automated message.  Your process:\n\n" \
		+ "\t" + (" ".join(args)) + "\n\n" \
		+ "running on the computer '" + host \
		+ "' has terminated with exit code " + str(ret) + ".  " \
		+ "It took " + str(toc-tic) + " sec to complete.\n\n\n\n" \
		+ "------------------------------------------\n" \
		+ "This message sent by the emailer.py script\n" \
		+ "by Eric Turner: elturner@eecs.berkeley.edu\n");
	msg['Subject'] = "Your process on " + host + " has terminated"
	msg['From']    = email
	msg['To']      = email

	# send the message
	s = smtplib.SMTP('smtp.gmail.com:587')
	s.starttls()
	s.login(handle, passwd)
	s.sendmail(email, [email], msg.as_string())
	s.quit()

##
# Boilerplate code to call main function
#
if __name__ == '__main__':
	main()
