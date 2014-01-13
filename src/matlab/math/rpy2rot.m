% rpy2rot.m
% By: John Kua <jkua@eecs.berkeley.edu>
% Date: 3/18/2011
%
% Converts three extrinsic Euler angle rotations (roll/rx, pitch/ry, yaw/rz) 
% into a rotation matrix. Assumes a right handed coordinate system. By default, 
% composes the rotations in the order roll/rx, pitch/ry, yaw, rz and
% assumes the rotations are in radians.
%
% Input:
% - roll:   rotation about the x-axis
% - pitch:  rotation about the y-axis
% - yaw:    rotation about the z-axis
% - angles: alternately, provide the angles as a 3-vector
% - units:  (optional) 'd' for degrees or 'r' for radians (default)
% - order:  (optional) 3-vector specifying the composition order. 
%                      Default is [1 2 3].
%
% Output:
% - Rout:   3x3 rotation matrix
%

%function Rout = rpy2rot(roll, pitch, yaw, units, order)
%function Rout = rpy2rot(angles, units, order)
function Rout = rpy2rot(varargin)

%% Input handling

% Rotation values
if length(varargin{1}) == 1 && nargin >= 3
	roll = varargin{1};
	pitch = varargin{2};
	yaw = varargin{3};
    nRotArgs = 3;    
elseif numel(varargin{1}) == 3
	roll = varargin{1}(1);
	pitch = varargin{1}(2);
	yaw = varargin{1}(3);
    nRotArgs = 1;
else
	error('Euler angles must be provided as 3 values or a single 3-vector!');
end

% Units parameter
if nargin >= nRotArgs + 1
    units = varargin{nRotArgs + 1};
    if ~isempty(units)
        if lower(units(1)) == 'd'
            roll = roll/180*pi;
            pitch = pitch/180*pi;
            yaw = yaw/180*pi;
        elseif lower(units(1) ~= 'r')
            error('Invalid units! Must be ''d'' or ''r''!');
        end
    end
end

% Order parameter
if nargin >= nRotArgs + 2
    order = varargin{nRotArgs + 2};
    if isempty(order)
        order = [1 2 3];
    else
        if numel(order) == 3
            order = order(:);
        else
            error('Order parameter must be a 3 vector!')
        end
    end
else
    order = [1 2 3];    
end

% Check number of arguments
if nargin > nRotArgs + 2
    error('Too many input arguments!')
end


%% Generate Rotation Matrix        
cr = cos(roll);
sr = sin(roll);
R(:,:,1) = [1.0, 0.0, 0.0; 0.0, cr, -sr; 0.0, sr, cr];

cp = cos(pitch);
sp = sin(pitch);
R(:,:,2) = [cp, 0, sp; 0, 1, 0; -sp, 0, cp];

cy = cos(yaw);
sy = sin(yaw);
R(:,:,3) = [cy, -sy, 0; sy, cy, 0; 0, 0, 1];

for i = 1:3
    if i == 1
        Rout = R(:,:,order(i));
    else
        Rout = R(:,:,order(i))*Rout;
    end
end

end