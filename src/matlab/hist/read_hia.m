function hia = read_hia(filename)
	% hia = READ_HIA(filename)
	%
	%	Imports a .hia file into a structure.
	%
	%	A Histogrammed Interior Area (HIA) file denotes
	%	the interior area discovered by an octree (or similiar
	%	structure) as a top-down 2D histogram.
	%
	% arguments:
	%
	%	filename -	The file to import
	%
	% output:
	%
	%	hia -	The output structure.  This structure has
	%		the following fields:
	%
	%		version_major	-	the major version number
	%		version_minor	-	the minor version number
	%
	%		level_index	-	the level index
	%
	%		num_cells	-	number of cells
	%	
	%		bounds		-	[ x_min x_max ;
	%					  y_min y_max ;
	%					  z_min z_max ]
	%
	%		resolution	-	the resolution (in meters)
	%
	%		centers		-	a num_cells x 2 matrix,where
	%					each row is [x,y] center of
	%					a cell (in meters).
	%
	%		heights		-	a num_cells x 2 matrix, 
	%					where each row is 
	%					[min_z, max_z] for that cell
	%					(measured in meters).
	%
	%		open_heights	-	A num_cells x 1 vector,
	%					where each element is the
	%					amount of elevation marked
	%					as interior (in meters).
	% 
	% author:
	%
	%	Written by Eric Turner <elturner@eecs.berkeley.edu>
	%	Created on February 27, 2015
	%

	% first, attempt to open as a binary file
	fid = fopen(filename, 'rb');
	if(fid < 0)
		error(['Unable to open file for reading: ', filename]);
	end
	
	% read in magic number
	M = fread(fid, 8, 'schar')';
	if(any(M ~= ['hiafile', 0]))
		fclose(fid)
		error(['Not a valid hiafile: ', filename]);
	end

	% read in header info
	hia.version_major = fread(fid, 1, 'int32');
	hia.version_minor = fread(fid, 1, 'int32');
	hia.level_index   = fread(fid, 1, 'int32');
	hia.num_cells     = fread(fid, 1, 'uint32');
	
	% read in bounding box (units: meters)
	hia.bounds        = zeros(3,2);
	hia.bounds(1,1)   = fread(fid, 1, 'double'); % min-x
	hia.bounds(2,1)   = fread(fid, 1, 'double'); % min-y
	hia.bounds(3,1)   = fread(fid, 1, 'double'); % min-z
	hia.bounds(1,2)   = fread(fid, 1, 'double'); % max-x
	hia.bounds(2,2)   = fread(fid, 1, 'double'); % max-y
	hia.bounds(3,2)   = fread(fid, 1, 'double'); % max-z

	% import resolution
	hia.resolution    = fread(fid, 1, 'double'); % units: meters
	
	% read in all cell information.
	% this call will populate matrix C in column order
	%
	% cells have five values:  
	%		center_x, center_y, min_z, max_z, open_height
	C                 = fread(fid, [5 hia.num_cells], 'double');
	hia.centers       = C(1:2,:)';
	hia.heights       = C(3:4,:)';
	hia.open_heights  = C(5,:)';

	% clean up
	fclose(fid);
end
