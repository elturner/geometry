function R = quat2rot(q)
    % R = QUAT2ROT(q)
    %
    %   Assumes Q = [W X Y Z]
    %

a = q(1);
b = q(2);
c = q(3);
d = q(4);

R = zeros(3);

R(1,1) = a^2+b^2-c^2-d^2;
R(1,2) = 2*(b*c-a*d);
R(1,3) = 2*(b*d+a*c);

R(2,1) = 2*(b*c+a*d);
R(2,2) = a^2-b^2+c^2-d^2;
R(2,3) = 2*(c*d-a*b);

R(3,1) = 2*(b*d-a*c);
R(3,2) = 2*(c*d+a*b);
R(3,3) = a^2-b^2-c^2+d^2;

end