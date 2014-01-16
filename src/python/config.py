import xml.etree.ElementTree as ET

##
# @file config.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
#
# @section DESCRIPTION
#
#	This file defines functions that can parse
#	xml settings files to extract relevant information
#	for this python program
#

##
# Will extract configuration from xml settings file
#
# Given a xml settings file path, will parse that file and
# will return the configuration information.
#
# @param filename The location of the file to parse
#
# @return         Returns the configuration values as a dictionary
#                 Returns None on failure
#
def parse_settings_xml(filename):
	
	# catch file-not-found exceptions
	try:
		# parse file as xml tree
		tree = ET.parse(filename)
		root = tree.getroot()

		# verify that this is a settings file
		if root.tag != 'settings':
			return None

		# initialize settings
		settings = {}

		# iterate over settings, storing each
		for child in root:
			settings[child.tag.strip()] = child.text.strip()

		# return success
		return settings
	
	except:
		return None

##
# Will extract backpack configuration from xml settings file
# 
# The backpack configuration xml file stores each sensor type
# and its characteristics.  This information will be (partly)
# parsed and stored in the output
#
# @param filename The location of the file to parse
#
# @return         Returns list of sensor types and configuration files
#                 as tuple of arrays of strings
#                 Returns None on error
#
def parse_backpack_xml(filename):

	# catch i/o exceptions and bad formatting
	try:

		# parse file as xml tree
		tree = ET.parse(filename)
		sensors = tree.getroot()
	
		# check valid file
		if sensors.tag != 'sensors':
			return None

		# initialize lists
		sensor_types   = []
		sensor_configs = []

		# parse the file
		for group in sensors: # iterate over sensor type groups
			for sensor in group: # iterate over sensors
				
				# check if this sensor is enabled
				if sensor.find('enable').text.strip() == '0':
					# not enabled, skip it
					continue

				# add to list
				sensor_types.append(sensor.tag.strip())
				sensor_configs.append(
					sensor.find(
					'configFile').text.strip())

		# return lists
		return (sensor_types, sensor_configs)

	except:
		return None

