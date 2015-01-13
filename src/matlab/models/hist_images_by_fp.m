function [counts,locations] = hist_images_by_fp(fp, camposefile, whitelist)
	% counts = HIST_IMAGES_BY_FP(fp, camposefile, whitelist)
	%
	%	Will compute a histogram of image locations, by counting
	%	how many images appear in each room of the given floorplan.
	%
	% arguments:
	%
	%	fp -		The floorplan to use to histogram the images
	%
	%	camposefile -	A camera pose file.
	%			This is the camera pose .txt file.  
	%			The camera pose file is
	%			expected to be in the form that each 
	%			line is an :
	%
	%			   imageidx, x, y, z, roll, pitch, yaw
	%
	%			Where (x,y,z) are in units of meters
	%			(roll,pitch,yaw) in radians
	%
	%	whitelist -	A whitelist of image indices to sort.
	%			If present, only images whose indices
	%			are in this list will be inserted into
	%			the histogram.
	%
	% output:
	%
	%	counts -	An array representing how many images
	%			are in each room.  This array is length
	%			equal to fp.num_rooms, and counts all
	%			images given that occur in each room.
	%
	% author:
	%
	%	Eric Turner <elturner@eecs.berkeley.edu>
	%	Created on January 13, 2015
	%

	% read in the camera pose file
	poses = load(camposefile);

	% Strip the structure
	cameraIdx = poses(:,1);
	cameraX = poses(:,2);
	cameraY = poses(:,3);
	use_cams = true(size(cameraIdx));

	% filter based on whitelist
	if(exist('whitelist', 'var') && ~isempty(whitelist))
		for i = 1:length(cameraIdx)
			% remove any indices not in the list
			use_cams(i) = any(whitelist == cameraIdx(i));
		end
	end
	cameraIdx(~use_cams) = [];
	cameraX(~use_cams)   = [];
	cameraY(~use_cams)   = [];
	locations = [cameraX, cameraY];

	% iterate through the rooms of the floorplan
	counts = zeros(fp.num_rooms, 1);
	for i = 1:1:fp.num_rooms
	
		% Collect all the triangles that are in this room
		roomtris = fp.tris(fp.room_inds==i,:);
	
		% Set the mask to be all false
		mask = false(numel(cameraIdx),1);

		% test intersection of camera poses with triangles
		% of this room
		for j = 1:1:size(roomtris,1)
			
			roomX = fp.verts(roomtris(j,:),1);
			roomY = fp.verts(roomtris(j,:),2);
		
			% Find the images that are in this room
			mask = mask | inpolygon(cameraX, cameraY, ...
						roomX, roomY);
		end

		% record how many cameras are in this room
		counts(i) = sum(mask);
		
		% Mask down the cameraIdx, cameraX, and 
		% cameraY to what remains
		cameraIdx = cameraIdx(~mask);
		cameraX = cameraX(~mask);
		cameraY = cameraY(~mask);
	end
end
