function [] = stream_d_imager_scandata(filename, pc, Rtocommon, Ttocommon)
	% stream_d_imager_scandata(filename)
	%
	%	Will parse the data output of the Panasonic
	%	D-Imager and stream intensity values to a figure
	%
	% arguments:
	%
	%	filename -	The file to parse
	%	pc -		OPTIONAL. If true, will render pointcloud
	%

	% check arguments
    if(~exist('pc', 'var') || isempty(pc))
        pc = false;
    else
        pc = true;
        if(~exist('Rtocommon', 'var') || isempty(Rtocommon))
           Rtocommon = eye(3); 
        end
        if(~exist('Ttocommon', 'var') || isempty(Ttocommon))
           Ttocommon = [0;0;0]; 
        end
    end

	% set up figure
	figure();
	f = gcf;
	axis equal;
	hold off;
	set(f, 'renderer', 'opengl');
	title('Panasonic D-Imager', 'FontSize', 15);
	
	% open the file for reading in binary
	fid = fopen(filename, 'rb');
	if(fid < 0)
		error(['Unable to read file: ', filename]);
	end

	% check for magic number in the file, should be 'dimager'
	M = fread(fid, 8, 'schar');
	M_lookup = [100, 105, 109, 97, 103, 101, 114, 0]';
	if(any(M ~= M_lookup))
		% wrong file type (doesn't have magic number)
		fclose(fid);
		error('Unrecognized file type');
	end
	
	% read header information
	image_width  = fread(fid, 1, 'uint32');
	image_height = fread(fid, 1, 'uint32');
	fps          = fread(fid, 1, 'uint32');
	freq         = fread(fid, 1, 'uint32');
	num_scans    = fread(fid, 1, 'uint32');

	% read each scan
	index = 0;
	pts_per_scan = image_width * image_height;
	while(~feof(fid))
	
		% read in timestamp
		timestamp = fread(fid, 1, 'uint64');
		if(isempty(timestamp))
			continue;
		end
		index = index + 1;

		% read in pointcloud
		xdat = fread(fid, pts_per_scan, 'int16');
		ydat = fread(fid, pts_per_scan, 'int16');
		zdat = fread(fid, pts_per_scan, 'int16');
		ndat = fread(fid, pts_per_scan, 'uint16');
	
		% plot the next frame of data
		figure(f);
		
		if(pc)
			% render pointcloud
			[az,el] = view();
            
            points = Rtocommon*[xdat';ydat';zdat']+repmat(Ttocommon,1,numel(xdat));
            
			scatter3(points(1,:), points(2,:), points(3,:), 10, ndat, 'filled');
			view(az, el);
		else
			% get the next image frame
			M = reshape(ndat, image_width, image_height)';

			imagesc(M);
			title(['frame ', num2str(index)]); 
		end

		% render the frame
		drawnow;
	end

	% clean up
	fclose(fid);
end
