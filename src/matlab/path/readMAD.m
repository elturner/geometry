%--------------------------------------------------------------------------
% readMAD
% coded by George Chen
%
% Parses Matt Carlberg's MAD file format.
%
% Input: MAD filename
%
% Output:
% - poses: in format [x; y; z; roll; pitch; yaw], angles in *RADIANS*
% - timestamps
% - zuptIntervals where each row specifies a begin and end time

function [poses, timestamps, zuptIntervals] = readMAD(filename)

fid = fopen(filename, 'r');

numZupts = fread(fid, 1, 'int32');

zuptIntervals = zeros(numZupts, 2);

for i = 1:numZupts
    zuptIntervals(i, :) = fread(fid, 2, 'double');
end

numIMUMeas = fread(fid, 1, 'int32');
poses      = zeros(6, numIMUMeas);
timestamps = zeros(numIMUMeas, 1);

for i = 1:numIMUMeas
    timestamps(i) = fread(fid, 1, 'double');
    poses(:, i)   = fread(fid, 6, 'double');
end
poses(4:6, :) = poses(4:6, :)*pi/180; % convert degrees to radians

fclose(fid);

end