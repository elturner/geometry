% decomposeRotationMatrix - This function uses Givens rotations to solve
% for the Euler angles given a rotation matrix. 
%
% Written by: John Kua <jkua@eecs.berkeley.edu>
% Date: 4/14/2011
%
% Input: R - this is an 3x3xN set of rotation matrices
%
% Output: rotations - this is a 3xN matrix where the first row is roll, the
%               second row is pitch and the third row is yaw. Units are in
%               radians
%
%         roll, pitch, yaw - each of the Euler angles in radians
%
function [varargout] = decomposeRotationMatrix(R, debug)

if ~exist('debug', 'var')
    debug = 0;
end

roll = zeros(1, size(R, 3));
pitch = zeros(1, size(R, 3));
yaw = zeros(1, size(R, 3));

for i = 1:size(R, 3)
    [r, G, c, s, theta] = givensRotation(R(1,1,i), R(2,1,i));
    
    Ryaw = [c s 0; -s c 0; 0 0 1];
    yaw(i) = theta;
    
    [r, G, c, s, theta] = givensRotation(r, R(3,1,i));
    
    Rpitch = [c 0 s; 0 1 0; -s 0 c];
    pitch(i) = -theta;
    
    tempR = Rpitch * Ryaw * R(:,:,i);
    
    [r, G, c, s, theta] = givensRotation(tempR(2, 2), tempR(3,2));
    
    Rroll = [1 0 0; 0 c s; 0 -s c];
    roll(i) = theta;
    
    Q = Rroll * tempR;
    temp = abs(Q);
    temp([1 5 9]) = 0;
    test = sum(sum(temp));
    if any(diag(Q) ~= [1; 1; 1]) && test > 0.001
        Q
        error(['Decomposition failed! Is R(' int2str(i) ') really a rotation matrix? decomposeRotationMatrix()'])
    end
    
    if debug
        disp([int2str(i) ':: Roll: ' num2str(roll(i)/pi*180) ' degrees | Pitch: ' num2str(pitch(i)/pi*180) ' degrees | Yaw: ' num2str(yaw(i)/pi*180) ' degrees'])
    end
    
end

if nargout <= 1
    varargout(1) = {[roll; pitch; yaw]};
else
    varargout(1) = {roll'};
    varargout(2) = {pitch'};
    varargout(3) = {yaw'};
end