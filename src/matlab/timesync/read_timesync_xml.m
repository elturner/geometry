%
% syncinfo = read_timesync_xml(filename)
%
% This function reads in the timesync xml file and returns the slope and
% offset of each sensor in a struct
%
function syncinfo = read_timesync_xml(filename)

% Read in the xml file 
xml_struct = parseXML(filename(filename ~= '"'));

% Next we strip out all of the empty text fields and comment fields
xml_struct = deleteChildrenTextNodes(xml_struct);
xml_struct = deleteChildrenCommentNodes(xml_struct);

% Copy the data into the output structure
sensor_nodes = xml_struct.Children;
for i = 1:1:numel(sensor_nodes)
    
    syncinfo(i).name = sensor_nodes(i).Name;
    for j = 1:1:numel(sensor_nodes(i).Children)
       eval(['syncinfo(i).' sensor_nodes(i).Children(j).Name '='...
           'str2num(sensor_nodes(i).Children(j).Children.Data);']);
    end
    
end

end



%
% HELPER FUNCTIONS
%
function thisNode = deleteChildrenTextNodes(thisNode)

    % Check for stopping condition
    if(isempty(thisNode.Children))
        return
    end
    
    % Loop over the children of this node and delete any all
    % whitespace text nodes
    mask = true(size(thisNode.Children));
    for i = 1:1:numel(thisNode.Children)
       if(strcmp(thisNode.Children(i).Name,'#text') && ...
           isempty(strtrim(thisNode.Children(i).Data)))
            mask(i) = false;
       end
    end
    thisNode.Children = thisNode.Children(mask);
    
    % Loop over the children recursing
    for i = 1:1:numel(thisNode.Children)
       thisNode.Children(i) = deleteChildrenTextNodes(thisNode.Children(i)); 
    end
    
end

function thisNode = deleteChildrenCommentNodes(thisNode)

    % Check for stopping condition
    if(isempty(thisNode.Children))
        return
    end
    
    % Loop over the children of this node and delete any all
    % whitespace text nodes
    mask = true(size(thisNode.Children));
    for i = 1:1:numel(thisNode.Children)
       if(strcmp(thisNode.Children(i).Name,'#comment'))
            mask(i) = false;
       end
    end
    thisNode.Children = thisNode.Children(mask);
    
    % Loop over the children recursing
    for i = 1:1:numel(thisNode.Children)
       thisNode.Children(i) = deleteChildrenCommentNodes(thisNode.Children(i)); 
    end
    
end


function theStruct = parseXML(filename)
% PARSEXML Convert XML file to a MATLAB structure.
try
   tree = xmlread(filename);
catch
   error('Failed to read XML file %s.',filename);
end

% Recurse over child nodes. This could run into problems 
% with very deeply nested trees.
try
   theStruct = parseChildNodes(tree);
catch
   error('Unable to parse XML file %s.',filename);
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