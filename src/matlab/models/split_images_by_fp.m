%
% SPLIT_IMAGES_BY_FP( FLOORPLANFILE, CAMERAPOSEFILE, OUTPUTDIR )
%
% This function splits the images from a camera pose file and groups them
% based on the room that the floorplan thinks that they are in.
%
%   Inputs:
%       FLOORPLANFILE :
%           This is a .fp file that contains the floorplan.
%
%       CAMERAPOSEFILE :
%           This is the camera pose .txt file.  The camera pose file is
%           expected to be in the form that each line is an :
%               imageidx, x, y, z, roll, pitch, yaw
%
%       OUTPUTDIR :
%           Specifies the directory where the files will be saved.  If this
%           does not exist then it will be created for you
%
%
%   Outputs:
%       N files are created and placed in the OUTPUTDIR.  Each file
%       contains the imageidx of the images that were taken from inside
%       that room.
%
%
%   Last Edit :
%       Nicholas Corso 12/1/14
%
function split_images_by_fp(floorplanFile, cameraposeFile, outputDir)

% Read in the fp file
fp = read_fp(floorplanFile);

% Read the camera poses
poses = load(cameraposeFile);

% Make sure the output directory exists
if(~exist(outputDir,'dir'))
   mkdir(outputDir); 
end

% Strip the structure
cameraIdx = poses(:,1);
cameraX = poses(:,2);
cameraY = poses(:,3);

% Go through each room testing the images and see if they are in the
% polygon that denotes each room
for i = 1:1:fp.num_rooms
    
    % Collect all the triangles that are in this room
    roomtris = fp.tris(fp.room_inds==i,:);
    
    % Set the mask to be all false
    mask = false(numel(cameraIdx),1);
    
    for j = 1:1:size(roomtris,1)
        
        roomX = fp.verts(roomtris(j,:),1);
        roomY = fp.verts(roomtris(j,:),2);
        
        % Find the images that are in this room
        mask = mask | inpolygon(cameraX, cameraY, roomX, roomY);
    
    end
    
    % Mask out the cameraIdx that are in this room
    thisRoomIdx = cameraIdx(mask);
    
    % Create the output file
    thisRoomOutFile = fullfile(outputDir, sprintf('room_%03d.txt', i-1));
    f = fopen(thisRoomOutFile, 'w');
    
    % Write the indices to the file
    for j = 1:1:numel(thisRoomIdx)
       fprintf(f, '%06d\n', thisRoomIdx(j)); 
    end
    
    % Close this room's file
    fclose(f);
    
    % Mask down the cameraIdx, cameraX, and cameraY to what remains
    cameraIdx = cameraIdx(~mask);
    cameraX = cameraX(~mask);
    cameraY = cameraY(~mask);
end


end