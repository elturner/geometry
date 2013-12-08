function conf = read_hardware_config(filename)
	% conf = read_hardware_config(filename)
	%
	%	Reads the hardware xml config file
	%	and stores the data in the output
	%	structure
	%

	try
	   tree = xmlread(filename(filename ~= '"'));
	catch
	   error('Failed to read XML file %s.',filename);
	end

	% Recurse over child nodes. This could run into problems 
	% with very deeply nested trees.
	try
	   s = parseChildNodes(tree);
	catch
	   error('Unable to parse XML file %s.',filename);
	end

	% find the sensors child
	sensors = [];
	for i = 1:length(s)
		
		% check if this is the sensors child
		if(~strcmp(s(i).Name, 'sensors'))
			continue;
		end

		% save this child's list of children
		sensors = remove_comments_and_whitespace(s(i).Children);
		break;
	end

	% check if sensors were found
	if(isempty(sensors))
		error('No sensors are listed in XML file %s.',filename);
	end

	% iterate over sensors, populating our struct
	for i = 1:length(sensors)

		% get sensor type
		type = sensors(i).Name;

		% iterate over the individual sensors for this type
		for j = 1:length(sensors(i).Children)
			
			% populate conf with this sensor's properties
			for k = 1:length(sensors(i).Children(j).Children)
				
				% get field name
				name = sensors(i).Children(...
						j).Children(...
						k).Name;

				% get field data
				if(~isempty(sensors(i).Children(...
						j).Children(k).Children))
					data = strtrim(sensors(...
							i).Children(...
							j).Children(...
							k).Children(...
							1).Data);
				else
					data = '';
				end

				% store in conf
				eval(['conf.', type, '(j).', name, ...
				      ' = ''', data, ''';']);
			end
		end
	end

	% remake the transformations to matlab matrices and vectors
	conf = parse_transforms(conf);
end

% ----- Local function REMOVE_COMMENTS_AND_WHITESPACE ------
function node_list_p = remove_comments_and_whitespace(node_list)
	
	node_list_p = [];

	% iterate over nodes in list
	for i = 1:length(node_list)

		% check if this node is a comment
		if(strcmp(node_list(i).Name, '#comment'))
			% it's a comment, so ignore it
			continue;
		end

		% check if this is whitespace text
		if(strcmp(node_list(i).Name, '#text'))
			if(isempty(strtrim(node_list(i).Data)))
				continue;
			end
		end

		% add to our final list
		n = length(node_list_p)+1;
		node_list_p(n).Name = node_list(i).Name;
		node_list_p(n).Attributes = node_list(i).Attributes;
		node_list_p(n).Data = node_list(i).Data;
		node_list_p(n).Children = remove_comments_and_whitespace(...
		                          node_list(i).Children);
	end
end

% ----- Local function PARSECHILDNODES -----
function children = parseChildNodes(theNode)
	% Recurse over node children.
	children = [];
	if theNode.hasChildNodes
	   childNodes = theNode.getChildNodes;
	   numChildNodes = childNodes.getLength;
	   allocCell = cell(1, numChildNodes);
	
	   children = struct(             ...
	      'Name', allocCell, 'Attributes', allocCell,    ...
	      'Data', allocCell, 'Children', allocCell);

	    for count = 1:numChildNodes
	        theChild = childNodes.item(count-1);
	        children(count) = makeStructFromNode(theChild);
	    end
	end
end

% ----- Local function MAKESTRUCTFROMNODE -----
function nodeStruct = makeStructFromNode(theNode)
	% Create structure of node info.

	nodeStruct = struct(                        ...
	   'Name', char(theNode.getNodeName),       ...
	   'Attributes', parseAttributes(theNode),  ...
	   'Data', '',                              ...
	   'Children', parseChildNodes(theNode));

	if any(strcmp(methods(theNode), 'getData'))
	   nodeStruct.Data = char(theNode.getData); 
	else
	   nodeStruct.Data = '';
	end
end

% ----- Local function PARSEATTRIBUTES -----
function attributes = parseAttributes(theNode)
	% Create attributes structure.

	attributes = [];
	if theNode.hasAttributes
	   theAttributes = theNode.getAttributes;
	   numAttributes = theAttributes.getLength;
	   allocCell = cell(1, numAttributes);
	   attributes = struct('Name', allocCell, 'Value', ...
	                       allocCell);

	   for count = 1:numAttributes
	      attrib = theAttributes.item(count-1);
	      attributes(count).Name = char(attrib.getName);
	      attributes(count).Value = char(attrib.getValue);
	   end
	end
end
