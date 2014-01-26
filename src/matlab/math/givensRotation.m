% givensRotation.m - This function solves for a Givens rotation such that
%                    given a vector [a; b], the Given rotation G of the
%                    form [c s; -s c], where c = cos(theta) and s =
%                    sin(theta), transforms the input vector to [r; 0]
%
%                    G * [a; b] = [r; 0]
%                    [c s; -s c] * [a; b] = [r; 0];
%
%                    This implementation is based on the algorithm given
%                    in the Wikipedia article on Givens Rotations:
%                    http://en.wikipedia.org/wiki/Givens_rotation


function [r G c s theta] = givensRotation(a, b);

if b == 0
%   then {c ? copysign(1,a); s ? 0; r ? abs(a)}
    c = 1*sign(a);
    s = 0;
    r = abs(a);
elseif a == 0
%   then {c ? 0; s ? copysign(1,b); r ? abs(b)}
    c = 0;
    s = 1*sign(b);
    r = abs(b);
elseif abs(b) > abs(a)
%   then
%   t ? a/b
%   u ? copysign(sqrt(1+t*t),b)
%   s ? 1/u
%   c ? s*t
%   r ? b*u
    t = a / b;
    u = sqrt(1+t^2) * sign(b);
    s = 1 / u;
    c = s * t;
    r = b * u;
else
%   t ? b/a
%   u ? copysign(sqrt(1+t*t),a)
%   c ? 1/u
%   s ? c*t
%   r ? a*u
    t = b / a;
    u = sqrt(1+t^2) * sign(a);
    c = 1 / u;
    s = c * t;
    r = a * u;
end

G = [c s; -s c];
theta = atan2(s, c);

test = G * [a; b];

if abs(test(2)) > 1e-10
    test(2)
    error('Givens Rotation computation failed! givensRotation()')
end
    
