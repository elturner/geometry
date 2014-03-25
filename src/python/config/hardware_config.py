##
# config_file.py
# Nicholas Corso <ncorso@eecs.berkeley.edu>
#
# This class provides an interface for the backpack config files.
# 
#

# Imports needed
import xml.etree.ElementTree as ET
import os
import sys
import shutil
import xml.dom.minidom
import re

##
# The actual config_file class
#
class ConfigFile:

	##
	#  __init__(self, config_file_name)
	#
	# Initialization function
	def __init__(self, config_file_name) :

		# Read in the input config file
		try :
			self.xml_tree =  ET.parse(config_file_name)
			self.xml_root = self.xml_tree.getroot()
		except :
			self.xml_tree = [];
			self.xml_root = [];

	##
	# def list_sensors(self)
	#
	# List The Sensors in the XML File.  This function
	# returns a newline seperated list that begins with the number
	# of sensors found.  Following that is a list of tipples 
	# where each triple of three lines represents a single sensor.  
	# Each tripple contains the following :
	#
	# sensor_type
	# sensor_name
	# sensor_is_enabled
	def list_sensors(self) :

		# String to hold the serialized representation of the 
		# sensors
		serialized_string_list = ""
		num_sensors_seen = 0;

		# Check to make sure we at least have data in the tree
		if self.xml_root == [] :
			return str(num_sensors_seen) + '\n' + serialized_string_list

		# List all the types of sensors in the xml file
		sensor_types = self.xml_tree.findall("./*");
		for sensor_type in sensor_types :

			# Find all instances of that type of sensor
			sensor_instances = sensor_type.findall("./*")
			
			# For Each Sensor Instance we should look to see if it has
			# a <name> attribute and then store that information
			for sensor_instance in sensor_instances :

				# Query the block for the sensor type, name, and if it is currently
				# enables
				sensor_instance_type = sensor_type.tag
				sensor_instance_name = sensor_instance.findall("./name")[0].text
				sensor_instance_is_enabled = sensor_instance.findall("./enable")[0].text

				# Push it into the string
				serialized_string_list += sensor_instance_type + '\n'
				serialized_string_list += sensor_instance_name + '\n'
				serialized_string_list += sensor_instance_is_enabled + '\n'
				num_sensors_seen += 1

		# Return the string to the caller
		return str(num_sensors_seen) + '\n' + serialized_string_list

	##
	# def update_sensors(self, serialized_string_list)
	#
	# This function updates the internal element tree
	# using the serialized string given.  Returns
	# True on success and False on failure
	def update_sensors(self, serialized_string_list) : 
		
		# Check to make sure we actually have data in the
		# class
		if self.xml_root == [] :
			return False

		# We split the string into an array for easier
		# processing
		sensor_array = serialized_string_list.split()

		# The first arguement should always be the number of
		# sensors in the config file
		num_sensors = int(sensor_array[0])
		
		# Loop over the sensors setting the enable
		for idx in range(0,num_sensors) :

			# Strip out what I care about
			sensor_triple = sensor_array[(1+idx*3):(4+idx*3)]
			
			# Attempt to alter the state of the xml file
			sensor_info_array = [sensor_triple[0]];
			sensor_info_array.insert(0,'.')
			sensor_info_array.insert(len(sensor_info_array),'*')

			# Find all instances of this type of sensor
			sensor_instances = self.xml_tree.findall('/'.join(sensor_info_array))

			# Check to make sure this is non empty
			if sensor_instances == [] :
				return False;

			# Loop over the returned sensors looking for the one with the correct
			# name
			for sensor_instance in sensor_instances :

				sensor_instance_name = sensor_instance.findall("./name")[0].text
				
				# If we found the correct name then update the enable bit
				if sensor_instance_name == sensor_triple[1] :
					sensor_instance.findall("./enable")[0].text = sensor_triple[2]

		# Return Success
		return True;

	##
	# Reprints the config xml to file
	#
	def reprint_xml(self, out_file_name) :	

		# Write the settings XML file to the out
		xmlAsString = ET.tostring(self.xml_root);
		xmlAsString = xmlAsString.replace('\n','');
		doc = xml.dom.minidom.parseString(xmlAsString);
		uglyXml = doc.toprettyxml(indent='  ')
		text_re = re.compile('>\n\s+([^<>\s].*?)\n\s+</', re.DOTALL)    
		prettyXml = text_re.sub('>\g<1></', uglyXml)
		f = open(out_file_name,'w');
		f.write(prettyXml);
		f.close();
	
		# Ensure that the file wrote
		if not os.path.exists(out_file_name):
			return False;
		
		return True;	



		


# Allow me to do the some checks on this class
def test_function():

	# Test file names
	test_file_in = "C:\\Projects\\indoormapping\\magneto\\config\\backpack_config.xml"
	test_file_out = "C:\\Projects\\indoormapping\\magneto\\config\\backpack_config_out.xml"

	# Read the config file
	x = ConfigFile(test_file_in)
	
	# Obtain the listing of sensors
	sensor_list =  x.list_sensors()

	# Update the list of sensors with some test conditions
	sensor_list = sensor_list.split()
	sensor_list[9] = '0'

	# Update the sensor list
	x.update_sensors('\n'.join(sensor_list))
	
	# Reprint the xml
	x.reprint_xml(test_file_out)	



if  __name__ =='__main__':test_function()