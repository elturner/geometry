#!/usr/bin/python

##
# @file generate_tex.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief  Calls the generate_tex pipeline on a dataset
#
# @section DESCRIPTION
#
# Will call the generate_tex program on a dataset, 
# verifying that all input files exist, and that the output 
# directories are valid.
#

# Import standard python libraries
import os
import sys
import shutil
import subprocess

# Get the location of this file
SCRIPT_LOCATION = os.path.dirname(__file__)

# Import our python files
sys.path.append(os.path.join(SCRIPT_LOCATION, '..', 'files'))
import dataset_filepaths

# the following indicates the expected location of the generate_tex
# executable file
EXE_DIR = os.path.join(SCRIPT_LOCATION,'..','..','..','bin')
GEN_TEX_EXE     = os.path.abspath(os.path.join(EXE_DIR,  "generate_tex"))

# compatibility for windows
if sys.platform == 'win32':
	GEN_TEX_EXE += '.exe'

##
# The main function of this script, which is used to run the pipeline
#
# This call will run the generate_tex program, verifying inputs and
# outputs.
#
# @param dataset_dir      The path to the dataset to process
# @param name_of_dataset  The name of this dataset
#
# @return             Returns zero on success, non-zero on failure
def run(dataset_dir, name_of_dataset):

	# check that all required executables exist
	if not os.path.exists(GEN_TEX_EXE):
		print "Error! Could not find generate_tex executable:", \
		      XYZ2DQ_EXE
		return -1

	# get paths to input files
	configfile = dataset_filepaths.get_hardware_config_xml(dataset_dir)
	pathfile   = dataset_filepaths.get_madfile_from_name(dataset_dir, \
						name_of_dataset)
	fp_files   = dataset_filepaths.get_all_floorplan_files(dataset_dir)

	# prepare output directory
	docs_dir   = dataset_filepaths.get_docs_dir(dataset_dir)
	if not os.path.exists(docs_dir):
		os.makedirs(docs_dir)
	texfile    = dataset_filepaths.get_tex_file(dataset_dir, \
				name_of_dataset)
	
	# prepare arguments
	args = [GEN_TEX_EXE,'-c',configfile,'-p',pathfile,'-o',texfile]
	for f in fp_files:
		args += ['-f', f]

	# generate the .tex file
	print "##### generating dataset documentation #####"
	ret = subprocess.call(args, \
                      executable=GEN_TEX_EXE, cwd=dataset_dir, \
                      stdout=None, stderr=None, stdin=None, shell=False)
	if ret != 0:
		print "Error! Tex-file generation program returned",ret
		return -2

	# run pdflatex on the result
	(junk, texfile_local) = os.path.split(texfile);
	ret = subprocess.call( \
		['pdflatex', '-halt-on-error', texfile_local], \
		cwd=docs_dir, stdout=None, stderr=None, stdin=None, \
		shell=False)
	if ret != 0:
		print "Error! pdflatex returned",ret
		return -3

	# move the output pdf file to the root directory of the dataset
	pdffile_old = os.path.abspath(os.path.join(dataset_dir, docs_dir, \
				(name_of_dataset + '.pdf')))
	pdffile_new = os.path.abspath(os.path.join(dataset_dir, \
				(name_of_dataset + '.pdf')))
	if os.path.exists(pdffile_old):
		shutil.move(pdffile_old, pdffile_new)

	# success
	return 0

##
# The main function
#
# This will call the run() function using command-line arguments
#
def main():

	# check command-line arguments
	if len(sys.argv) != 3:
		print ""
		print " Usage:"
		print ""
		print "\t",sys.argv[0],"<path_to_dataset>",\
			"<name_of_dataset>"
		print ""
		print " Dependencies:"
		print ""
		print "\tThis program requires pdflatex"
		print ""
		sys.exit(1)

	# run this script with the given arguments
	ret = run(sys.argv[1], sys.argv[2])
	sys.exit(ret)

##
# Boilerplate code to call main function when used as executable
#
if __name__ == '__main__':
	main()
