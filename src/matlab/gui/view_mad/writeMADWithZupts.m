%--------------------------------------------------------------------------
% writeMADWithZupts
% coded by George Chen/Matt Carlberg
%
% Writes poses, zupts, and timestamps into .MAD file format used for 3d
% modeling and error comparisons.

function writeMADWithZupts(poses, timestamps, zupts, filename)
% Inputs: 
%   1. poses is a 6xN vector with each measurement specifying (x,y, z) and 
%   (roll, pitch, yaw).  Note (roll, pitch, yaw) are in radians.
%   2. timestamps is a length N vector with a timestamp for each pose.
%   3. zupts is a matrix specifying zupts.  Assuming there are M zupts
%   during data acquisition, the matrix can be a 2Mx1 vector, where
%   every other row is a zupt start time and every other row is a zupt end
%   time.  Alternatively, zupts ca also be a Mx2 vector, where the first
%   column specifies the zupt start times and the second column specifies
%   the zupt end times.
%   4. filename Filename of .mad file to be written.
%
% Output: writes poses and timestamps in MAD format to the specified
%         filename


fid = fopen(filename, 'w');


if(size(zupts, 2) == 2)
    numZupts = size(zupts, 1); 
    zupts = reshape(zupts', [2*size(zupts,1) 1]);
    fwrite(fid, numZupts, 'int32'); 
    fwrite(fid, zupts, 'double'); 
elseif(size(zupts, 2) == 1) 
    numZupts = floor(size(zupts, 1)/2); 
    fwrite(fid, numZupts, 'int32'); 
    fwrite(fid, zupts, 'double');  
else
    numZupts = 0; 
    fwrite(fid, numZupts, 'int32'); 
end

fwrite(fid, size(poses, 2), 'int32'); % numIMUMeas

for i = 1:size(poses, 2)
    fwrite(fid, timestamps(i), 'double');
    fwrite(fid, poses(1, i), 'double');
    fwrite(fid, poses(2, i), 'double');
    fwrite(fid, poses(3, i), 'double');
    fwrite(fid, poses(4, i)*180/pi, 'double');
    fwrite(fid, poses(5, i)*180/pi, 'double');
    fwrite(fid, poses(6, i)*180/pi, 'double');
end

fclose(fid);

end