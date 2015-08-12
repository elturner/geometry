#
# 	backpackconfig.py
#
#	This module gives an interface for working with backpack config xml files
#
#	It allows a user to easily travel through the xml hierarchy associated
#	with a tree of .xml config files
#

import os
import sys
import xml.etree.ElementTree as ET

class Configuration :

	#
	#	def __init__(self, filename, trimByEnable=True, relativeDir=None)
	#
	# 	The constructor for the configuration file
	#	parses the tree of xml files importing all of the settings
	# 	into a single element tree
	#
	#	If you give relativeDir it resolves all relative paths
	# 	to that directory, if it is not given then all paths
	#	are assumed to be relative to the file they are in
	#
	#	Also optionally you can specify to trim the thing by an enable
	#	flag
	#
	def __init__(self, filename, trimByEnable=True, relativeDir=None) :

		# Records the filename used when importing
		self.filename = filename
		self.relativeDir = relativeDir

		# Set the flag for if there was an error
		#
		#	0 - No error
		#	1 - Unable to parse a file
		#	2 - Circular File Tree Detected, Branch was aborted
		#
		# 	This is actually bit packed so you need to and
		# 	the value stored here with the error code above
		#	to detect the problem case
		#
		self.wasError, self.tree = parse_files(filename, relativeDir, trimByEnable)

	#
	#	def dump()
	#
	# 	Dumps the tree for debugging
	#
	def dump(self) :
		ET.dump(self.tree)

	#
	#	def circular_file_tree()
	#
	#	Returns a boolean that indicates if the xml file tree was
	#	circularly defined
	#
	def circular_file_tree(self) :
		return (self.wasError | 2) == 2

	#
	#	def import_error(self) :
	#
	#	Returns if there was an error importing a childs
	#	xml files somewhere in the backpack xml tree
	#
	def import_error(self) :
		return (self.wasError | 1) == 1

	#
	#	def find(self, findstr)
	#
	#	Wraps ETs find functionality
	#
	def find(self, findstr) :
		return self.tree.getroot().find(findstr)

	#
	#	def findall(self, findstr)
	#
	#	Wraps ETs findall functionality
	#
	def findall(self, findstr) :
		return self.tree.getroot().findall(findstr)

	#
	#	def hardware_version(self)
	#
	#	Returns the hardware version string
	#
	def hardware_version(self) :
		hardwareVersion = self.tree.getroot().find("./hardware/version")
		if hardwareVersion != None :
			return hardwareVersion.text.strip()

	#
	#	def hardware_codename(self)
	#
	#	Returns the codename string of the hardware
	#
	def hardware_codename(self) :
		hardwareCodename = self.tree.getroot().find("./hardware/codename")
		if hardwareCodename != None :
			return hardwareCodename.text.strip()

	#
	#	def find_sensor(self, sensorName, sensorType="*")
	#
	#	Finds a specific sensors tree based on a type and a sensor name.
	#	A type can be specified to speed up the search or resolve 
	#	name conflicts between different sensor types
	#
	def find_sensor(self, sensorName, sensorType="*") :
		sensorList = self.tree.getroot().findall("./"+sensorType)
		for sensorType in sensorList :
			for sensor in sensorType :
				nameElement = sensor.find("./name")
				if nameElement != None :
					if nameElement.text.strip() == sensorName :
						return sensor
		return None

	# 
	#	def find_sensors_by_type(self, sensorType)
	#
	#	Returns a list of all sensors names who have a specific sensor type
	#
	def find_sensors_by_type(self, sensorType) :
		sensorList = self.tree.getroot().findall("./"+sensorType)
		retList = []
		for sensorType in sensorList :
			for sensor in sensorType :
				nameElement = sensor.find("./name")
				if nameElement != None :
					retList.append(nameElement.text.strip())

		# Return the list
		return retList

	#
	#	def find_sensor_prop(self, sensorName, sensorProp, sensorType="*")
	#
	#	Looks up a sensor property based on sensor name.
	#
	#	sensorName  -  	specifies the name of the sensor from the configuration
	#				   	file
	#	sensorProp 	- 	what property you want to look for.  This can be a path
	#					like object such as "configFile/fps" for properties that
	#					are nested more levels down
	#	sensorType	-	Specifies what type of sensor you are looking for to cut
	#					down on search time or to differentiate between different
	#					types of sensor with the same name.
	#	
	#	Returns None if the property is not found 
	#
	def find_sensor_prop(self, sensorName, sensorProp, sensorType="*") :
		
		# Find the sensor
		sensor = self.find_sensor(sensorName, sensorType)
		if sensor == None :
			return None

		# Find the property
		prop = sensor.find("./"+sensorProp)
		if prop == None :
			return None

		# Return the text
		if prop.text == None :
			return None
		return prop.text.strip()


#
# def parse_files(filename, relativeDir, trimByEnable)
#
# Runs the actual import code
#
def parse_files(filename, relativeDir, trimByEnable) :

	# First we need to open the initial file
	try :
		tree = ET.parse(filename)
	except :
		return 1, None

	# If we want to trim by enable we do this now
	if trimByEnable :
		trim_by_enable(tree.getroot())

	# Next we do the actual imports
	filename = os.path.abspath(filename)
	seenFiles = [filename]
	error, root = recursive_import(tree.getroot(), seenFiles, 0, filename, relativeDir)

	# Return the tree and the error status
	return error, tree

#
#	def recursive_import(thisElem, seenFiles. lastError, parentFile, relativeDir)
#
#	The actual recursive part
#
def recursive_import(thisElem, seenFiles, lastError, parentFile, relativeDir) :

	# The first thing we do is check if this particular element
	# needs has an xml file associated with it
	thisText = thisElem.text
	if thisText == None :
		thisExt = ""
	else :
		thisText = thisText.strip()
		thisFile, thisExt = os.path.splitext(thisText)

	if thisExt.lower() == '.xml' :

		# Attempt to import the xml file
		try :
			
			# Filepaths are assumed to be relative, so we tack
			# it onto the parentFiles path
			if relativeDir == None :
				head, tail = os.path.split(parentFile)
			else :
				head = relativeDir
			newParentFile = os.path.normpath(os.path.join(head, thisFile+thisExt))

			newRoot = ET.parse(newParentFile).getroot()

			# Record that we saw this file if we have not seen it already
			if newParentFile in seenFiles :
				return (lastError | 2), thisElem
			else :
				seenFiles.insert(0, newParentFile)

			# Insert this into the tree at the current place
			clone_tree(newRoot, thisElem)

			# Recurse over this subtree
			lastError, thisElem = recursive_import(newRoot, seenFiles, lastError, newParentFile, relativeDir)

		except Exception as e:
			return (lastError | 1), thisElem

	# Iterate the child elements of this element looking for other xml files
	for child in thisElem :
		lastError, thisElem = recursive_import(child, seenFiles, lastError, parentFile, relativeDir)

	return lastError, thisElem

#
#	def clone_tree(inroot, outroot)
#
def clone_tree(inroot, outroot) :

	for child in inroot :
		newOutRoot = ET.SubElement(outroot, child.tag, child.attrib)
		newOutRoot.text = child.text
		clone_tree(child, newOutRoot)

#
#	def trim_by_enable(root)
#
#	Trims the tree of any nodes and subnodes if they
#	contain a tag that is "enable" and it is set to
#	0
#
def trim_by_enable(root) :

	# Check if the child has an enable flag
	toRemove = []
	for child in root :
		enableElem = child.find("./enable")
		if enableElem != None :
			
			# if it is not enabled flag it for descruction it
			isEnabled = (int(enableElem.text) != 0) 
			if not isEnabled :
				toRemove.append(child)

	# Destory things to remove
	for child in toRemove :
		root.remove(child)

	# Recurse
	for child in root :
		trim_by_enable(child)



