function [mapData] = readMapData(mapFileName)
% [mapData] = readMapData(filename)
%
%	Read the specified *.dq file, and
%	store in the struct mapData
%

fid = fopen(mapFileName);
if(fid == -1) 
    error(['unable to read map file: ', mapFileName]);
end

% Read the header information
tline = fgets(fid);
mapData.maxDepth = str2num(tline);
tline = fgets(fid);
mapData.halfwidth = str2num(tline);
tline = fgets(fid);
mapData.rootNodePosition = str2num(tline)';
tline = fgets(fid);

% initialize the fields
mapData.pos = cell(0);
mapData.heightRange = cell(0);
mapData.numPoints = cell(0);
mapData.poseIdx = cell(0);

% Read the point information
while(tline ~= -1)
    
    % Parse the text string for numbers
    nline = str2num(tline);
    if(isempty(nline))
       continue; 
    end
    
    mapData.pos{end+1} = nline(1:2)';
    mapData.heightRange{end+1} = nline(3:4)';
    mapData.numPoints{end+1} = nline(5);
    if(nline(6) ~= 0)
        mapData.poseIdx{end+1} = nline(7:end);
    else
        mapData.poseIdx{end+1} = []; 
    end
 
    tline = fgets(fid);
end

% Convert the cells to matricies because fuck cells
mapData.pos = cell2mat(mapData.pos);
mapData.heightRange = cell2mat(mapData.heightRange);
mapData.numPoints = cell2mat(mapData.numPoints);

fclose(fid);
end
